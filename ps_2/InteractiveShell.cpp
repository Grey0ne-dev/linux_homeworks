#include "InteractiveShell.h"

InteractiveShell::InteractiveShell() {
        char buffer[1024];
        if (getcwd(buffer, sizeof(buffer)) != nullptr) {
            currentPath = buffer;
        }
    }

void InteractiveShell::run() {
            std::string input;
            std::cout << "Interactive Shell Started. Type 'exit' to quit." << std::endl;

            while (true) {
                std::cout << "shell> ";
                std::getline(std::cin, input);

                if (input == "exit") break;
                if (input.empty()) continue;

                executeCommand(input);
            }
        }

void InteractiveShell::executeCommand(const std::string& command) {
        std::vector<std::string> args = parseCommand(command);
        if (args.empty()) return;

        bool silentMode = (args[0] == "silent");
        if (silentMode) args.erase(args.begin());
        if (args.empty()) return;

        pid_t pid = fork();
        if (pid == -1) return;
        else if (pid == 0) setupChildProcess(args, silentMode);
        else waitForChildProcess(pid);
    }

std::vector<std::string> InteractiveShell::parseCommand(const std::string& command) {
        std::vector<std::string> args;
        std::string currentArg;
        bool inQuotes = false;

        for (size_t i = 0; i < command.length(); i++) {
            char c = command[i];
            if (c == '"') inQuotes = !inQuotes;
            else if (c == ' ' && !inQuotes) {
                if (!currentArg.empty()) {
                    args.push_back(currentArg);
                    currentArg.clear();
                }
            } else currentArg += c;
        }

        if (!currentArg.empty()) args.push_back(currentArg);
        return args;
    }

void InteractiveShell::setupChildProcess(const std::vector<std::string>& args, bool silentMode) {
        addCurrentDirectoryToPath();
        if (silentMode) setupSilentMode();

        std::vector<char*> execArgs;
        for (const auto& arg : args) execArgs.push_back(const_cast<char*>(arg.c_str()));
        execArgs.push_back(nullptr);

        execvp(execArgs[0], execArgs.data());
        exit(EXIT_FAILURE);
    }

void InteractiveShell::addCurrentDirectoryToPath() {
        std::string currentPath = getCurrentPath();
        char* existingPath = getenv("PATH");
        if (existingPath) {
            std::string newPath = currentPath + ":" + existingPath;
            setenv("PATH", newPath.c_str(), 1);
        } else setenv("PATH", currentPath.c_str(), 1);
    }

std::string InteractiveShell::getCurrentPath() {
        char buffer[1024];
        if (getcwd(buffer, sizeof(buffer)) != nullptr) return buffer;
        return ".";
    }

void InteractiveShell::setupSilentMode() {
        pid_t pid = getpid();
        std::string filename = std::to_string(pid) + ".log";

        int logFile = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
        if (logFile == -1) exit(EXIT_FAILURE);

        dup2(logFile, STDOUT_FILENO);
        dup2(logFile, STDERR_FILENO);
        close(logFile);
    }

void InteractiveShell::waitForChildProcess(pid_t pid) {
        int status;
        waitpid(pid, &status, 0);
    }

