#pragma once

#include <string>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdint.h>
#include <limits>
#include <vector>

class shared_array { // arrays are implementedas chunks bc 4gb is huge
public:
    using value_type = int32_t;
    using u64 = uint64_t;

    static constexpr size_t DEFAULT_CHUNK_BYTES = 1UL << 30; // 1gb per chunk by default

    shared_array(const std::string& name, u64 size, size_t chunk_bytes = DEFAULT_CHUNK_BYTES)
        : base_name_(normalize_name(name)), size_(size), chunk_bytes_(chunk_bytes), mutex_(nullptr), control_map_(nullptr), control_size_(0)
    {
        if (size == 0) throw std::invalid_argument("size must be >=1");
        if (chunk_bytes_ < sizeof(value_type)) chunk_bytes_ = sizeof(value_type);
        // compute total bytes and chunk count, checking overflow
        u64 bytes_total = 0;
        if (!mul_overflow(size, sizeof(value_type), bytes_total)) throw std::overflow_error("size too large");
        chunk_count_ = (bytes_total + chunk_bytes_ - 1) / chunk_bytes_;

        control_name_ = base_name_ + std::string(".ctl");
        int ctl_fd = shm_open(control_name_.c_str(), O_CREAT | O_EXCL | O_RDWR, 0666);//nice number
        u64 meta_count = 3;
        size_t meta_size = meta_count * sizeof(u64);
        control_size_ = meta_size + sizeof(pthread_mutex_t);
        if (ctl_fd != -1) {
            if (ftruncate(ctl_fd, (off_t)control_size_) == -1) { int e=errno; close(ctl_fd); shm_unlink(control_name_.c_str()); throw std::runtime_error(std::string("ftruncate ctl failed: ")+strerror(e)); }
            void* mp = mmap(nullptr, control_size_, PROT_READ | PROT_WRITE, MAP_SHARED, ctl_fd, 0);
            if (mp == MAP_FAILED) { int e=errno; close(ctl_fd); shm_unlink(control_name_.c_str()); throw std::runtime_error(std::string("mmap ctl failed: ")+strerror(e)); }
            u64* meta = static_cast<u64*>(mp);
            meta[0] = size;
            meta[1] = chunk_count_;
            meta[2] = chunk_bytes_;
            // initialize process-shared mutex placed after metadata
            pthread_mutexattr_t attr; pthread_mutexattr_init(&attr); pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
            pthread_mutex_t* mptr = reinterpret_cast<pthread_mutex_t*>(reinterpret_cast<char*>(mp) + meta_size);
            if (pthread_mutex_init(mptr, &attr) != 0) { int e = errno; pthread_mutexattr_destroy(&attr); munmap(mp, control_size_); close(ctl_fd); shm_unlink(control_name_.c_str()); throw std::runtime_error(std::string("pthread_mutex_init failed: ")+strerror(e)); }
            pthread_mutexattr_destroy(&attr);
            msync(mp, control_size_, MS_SYNC);
            control_map_ = mp;
            mutex_ = mptr;
            close(ctl_fd);
        } else {
            if (errno == EEXIST) {
                ctl_fd = shm_open(control_name_.c_str(), O_RDWR, 0666);
                if (ctl_fd == -1) throw std::runtime_error(std::string("shm_open ctl failed: ")+strerror(errno));
                struct stat st; if (fstat(ctl_fd,&st)==-1){close(ctl_fd); throw std::runtime_error(std::string("fstat ctl failed: ")+strerror(errno));}
                if ((size_t)st.st_size != control_size_) { close(ctl_fd); throw std::runtime_error("control shm has unexpected size"); }
                void* mp = mmap(nullptr, control_size_, PROT_READ | PROT_WRITE, MAP_SHARED, ctl_fd, 0);
                if (mp == MAP_FAILED){int e=errno; close(ctl_fd); throw std::runtime_error(std::string("mmap ctl failed: ")+strerror(e));}
                u64* meta = static_cast<u64*>(mp);
                u64 existing_size = meta[0]; u64 existing_chunks = meta[1]; u64 existing_chunk_bytes = meta[2];
                if (existing_size != size) { munmap(mp, control_size_); close(ctl_fd); throw std::runtime_error("Existing shared array has different size"); }
                if (existing_chunk_bytes != chunk_bytes_) { munmap(mp, control_size_); close(ctl_fd); throw std::runtime_error("Existing shared array has different chunk size"); }
                chunk_count_ = existing_chunks;
                control_map_ = mp;
                mutex_ = reinterpret_cast<pthread_mutex_t*>(reinterpret_cast<char*>(mp) + meta_size);
                close(ctl_fd);
            } else {
                throw std::runtime_error(std::string("shm_open ctl failed: ") + strerror(errno));
            }
        }

        chunk_names_.resize(chunk_count_);
        for (u64 i=0;i<chunk_count_;++i) chunk_names_[i] = base_name_ + ".chunk." + std::to_string(i);
        maps_.assign(chunk_count_, nullptr);
        fds_.assign(chunk_count_, -1);
    }

