#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include "parser.h"

char** vectorToCharArray(const std::vector<std::string>& vec) {
    char** args = new char*[vec.size() + 1];
    for (size_t i = 0; i < vec.size(); ++i) {
        args[i] = const_cast<char*>(vec[i].c_str());
    }
    args[vec.size()] = nullptr;
    return args;
}

void executeSingleCommand(const command& cmd) {
    std::vector<std::string> args;
    for (int i = 0; cmd.args[i] != nullptr; ++i) {
        args.push_back(std::string(cmd.args[i]));
    }

    char** argv = vectorToCharArray(args);
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process. The first argument is the command to execute, rest are the arguments
        execvp(argv[0], argv);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        wait(nullptr);
    }
    delete[] argv;
}

int main() {
    std::string line;
    parsed_input input;

    while (true) {
        std::cout << "/> ";
        std::getline(std::cin, line);

        if (std::cin.eof()) {
            std::cout << "\nQuitting...\n";
            break;
        }

        if (!parse_line(const_cast<char*>(line.c_str()), &input)) {
            std::cout << "Error: Command parsing failed.\n";
            continue;
        }

        // Handle quit command
        if (input.num_inputs == 1 && input.inputs[0].type == INPUT_TYPE_COMMAND &&
            std::strcmp(input.inputs[0].data.cmd.args[0], "quit") == 0) {
            break;
        }

        // Execute single command
        if (input.num_inputs == 1 && input.inputs[0].type == INPUT_TYPE_COMMAND) {
            executeSingleCommand(input.inputs[0].data.cmd);
        }

        // Clean up
        free_parsed_input(&input);
    }

    return 0;
}
