#ifndef INTERACTIVESHELL_H
#define INTERACTIVESHELL_H

#include <string>
#include <vector>
#include <sys/types.h>

class InteractiveShell {
private:
    std::string currentPath;
    bool silentMode;

    std::vector<std::string> parseCommand(const std::string& command);
    void executeCommand(const std::string& command);

    bool executeCommandWithOperators(const std::vector<std::string>& tokens);
    bool executeSingleCommand(const std::vector<std::string>& args,
                             const std::string& inputFile = "",
                             const std::string& outputFile = "",
                             bool append = false);
    bool executePipeline(const std::vector<std::vector<std::string>>& commands);

    void addCurrentDirectoryToPath();
    std::string getCurrentPath();
    void setupRedirections(const std::string& inputFile,
                          const std::string& outputFile,
                          bool append);
    int executeProcess(const std::vector<std::string>& args,
                      const std::string& inputFile = "",
                      const std::string& outputFile = "",
                      bool append = false);
    int executeProcess(const std::vector<std::string>& args,
                      const std::string& inputFile,
                      const std::string& outputFile,
                      bool append,
                      bool background);

public:
    InteractiveShell(bool silent = false);
    void run();
};

#endif
