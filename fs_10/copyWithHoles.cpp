#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <sys/stat.h>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <source> <destination>\n";
        return 1;
    }

    const char* srcPath = argv[1];
    const char* dstPath = argv[2];

    int srcFd = open(srcPath, O_RDONLY);
    if (srcFd == -1) {
        perror("Error opening source file");
        return 1;
    }

    struct stat st;
    if (fstat(srcFd, &st) == -1) {
        perror("fstat(source) failed");
        close(srcFd);
        return 1;
    }

    off_t fileSize = st.st_size;

    int dstFd = open(dstPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dstFd == -1) {
        perror("Error opening destination file");
        close(srcFd);
        return 1;
    }

    const size_t BUFFER_SIZE = 4096;
    std::vector<char> buffer(BUFFER_SIZE);

    off_t offset = 0;
    off_t totalPhysical = 0;

    while (offset < fileSize) {
        off_t dataOffset = lseek(srcFd, offset, SEEK_DATA);
        if (dataOffset == -1) {
            dataOffset = fileSize;
        }

        if (dataOffset > offset) {
            off_t holeSize = dataOffset - offset;
            if (lseek(dstFd, holeSize, SEEK_CUR) == -1) {
                perror("seek in destination failed");
                close(srcFd);
                close(dstFd);
                return 1;
            }
            offset = dataOffset;
        }

        if (offset >= fileSize)
            break;

        off_t holeOffset = lseek(srcFd, offset, SEEK_HOLE);
        if (holeOffset == -1) {
            holeOffset = fileSize;
        }

        off_t chunkEnd = std::min(holeOffset, fileSize);

        if (lseek(srcFd, offset, SEEK_SET) == -1 ||
            lseek(dstFd, offset, SEEK_SET) == -1) {
            perror("lseek failed");
            close(srcFd);
            close(dstFd);
            return 1;
        }

        while (offset < chunkEnd) {
            size_t toRead = static_cast<size_t>(std::min<off_t>(BUFFER_SIZE, chunkEnd - offset));
            ssize_t bytesRead = read(srcFd, buffer.data(), toRead);
            if (bytesRead <= 0) break;

            ssize_t bytesWritten = write(dstFd, buffer.data(), bytesRead);
            if (bytesWritten != bytesRead) {
                perror("write error");
                close(srcFd);
                close(dstFd);
                return 1;
            }

            offset += bytesWritten;
            totalPhysical += bytesWritten;
        }
    }

    if (truncate(dstPath, fileSize) == -1) {
        perror("truncate failed");
        close(srcFd);
        close(dstFd);
        return 1;
    }

    close(srcFd);
    close(dstFd);

    std::cout << "Copied " << fileSize
              << " bytes (data written: " << totalPhysical
              << ", holes: " << (fileSize - totalPhysical) << ").\n";
    return 0;
}

