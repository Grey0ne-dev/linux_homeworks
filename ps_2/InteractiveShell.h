#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <cerrno>

class InteractiveShell {
private:
    std::string currentPath;

public:
    InteractiveShell();

    void run();

private:
    void executeCommand(const std::string& command);
    std::vector<std::string> parseCommand(const std::string& command);

    void setupChildProcess(const std::vector<std::string>& args, bool silentMode);

    void addCurrentDirectoryToPath();

    std::string getCurrentPath();

    void setupSilentMode();

    void waitForChildProcess(pid_t pid);
};
