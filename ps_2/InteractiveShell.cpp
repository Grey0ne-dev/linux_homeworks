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
    bool runInBackground = false;
    std::vector<std::vector<std::string>> pipeCommands;

    for (size_t i = 0; i < tokens.size(); i++) {
        const std::string& token = tokens[i];

        if (token == "&" && i == tokens.size() - 1) {
            runInBackground = true;
            break;
        } else if (token == ">") {
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
        } else if (token == "|") {
            if (!currentArgs.empty()) {
                pipeCommands.push_back(currentArgs);
                currentArgs.clear();
            }
            continue;
        }

        if (token == "&&" || token == "||") {
            if (!pipeCommands.empty() || !currentArgs.empty()) {
                if (!currentArgs.empty()) {
                    pipeCommands.push_back(currentArgs);
                    currentArgs.clear();
                }
                
                int exitCode;
                if (pipeCommands.size() > 1) {
                    exitCode = executePipeline(pipeCommands) ? 0 : -1;
                } else if (!pipeCommands.empty()) {
                    exitCode = executeProcess(pipeCommands[0], inputFile, outputFile, append, runInBackground);
                } else {
                    exitCode = -1;
                }

                pipeCommands.clear();
                inputFile.clear();
                outputFile.clear();
                append = false;
                runInBackground = false;

                if (token == "&&" && exitCode != 0) {
                    return false;
                } else if (token == "||" && exitCode == 0) {
                    return true;
                }
            }
            continue;
        }

        currentArgs.push_back(token);
    }

    if (!pipeCommands.empty() || !currentArgs.empty()) {
        if (!currentArgs.empty()) {
            pipeCommands.push_back(currentArgs);
        }
        
        if (pipeCommands.size() > 1) {
            return executePipeline(pipeCommands);
        } else if (!pipeCommands.empty()) {
            int exitCode = executeProcess(pipeCommands[0], inputFile, outputFile, append, runInBackground);
            return exitCode == 0;
        }
    }

    return true;
}

int InteractiveShell::executeProcess(const std::vector<std::string>& args,
                                   const std::string& inputFile,
                                   const std::string& outputFile,
                                   bool append) {
    return executeProcess(args, inputFile, outputFile, append, false);
}

int InteractiveShell::executeProcess(const std::vector<std::string>& args,
                                   const std::string& inputFile,
                                   const std::string& outputFile,
                                   bool append,
                                   bool background) {
    pid_t pid = fork();

    if (pid == -1) {
        if (!silentMode) {
            std::cerr << "Fork failed" << std::endl;
        }
        return -1;
    }

    if (pid == 0) {
        addCurrentDirectoryToPath();

        if (silentMode) {
            int devnull = open("/dev/null", O_WRONLY);
            if (devnull != -1) {
                dup2(devnull, STDERR_FILENO);
                close(devnull);
            }
        }

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
        if (background) {
            if (!silentMode) {
                std::cout << "Background process started with PID: " << pid << std::endl;
            }
            return 0;
        }
        
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    }
}

bool InteractiveShell::executePipeline(const std::vector<std::vector<std::string>>& commands) {
    int numCommands = commands.size();
    int pipefds[2 * (numCommands - 1)];
    
    for (int i = 0; i < numCommands - 1; i++) {
        if (pipe(pipefds + i * 2) == -1) {
            if (!silentMode) {
                std::cerr << "Pipe creation failed" << std::endl;
            }
            return false;
        }
    }
    
    for (int i = 0; i < numCommands; i++) {
        pid_t pid = fork();
        
        if (pid == -1) {
            if (!silentMode) {
                std::cerr << "Fork failed" << std::endl;
            }
            return false;
        }
        
        if (pid == 0) {
            addCurrentDirectoryToPath();
            
            if (silentMode) {
                int devnull = open("/dev/null", O_WRONLY);
                if (devnull != -1) {
                    dup2(devnull, STDERR_FILENO);
                    close(devnull);
                }
            }
            
            if (i > 0) {
                if (dup2(pipefds[(i - 1) * 2], STDIN_FILENO) == -1) {
                    std::cerr << "dup2 failed for stdin" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            
            if (i < numCommands - 1) {
                if (dup2(pipefds[i * 2 + 1], STDOUT_FILENO) == -1) {
                    std::cerr << "dup2 failed for stdout" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            
            for (int j = 0; j < 2 * (numCommands - 1); j++) {
                close(pipefds[j]);
            }
            
            std::vector<char*> execArgs;
            for (const auto& arg : commands[i]) {
                execArgs.push_back(const_cast<char*>(arg.c_str()));
            }
            execArgs.push_back(nullptr);
            
            execvp(execArgs[0], execArgs.data());
            
            std::cerr << "Command not found: " << commands[i][0] << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    
    for (int i = 0; i < 2 * (numCommands - 1); i++) {
        close(pipefds[i]);
    }
    
    for (int i = 0; i < numCommands; i++) {
        int status;
        wait(&status);
    }
    
    return true;
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
