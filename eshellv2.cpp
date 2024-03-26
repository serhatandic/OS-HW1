#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include "parser.h"

void executeSubshell(single_input line);

void executeSingleCommand(const command& cmd) {
    execvp(cmd.args[0], cmd.args);
}

void executePipeline(single_input* inputs, int size, bool parallel = false){
    int pipefds[size - 1][2]; // 0 is the read end, 1 is the write end
    
    for (int i = 0; i < size - 1; i++){
        pipe(pipefds[i]);
    }

    for (int i = 0; i < size ; i++){
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            if (i == 0) {
                // write to the next pipe
                dup2(pipefds[i][1], STDOUT_FILENO);
            }else if (i < size - 1){
                // read from the previous pipe, write to the next pipe
                dup2(pipefds[i - 1][0], STDIN_FILENO);
                dup2(pipefds[i][1], STDOUT_FILENO);

            } else {
                // read from the previous pipe
                dup2(pipefds[i - 1][0], STDIN_FILENO);
            }

            for (int j = 0; j < size - 1; j++){
                close(pipefds[j][0]);
                close(pipefds[j][1]);
            }

            // command or subshell
            if (inputs[i].type == INPUT_TYPE_COMMAND){
                executeSingleCommand(inputs[i].data.cmd);
            }else if (inputs[i].type == INPUT_TYPE_SUBSHELL){
                executeSubshell(inputs[i]);
            }
            exit(0);
        }
    }

    for (int i = 0; i < size - 1; i++){
        close(pipefds[i][0]);
        close(pipefds[i][1]);
    }
    // reap the child processes
    if (!parallel){
        while(wait(nullptr) > 0);
    }
}

void executePipelineForCmd(command* inputs, int size, bool parallel = false){
    int pipefds[size - 1][2]; // 0 is the read end, 1 is the write end
    
    for (int i = 0; i < size - 1; i++){
        pipe(pipefds[i]);
    }

    for (int i = 0; i < size ; i++){
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            if (i == 0) {
                // write to the next pipe
                dup2(pipefds[i][1], STDOUT_FILENO);
            }else if (i < size - 1){
                // read from the previous pipe, write to the next pipe
                dup2(pipefds[i - 1][0], STDIN_FILENO);
                dup2(pipefds[i][1], STDOUT_FILENO);

            } else {
                // read from the previous pipe
                dup2(pipefds[i - 1][0], STDIN_FILENO);
            }

            for (int j = 0; j < size - 1; j++){
                close(pipefds[j][0]);
                close(pipefds[j][1]);
            }

            // command or subshell
            executeSingleCommand(inputs[i]);
            exit(0);
        }
    }

    for (int i = 0; i < size - 1; i++){
        close(pipefds[i][0]);
        close(pipefds[i][1]);
    }
    // reap the child processes
    if (!parallel){
        while(wait(nullptr) > 0);
    }
}
void executeSequential(single_input* inputs, int size){
    for (int i = 0; i < size; i++){
        if (inputs[i].type == INPUT_TYPE_PIPELINE){
            executePipelineForCmd(inputs[i].data.pline.commands, inputs[i].data.pline.num_commands, false);
        } else if (inputs[i].type == INPUT_TYPE_COMMAND){
            pid_t pid = fork();
            if (pid == 0) {
                executeSingleCommand(inputs[i].data.cmd);
                exit(0);
            }else{
                waitpid(pid, nullptr, 0);
            }
        }
    }
}

void executeParallel(single_input* inputs, int size){
    for (int i = 0; i < size; i++){
        if (inputs[i].type == INPUT_TYPE_PIPELINE){
            executePipelineForCmd(inputs[i].data.pline.commands, inputs[i].data.pline.num_commands, true);
        } else if (inputs[i].type == INPUT_TYPE_COMMAND){
            pid_t pid = fork();
            if (pid == 0) {
                // Child process
                executeSingleCommand(inputs[i].data.cmd);
                exit(0);
            }
        }
    }

    // wait all pids to finish
    while(wait(nullptr) > 0);
}

