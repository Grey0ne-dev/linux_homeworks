#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <queue>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <memory>
#include <stdexcept>

class ExternalSorter {
private:
    size_t max_memory_size;

public:
    ExternalSorter(size_t max_mem) : max_memory_size(max_mem) {}

    void sort(const std::string& input_file, const std::string& output_file) {
        std::vector<std::string> temp_files = create_sorted_runs(input_file);
        merge_sorted_files(temp_files, output_file);

        for (const auto& temp_file : temp_files) {
            std::remove(temp_file.c_str());
        }
    }

private:
    std::vector<std::string> create_sorted_runs(const std::string& input_file) {
        std::vector<std::string> temp_files;
        int fd = open(input_file.c_str(), O_RDONLY);
        if (fd < 0) {
            throw std::runtime_error("Cannot open input file");
        }

        std::vector<std::string> buffer;
        size_t current_memory_usage = 0;
        int run_count = 0;

        constexpr size_t READ_BUFFER_SIZE = 4096;
        char read_buf[READ_BUFFER_SIZE];
        std::string leftover;

        ssize_t bytes_read;
        while ((bytes_read = read(fd, read_buf, READ_BUFFER_SIZE)) > 0) {
            leftover.append(read_buf, bytes_read);

            size_t pos;
            while ((pos = leftover.find('\n')) != std::string::npos) {
                std::string line = leftover.substr(0, pos);
                leftover.erase(0, pos + 1);

                size_t line_size = line.size() + 1;
                if (current_memory_usage + line_size > max_memory_size && !buffer.empty()) {
                    std::string temp_filename = "temp_run_" + std::to_string(run_count++) + ".txt";
                    write_sorted_chunk(buffer, temp_filename);
                    temp_files.push_back(temp_filename);

                    buffer.clear();
                    current_memory_usage = 0;
                }

                buffer.push_back(line);
                current_memory_usage += line_size;
            }
        }

        if (!leftover.empty()) {
            buffer.push_back(leftover);
        }

        if (!buffer.empty()) {
            std::string temp_filename = "temp_run_" + std::to_string(run_count++) + ".txt";
            write_sorted_chunk(buffer, temp_filename);
            temp_files.push_back(temp_filename);
        }

        close(fd);
        return temp_files;
    }

    void write_sorted_chunk(std::vector<std::string>& chunk, const std::string& filename) {
        std::sort(chunk.begin(), chunk.end());

        int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            throw std::runtime_error("Cannot create temporary file");
        }

        for (const auto& line : chunk) {
            std::string to_write = line + "\n";
            ssize_t written = write(fd, to_write.data(), to_write.size());
            if (written != static_cast<ssize_t>(to_write.size())) {
                close(fd);
                throw std::runtime_error("Write error");
            }
        }

        close(fd);
    }

    void merge_sorted_files(const std::vector<std::string>& input_files, const std::string& output_file) {
        if (input_files.empty()) {
            int fd = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd >= 0) close(fd);
            return;
        }

        if (input_files.size() == 1) {
            int in_fd = open(input_files[0].c_str(), O_RDONLY);
            int out_fd = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (in_fd < 0 || out_fd < 0) {
                throw std::runtime_error("Cannot open files for copying");
            }

            char buf[4096];
            ssize_t bytes_read;
            while ((bytes_read = read(in_fd, buf, sizeof(buf))) > 0) {
                write(out_fd, buf, bytes_read);
            }

            close(in_fd);
            close(out_fd);
            return;
        }

        using QueueItem = std::pair<std::string, int>;
        auto cmp = [](const QueueItem& a, const QueueItem& b) { return a.first > b.first; };
        std::priority_queue<QueueItem, std::vector<QueueItem>, decltype(cmp)> min_heap(cmp);

        std::vector<int> fds(input_files.size(), -1);
        std::vector<std::string> leftovers(input_files.size());

        for (size_t i = 0; i < input_files.size(); ++i) {
            fds[i] = open(input_files[i].c_str(), O_RDONLY);
            if (fds[i] < 0) throw std::runtime_error("Cannot open temporary file");

            char buf[4096];
            ssize_t bytes_read = read(fds[i], buf, sizeof(buf));
            if (bytes_read > 0) {
                leftovers[i].append(buf, bytes_read);
                size_t pos = leftovers[i].find('\n');
                if (pos != std::string::npos) {
                    std::string line = leftovers[i].substr(0, pos);
                    leftovers[i].erase(0, pos + 1);
                    min_heap.push({line, static_cast<int>(i)});
                }
            }
        }

        int out_fd = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (out_fd < 0) throw std::runtime_error("Cannot create output file");

        char temp_buf[4096];
        while (!min_heap.empty()) {
            auto [smallest, idx] = min_heap.top();
            min_heap.pop();

            std::string to_write = smallest + "\n";
            write(out_fd, to_write.data(), to_write.size());

            while (true) {
                size_t pos = leftovers[idx].find('\n');
                if (pos != std::string::npos) {
                    std::string line = leftovers[idx].substr(0, pos);
                    leftovers[idx].erase(0, pos + 1);
                    min_heap.push({line, idx});
                    break;
                }

                ssize_t bytes_read = read(fds[idx], temp_buf, sizeof(temp_buf));
                if (bytes_read <= 0) {
                    break;
                }
                leftovers[idx].append(temp_buf, bytes_read);
            }
        }

        for (auto fd : fds) {
            if (fd >= 0) close(fd);
        }
        close(out_fd);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file> <memory_MB>\n";
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];
    size_t memory_mb = std::stoul(argv[3]);
    size_t memory_bytes = memory_mb * 1024 * 1024;

    try {
        ExternalSorter sorter(memory_bytes);
        sorter.sort(input_file, output_file);
        std::cout << "File sorted successfully!\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}