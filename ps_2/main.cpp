#include "InteractiveShell.h"
#include <string>
#include <cstring>

int main(int argc, char* argv[]) {
    bool silentMode = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--silent") == 0 || strcmp(argv[i], "-s") == 0) {
            silentMode = true;
        }
    }

    InteractiveShell shell(silentMode);
    shell.run();

    return 0;
}
