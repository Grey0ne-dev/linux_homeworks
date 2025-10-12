#include <iostream>
#include <unistd.h>
#include <sys/wait.h> 
#include <chrono>
#include <vector>
#include <string>

void do_command(char** argv) {
    using namespace std::chrono;

    std::cout << "Executing: ";
    for (int i = 0; argv[i] != nullptr; ++i) {
        std::cout << argv[i] << " ";
    }
    std::cout << "\n" << std::endl;

    auto start = high_resolution_clock::now();

    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "Fork failed\n";
        return;
    } else if (pid == 0) {
        
        if (execvp(argv[0], argv) < 0) {
            std::cerr << "Execution failed for command: " << argv[0] << "\n";
            exit(1);
        }
    } else {
        
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            std::cerr << "waitpid failed\n";
            return;
        }

        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start).count();

        std::cout << "\n--- Command finished ---\n";
        if (WIFEXITED(status)) {
            std::cout << "Exit status: " << WEXITSTATUS(status) << "\n";
        } else if (WIFSIGNALED(status)) {
            std::cout << "Terminated by signal: " << WTERMSIG(status) << "\n";
        } else {
            std::cout << "Command terminated abnormally\n";
        }

        std::cout << "Execution time: " << duration << " ms\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <command> [args...]\n";
        std::cerr << "Example: " << argv[0] << " ls -l\n";
        return 1;
    }

    std::vector<char*> exec_args;
    for (int i = 1; i < argc; ++i) {
        exec_args.push_back(argv[i]);
    }
    exec_args.push_back(nullptr);

    do_command(exec_args.data());

    return 0;
}