void executeSubshell(single_input line){
    parsed_input input;
    
    parse_line(line.data.subshell, &input);
    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        if (input.num_inputs == 1 && input.inputs[0].type == INPUT_TYPE_COMMAND){
            executeSingleCommand(input.inputs[0].data.cmd);
        }else if (input.separator == SEPARATOR_PIPE){
            executePipeline(input.inputs, input.num_inputs);
        }else if (input.separator == SEPARATOR_SEQ){
            executeSequential(input.inputs, input.num_inputs);
        }else if (input.separator == SEPARATOR_PARA){
            // according to above specifications 
            int size = input.num_inputs;
            int pipefds[size][2];
            
            for (int i = 0; i < size; i++){
                pipe(pipefds[i]);
            }
                        
            for (int i = 0; i < size; i++){
                pid_t pid = fork();
                if (pid == 0){
                    
                    dup2(pipefds[i][0], STDIN_FILENO);
                    for (int j = 0; j < size; j++) {
                        close(pipefds[j][0]);
                        close(pipefds[j][1]);
                    }
                    if (input.inputs[i].type == INPUT_TYPE_PIPELINE){
                        executePipeline(input.inputs, input.num_inputs, true);
                        while(wait(nullptr) > 0);
                    }else if (input.inputs[i].type == INPUT_TYPE_COMMAND) {
                        pid_t pid2 = fork();
                        if (pid2 == 0){
                            // std::cout << "executing " << input.inputs[i].data.cmd.args[0] << std::endl;
                            executeSingleCommand(input.inputs[i].data.cmd);   
                        }else{
                            waitpid(pid2, nullptr, 0);
                        }
                    }
                    free_parsed_input(&input);
                    for (int j = 0; j < size; j++){
                        close(pipefds[j][0]);
                        close(pipefds[j][1]);
                    }
                    exit(0);
                }
                
            }

            pid_t pid2 = fork();
            if (pid2 == 0){
                std::string line;
                // repeater
                while (std::getline(std::cin, line)){
                    line += "\n";
                    for (int i = 0; i < size; i++){
                        write(pipefds[i][1], line.c_str(), line.size());
                    }
                }

                for (int i = 0; i < size; i++){
                    close(pipefds[i][0]);
                    close(pipefds[i][1]);
                }
                exit(0);
            }

            for (int i = 0; i < size; i++){
                close(pipefds[i][0]);
                close(pipefds[i][1]);
            }
        }
        while(wait(nullptr) > 0);
        exit(0);
    }

    while(wait(nullptr) > 0);
    // reap the child processes
    free_parsed_input(&input);
}

int main() {
    std::string line;
    parsed_input input;
    pid_t pid = getpid();
    while (true) {
        std::cout << "/> ";
        std::getline(std::cin, line);
        
        if (std::cin.eof()) {
            break;
        }

        if (!parse_line(const_cast<char*>(line.c_str()), &input)) {
            std::cout << "Error: Command parsing failed.\n";
            continue;
        }

        // Handle quit command
        if (input.num_inputs == 1 && input.inputs[0].type == INPUT_TYPE_COMMAND && std::strcmp(input.inputs[0].data.cmd.args[0], "quit") == 0) {
            break;
        }

        // Execute single command
        if (input.num_inputs == 1 && input.inputs[0].type == INPUT_TYPE_COMMAND) {
            pid_t pid = fork();
            if (pid == 0) {
                executeSingleCommand(input.inputs[0].data.cmd);
            } else {
                waitpid(pid, nullptr, 0);
            }
        }else if (input.separator == SEPARATOR_PIPE){
            executePipeline(input.inputs, input.num_inputs);
        }else if (input.separator == SEPARATOR_SEQ){
            executeSequential(input.inputs, input.num_inputs);
        }else if (input.separator == SEPARATOR_PARA){
            executeParallel(input.inputs, input.num_inputs);
        } else if (input.separator == SEPARATOR_NONE){
            executeSubshell(input.inputs[0]);
        }
        // Clean up
        free_parsed_input(&input);
    }

    return 0;
}
