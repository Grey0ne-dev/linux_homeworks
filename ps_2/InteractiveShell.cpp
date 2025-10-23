#include "InteractiveShell.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>

InteractiveShell::InteractiveShell(bool silent) : silentMode(silent) {
    char buffer[1024];
    if (getcwd(buffer, sizeof(buffer)) != nullptr) {
        currentPath = buffer;
    }
}

void InteractiveShell::run() {
    std::string input;

    if (!silentMode) {
        std::cout << "Interactive Shell Started. Type 'exit' to quit." << std::endl;
    }

    while (true) {
        if (!silentMode) {
            std::cout << "shell> ";
        }
        std::getline(std::cin, input);

        if (input == "exit") break;
        if (input.empty()) continue;

        executeCommand(input);
    }
}

void InteractiveShell::executeCommand(const std::string& command) {
    std::vector<std::string> tokens = parseCommand(command);
    if (tokens.empty()) return;

    executeCommandWithOperators(tokens);
}

std::vector<std::string> InteractiveShell::parseCommand(const std::string& command) {
    std::vector<std::string> tokens;
    std::string currentToken;
    bool inQuotes = false;
    char quoteChar = 0;

    for (size_t i = 0; i < command.length(); i++) {
        char c = command[i];

        if ((c == '"' || c == '\'') && !inQuotes) {
            inQuotes = true;
            quoteChar = c;
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
        } else if (inQuotes && c == quoteChar) {
            inQuotes = false;
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
        } else if (c == ' ' && !inQuotes) {
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
        } else {
            currentToken += c;
        }
    }

    if (!currentToken.empty()) {
        tokens.push_back(currentToken);
    }

    return tokens;
}

bool InteractiveShell::executeCommandWithOperators(const std::vector<std::string>& tokens) {
    std::vector<std::string> currentArgs;
    std::string inputFile, outputFile;
    bool append = false;
    bool useLogicalAnd = true;

    for (size_t i = 0; i < tokens.size(); i++) {
        const std::string& token = tokens[i];

        if (token == ">") {
            if (i + 1 < tokens.size()) {
                outputFile = tokens[++i];
                append = false;
            }
            continue;
        } else if (token == ">>") {
            if (i + 1 < tokens.size()) {
                outputFile = tokens[++i];
                append = true;
            }
            continue;
        } else if (token == "<") {
            if (i + 1 < tokens.size()) {
                inputFile = tokens[++i];
            }
            continue;
        }

        if (token == "&&" || token == "||") {
            if (!currentArgs.empty()) {
                int exitCode = executeProcess(currentArgs, inputFile, outputFile, append);

                inputFile.clear();
                outputFile.clear();
                append = false;

                if (token == "&&" && exitCode != 0) {
                    return false;
                } else if (token == "||" && exitCode == 0) {
                    return true;
                }

                currentArgs.clear();
            }
            continue;
        }

        currentArgs.push_back(token);
    }

    if (!currentArgs.empty()) {
        int exitCode = executeProcess(currentArgs, inputFile, outputFile, append);
        return exitCode == 0;
    }

    return true;
}

int InteractiveShell::executeProcess(const std::vector<std::string>& args,
                                   const std::string& inputFile,
                                   const std::string& outputFile,
                                   bool append) {
    pid_t pid = fork();

    if (pid == -1) {
        std::cerr << "Fork failed" << std::endl;
        return -1;
    }

    if (pid == 0) {
        addCurrentDirectoryToPath();

        if (!inputFile.empty()) {
            int fd = open(inputFile.c_str(), O_RDONLY);
            if (fd == -1) {
                std::cerr << "Couldn't open input file: " << inputFile << std::endl;
                exit(EXIT_FAILURE);
            }
            if (dup2(fd, STDIN_FILENO) == -1) {
                std::cerr << "Couldn't duplicate stdin" << std::endl;
                exit(EXIT_FAILURE);
            }
            close(fd);
        }

        if (!outputFile.empty()) {
            int flags = O_WRONLY | O_CREAT;
            flags |= append ? O_APPEND : O_TRUNC;

            int fd = open(outputFile.c_str(), flags, 0644);
            if (fd == -1) {
                std::cerr << "Couldn't open output file: " << outputFile << std::endl;
                exit(EXIT_FAILURE);
            }
            if (dup2(fd, STDOUT_FILENO) == -1) {
                std::cerr << "Couldn't duplicate stdout" << std::endl;
                exit(EXIT_FAILURE);
            }
            close(fd);
        }

        std::vector<char*> execArgs;
        for (const auto& arg : args) {
            execArgs.push_back(const_cast<char*>(arg.c_str()));
        }
        execArgs.push_back(nullptr);


        execvp(execArgs[0], execArgs.data());

        std::cerr << "Command not found: " << args[0] << std::endl;
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    }
}

void InteractiveShell::addCurrentDirectoryToPath() {
    std::string currentPath = getCurrentPath();
    char* existingPath = getenv("PATH");
    if (existingPath) {
        std::string newPath = currentPath + ":" + existingPath;
        setenv("PATH", newPath.c_str(), 1);
    } else {
        setenv("PATH", currentPath.c_str(), 1);
    }
}

std::string InteractiveShell::getCurrentPath() {
    char buffer[1024];
    if (getcwd(buffer, sizeof(buffer)) != nullptr) {
        return buffer;
    }
    return ".";
}