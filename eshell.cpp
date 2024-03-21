#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include "parser.h"


void freeCharArray(char** argv) {
    for (size_t i = 0; argv[i] != nullptr; ++i) {
        delete[] argv[i];
    }
    delete[] argv;
}

char** vectorToCharArray(const command& cmd) {
    std::vector<std::string> vec;
    for (int j = 0; cmd.args[j] != nullptr; ++j) {
        vec.push_back(std::string(cmd.args[j]));
    }

    char** args = new char*[vec.size() + 1];
    for (size_t i = 0; i < vec.size(); ++i) {
        args[i] = new char[vec[i].size() + 1];
        strcpy(args[i], vec[i].c_str());
    }
    args[vec.size()] = nullptr;
    return args;
}

void executeSingleCommand(const command& cmd) {
    char** argv = vectorToCharArray(cmd);
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
    
    // Free the memory
    size_t argvSize = sizeof(argv);
    freeCharArray(argv);
}

void executePipeline(single_input* inputs, int size){
    int pipefd[2]; // 0 is the read end, 1 is the write end
    // store pipe file descriptors
    int pipefds[size - 1][2];

    for (int i = 0; i < size - 1; i++) {
        if (pipe(pipefds[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < size; i++){
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            if (i == 0) {
                // write to the next pipe
                dup2(pipefds[i][1], STDOUT_FILENO);
            }else if (i < size - 1){
                // read from the previous pipe, write to the next pipe
                dup2(pipefds[i-1][0], STDIN_FILENO);
                dup2(pipefds[i][1], STDOUT_FILENO);
            } else {
                // read from the previous pipe
                dup2(pipefds[i - 1][0], STDIN_FILENO);
            }

            for (int j = 0; j < size - 1; j++) {
                close(pipefds[j][0]);
                close(pipefds[j][1]);
            }



            char** argv = vectorToCharArray(inputs[i].data.cmd);
            execvp(argv[0], argv);

            // free the memory
            freeCharArray(argv);
        }
    }

    // close all pipe fds in the parent
    for (int i = 0; i < size - 1; i++) {
        close(pipefds[i][0]);
        close(pipefds[i][1]);
    }

    // reap the child processes
    for (int i = 0; i < size; i++){
        wait(nullptr);
    }

}

void executeSequential(single_input* inputs, int size){

}

void executeParallel(single_input* inputs, int size){

}

void executeSubshell(single_input* inputs, int size){

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
        }else if (input.separator == SEPARATOR_PIPE){
            executePipeline(input.inputs, input.num_inputs);
        } else if (input.separator == SEPARATOR_SEQ){
            executeSequential(input.inputs, input.num_inputs);
        } else if (input.separator == SEPARATOR_PARA){
            executeParallel(input.inputs, input.num_inputs);
        } else if (input.separator == SEPARATOR_NONE){
            executeSubshell(input.inputs, input.num_inputs);
        }

        // Clean up
        free_parsed_input(&input);
    }

    return 0;
}
