#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include "parser.h"

void executeSubshell(single_input line);

void executeSingleCommand(const command& cmd) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execvp(cmd.args[0], cmd.args);
    }else{
        waitpid(pid, nullptr, 0);
    }

}

void executePipeline(single_input* inputs, int size){
    int pipefd[2]; // 0 is the read end, 1 is the write end
    // store pipe file descriptors
    int pipefds[size - 1][2];
    for (int i = 0; i < size - 1; i++) {
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
            // command or subshell
            if (inputs[i].type == INPUT_TYPE_COMMAND){
                executeSingleCommand(inputs[i].data.cmd);
                std::cout << "here" << std::endl;
            }else if (inputs[i].type == INPUT_TYPE_SUBSHELL){
                executeSubshell(inputs[i]);
            }
        }
    }

    for (int i = 0; i < size - 1; i++) {
        close(pipefds[i][0]);
        close(pipefds[i][1]);
    }

    // reap the child processes
    while(wait(nullptr) > 0);
}

void executePipelineForCmd(command* cmd, int size){
    // store pipe file descriptors
    int pipefds[size - 1][2];
    for (int i = 0; i < size - 1; i++) {
        pipe(pipefds[i]);
    }
    for (int i = 0; i < size; i++){
        pid_t pid = fork();
        if (pid == 0) {
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
            
            // close all pipe fds
            for (int i = 0; i < size - 1; i++) {
                close(pipefds[i][0]);
                close(pipefds[i][1]);
            }

            executeSingleCommand(cmd[i]);
        }
    }
    for (int i = 0; i < size - 1; i++) {
        close(pipefds[i][0]);
        close(pipefds[i][1]);
    }
    // reap
    while(wait(nullptr) > 0);


}

// dont wait for the pipeline to finish
void executePipelineForParallel(command* cmd, int size){
    int pipefd[2]; // 0 is the read end, 1 is the write end
    // store pipe file descriptors
    int pipefds[size - 1][2];
    for (int i = 0; i < size - 1; i++) {
        pipe(pipefds[i]);
    }

    for (int i = 0; i < size; i++){
        pid_t pid = fork();
        if (pid == 0) {
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

            // close all pipe fds
            for (int i = 0; i < size - 1; i++) {
                close(pipefds[i][0]);
                close(pipefds[i][1]);
            }

            executeSingleCommand(cmd[i]);
        }
    }

    for (int i = 0; i < size - 1; i++) {
        close(pipefds[i][0]);
        close(pipefds[i][1]);
    }

    // reap
    while(wait(nullptr) > 0);



}

void executeSequential(single_input* inputs, int size){
    for (int i = 0; i < size; i++){
        if (inputs[i].type == INPUT_TYPE_PIPELINE){
            executePipelineForCmd(inputs[i].data.pline.commands, inputs[i].data.pline.num_commands);
        } else if (inputs[i].type == INPUT_TYPE_COMMAND){
            pid_t pid = fork();
            if (pid == 0) {
                // Child process
                executeSingleCommand(inputs[i].data.cmd);
            }else{
                // Parent process
                waitpid(pid, nullptr, 0);
            }
        }
    }
}

void executeParallel(single_input* inputs, int size){
    for (int i = 0; i < size; i++){
        if (inputs[i].type == INPUT_TYPE_PIPELINE){
            executePipelineForParallel(inputs[i].data.pline.commands, inputs[i].data.pline.num_commands);
        } else if (inputs[i].type == INPUT_TYPE_COMMAND){
            pid_t pid = fork();
            if (pid == 0) {
                // Child process
                executeSingleCommand(inputs[i].data.cmd);
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
            for (int i = 0; i < size; i++) {
                pipe(pipefds[i]);
            }

            pid_t pidRepeater = fork();
            if (pidRepeater == 0) {
                for (int j = 0; j < size; j++) {
                    std::string cmdStr;
                    close(pipefds[j][0]);
                    if (input.inputs[j].type == INPUT_TYPE_COMMAND){
                        command cmd = input.inputs[j].data.cmd;
                        for (int k = 0; cmd.args[k] != NULL; k++) {
                            cmdStr += cmd.args[k];
                            // Write a space or a newline after each argument
                            char separator = (cmd.args[k+1] != NULL) ? ' ' : '\n';
                            cmdStr += separator;
                        }
                    }
                    if (input.inputs[j].type == INPUT_TYPE_PIPELINE){
                        for (int k = 0; k < input.inputs[j].data.pline.num_commands; k++){
                            command cmd = input.inputs[j].data.pline.commands[k];
                            for (int l = 0; cmd.args[l] != NULL; l++) {
                                cmdStr += cmd.args[l];
                                cmdStr += ' ';
                            }
                            if (k != input.inputs[j].data.pline.num_commands - 1){
                                cmdStr += "| ";
                            }
                        }                        
                    }

                    write(pipefds[j][1], cmdStr.c_str(), cmdStr.size());
                    close(pipefds[j][1]);
                }
            }

            for (int i = 0; i < size; i++){
                pid_t pid = fork();
                if (pid == 0){
                    close(pipefds[i][1]);
                    // read from the pipe
                    char buffer[1024];
                    int n = read(pipefds[i][0], buffer, 1024);
                    close(pipefds[i][0]);
                    if (n == -1) {
                        break;
                    }
                    buffer[n] = '\0';
                    parsed_input input2;
                    parse_line(buffer, &input2);

                    if (input2.separator == SEPARATOR_PIPE){
                        executePipeline(input2.inputs, input2.num_inputs);
                    }else if (input2.num_inputs == 1 && input2.inputs[0].type == INPUT_TYPE_COMMAND){
                        executeSingleCommand(input2.inputs[0].data.cmd);
                    }

                    free_parsed_input(&input2);
                }
            }

            // close all pipe fds
            for (int i = 0; i < input.num_inputs; i++) {
                close(pipefds[i][0]);
                close(pipefds[i][1]);
            }
        }
    }


    // reap the child processes
    while(wait(nullptr) > 0);
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
            executeSingleCommand(input.inputs[0].data.cmd);
        }else if (input.separator == SEPARATOR_PIPE){
            executePipeline(input.inputs, input.num_inputs);
        } else if (input.separator == SEPARATOR_SEQ){
            executeSequential(input.inputs, input.num_inputs);
        } else if (input.separator == SEPARATOR_PARA){
            executeParallel(input.inputs, input.num_inputs);
        } else if (input.separator == SEPARATOR_NONE){
            executeSubshell(input.inputs[0]);
        }

        // Clean up
        free_parsed_input(&input);
    }

    return 0;
}
