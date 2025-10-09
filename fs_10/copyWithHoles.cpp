#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Two file paths should be passed as arguments!\n";
        exit(1);
    }

    const char* srcPath = argv[1];
    const char* dstPath = argv[2];

    int srcFd = open(srcPath, O_RDONLY);
    if (srcFd == -1) {
        perror("Error opening source file");
        exit(1);
    }

    int dstFd = open(dstPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dstFd == -1) {
        perror("Error opening destination file");
        close(srcFd);
        exit(1);
    }

    off_t fileSize = lseek(srcFd, 0, SEEK_END);
    if (fileSize == -1) {
        perror("Error getting file size");
        close(srcFd);
        close(dstFd);
        exit(1);
    }
    
    if(lseek(srcFd, 0, SEEK_SET) == -1) {
        perror("Lseek error");
        close(srcFd);
        close(dstFd);
        exit(1);
    }
    
    off_t totalPhysical = 0;
    off_t offset = 0;
    size_t BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];

    while(offset < fileSize) {
        off_t dataOffset = lseek(srcFd, offset, SEEK_DATA);
        if (dataOffset == -1) {
            dataOffset = fileSize;
        }

        if (dataOffset > offset) {
            off_t holeSize = dataOffset - offset;
            if (lseek(dstFd, holeSize, SEEK_CUR) == -1) {
                perror("Error seeking in destination");
                close(srcFd);
                close(dstFd);
                exit(1);
            }
            offset = dataOffset;
        }

        off_t holeOffset = lseek(srcFd, offset, SEEK_HOLE);
        if (holeOffset == -1) {
            holeOffset = fileSize;
        }

        off_t chunkEnd = std::min(holeOffset, fileSize);
        while (offset < chunkEnd) {
            size_t toRead = std::min((off_t)BUFFER_SIZE, chunkEnd - offset);
            if (lseek(srcFd, offset, SEEK_SET) == -1) {
                perror("lseek error");
                close(srcFd);
                close(dstFd);
                exit(1);
            }
            
            ssize_t bytesRead = read(srcFd, buffer, toRead);
            if (bytesRead == -1) {
                perror("Read error");
                close(srcFd);
                close(dstFd);
                exit(1);
            }

            ssize_t bytesWritten = write(dstFd, buffer, bytesRead);
            if (bytesWritten != bytesRead) {
                perror("Write error");
                close(srcFd);
                close(dstFd);
                exit(1);
            }

            offset += bytesRead;
            totalPhysical += bytesWritten;
        }
    }

    close(srcFd);
    close(dstFd);

    std::cout << "Successfully copied " << fileSize
              << " bytes (data: " << totalPhysical
              << ", hole: " << (fileSize - totalPhysical) << ").\n";
    return 0;
}