    ~shared_array() {
        for (size_t i=0;i<maps_.size();++i) {
            if (maps_[i]) {
                size_t cb = chunk_size_map(i);
                munmap(maps_[i], cb);
            }
            if (fds_[i] != -1) close(fds_[i]);
        }
        if (control_map_) munmap(control_map_, control_size_);
    }

    shared_array(const shared_array&) = delete;
    shared_array& operator=(const shared_array&) = delete;

    value_type& operator[](u64 idx) {
        if (idx >= size_) throw std::out_of_range("index out of range");
        u64 byte_off = idx * sizeof(value_type);
        u64 chunk = byte_off / chunk_bytes_;
        size_t offset_in_chunk = (size_t)(byte_off - chunk * chunk_bytes_);
        ensure_mapped(chunk);
        return reinterpret_cast<value_type*>(reinterpret_cast<char*>(maps_[chunk]) + offset_in_chunk)[0];
    }

    u64 size() const { return size_; }

    void lock() { if (!mutex_) throw std::runtime_error("mutex not initialized"); if (pthread_mutex_lock(mutex_) != 0) throw std::runtime_error(std::string("pthread_mutex_lock failed: ")+strerror(errno)); }
    void unlock() { if (!mutex_) throw std::runtime_error("mutex not initialized"); if (pthread_mutex_unlock(mutex_) != 0) throw std::runtime_error(std::string("pthread_mutex_unlock failed: ")+strerror(errno)); }

private:
    static bool mul_overflow(u64 a, u64 b, u64 &out) {
        __uint128_t res = ( __uint128_t)a * (__uint128_t)b;
        u64 maxv = std::numeric_limits<u64>::max();
        if (res > maxv) return false;
        out = (u64)res;
        return true;
    }

    static std::string normalize_name(const std::string& name) {
        if (name.empty()) throw std::invalid_argument("name cannot be empty");
        if (name[0]=='/') return name;
        return std::string("/") + name;
    }

    size_t chunk_size_map(size_t chunk_index) const {
        u64 bytes_total = (u64)size_ * sizeof(value_type);
        u64 start = (u64)chunk_index * chunk_bytes_;
        u64 rem = (start + chunk_bytes_ > bytes_total) ? (bytes_total - start) : chunk_bytes_;
        return (size_t)rem;
    }

    void ensure_mapped(size_t chunk) {
        if (maps_[chunk]) return;
        const std::string &cname = chunk_names_[chunk];
        int fd = shm_open(cname.c_str(), O_CREAT | O_EXCL | O_RDWR, 0666);
        bool owner = false;
        size_t mapbytes = chunk_size_map(chunk);
        if (fd != -1) {
            owner = true;
            if (ftruncate(fd, (off_t)mapbytes) == -1) { int e=errno; close(fd); shm_unlink(cname.c_str()); throw std::runtime_error(std::string("ftruncate chunk failed: ")+strerror(e)); }
        } else {
            if (errno == EEXIST) {
                fd = shm_open(cname.c_str(), O_RDWR, 0666);
                if (fd == -1) throw std::runtime_error(std::string("shm_open chunk failed: ")+strerror(errno));
                struct stat st; if (fstat(fd,&st)==-1){close(fd); throw std::runtime_error(std::string("fstat chunk failed: ")+strerror(errno));}
                if ((size_t)st.st_size != mapbytes) { close(fd); throw std::runtime_error("Existing chunk has different size"); }
            } else { throw std::runtime_error(std::string("shm_open chunk failed: ")+strerror(errno)); }
        }
        void* p = mmap(nullptr, mapbytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (p == MAP_FAILED) { int e=errno; close(fd); if (owner) shm_unlink(cname.c_str()); throw std::runtime_error(std::string("mmap chunk failed: ")+strerror(e)); }
        maps_[chunk] = p;
        fds_[chunk] = fd;
    }

    std::string base_name_;
    std::string control_name_;
    u64 size_;
    size_t chunk_bytes_;
    u64 chunk_count_;
    std::vector<std::string> chunk_names_;
    std::vector<void*> maps_;
    std::vector<int> fds_;
    pthread_mutex_t* mutex_;
    void* control_map_;
    size_t control_size_;
};
