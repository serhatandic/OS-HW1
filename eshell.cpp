#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include "parser.h"
#include <signal.h>

void executeSubshell(single_input line);

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

    if (pid == 0) {
        // Child process. The first argument is the command to execute, rest are the arguments
        execvp(argv[0], argv);
    } else {
        // Parent process
        waitpid(pid, nullptr, 0);
    }
    
    // Free the memory
    freeCharArray(argv);
}

void executePipeline(single_input* inputs, int size){
    int pipefd[2]; // 0 is the read end, 1 is the write end
    // store pipe file descriptors
    int pipefds[size - 1][2];
    std::vector<pid_t> pids;
    for (int i = 0; i < size - 1; i++) {
        pipe(pipefds[i]);
    }

    for (int i = 0; i < size ; i++){
        pid_t pid = fork();
        pids.push_back(pid);
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
                char** argv = vectorToCharArray(inputs[i].data.cmd);
                execvp(argv[0], argv);

                // free the memory
                freeCharArray(argv);
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
    for (int i = 0; i < pids.size(); i++){
        waitpid(pids[i], nullptr, 0);
    }
}

void executePipelineForCmd(command* cmd, int size){
    // store pipe file descriptors
    int pipefds[size - 1][2];
    std::vector<pid_t> pids;

    for (int i = 0; i < size - 1; i++) {
        pipe(pipefds[i]);
    }
    for (int i = 0; i < size; i++){
        pid_t pid = fork();
        pids.push_back(pid);
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

            char** argv = vectorToCharArray(cmd[i]);
            execvp(argv[0], argv);

            // free the memory
            freeCharArray(argv);
        }
    }
    for (int i = 0; i < size - 1; i++) {
        close(pipefds[i][0]);
        close(pipefds[i][1]);
    }
    // reap
    for (int i = 0; i < pids.size(); i++){
        waitpid(pids[i], nullptr, 0);
    }
}

// dont wait for the pipeline to finish
void executePipelineForParallel(command* cmd, int size){
    int pipefd[2]; // 0 is the read end, 1 is the write end
    // store pipe file descriptors
    int pipefds[size - 1][2];
    std::vector<pid_t> pids;

    for (int i = 0; i < size - 1; i++) {
        pipe(pipefds[i]);
    }

    for (int i = 0; i < size; i++){
        pid_t pid = fork();
        pids.push_back(pid);
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

            char** argv = vectorToCharArray(cmd[i]);
            execvp(argv[0], argv);

            // free the memory
            freeCharArray(argv);
        }
    }

    for (int i = 0; i < size - 1; i++) {
        close(pipefds[i][0]);
        close(pipefds[i][1]);
    }

    // reap
    for (int i = 0; i < size; i++){
        waitpid(pids[i], nullptr, 0);
    }

}

void executeSequential(single_input* inputs, int size){
    for (int i = 0; i < size; i++){
        if (inputs[i].type == INPUT_TYPE_PIPELINE){
            executePipelineForCmd(inputs[i].data.pline.commands, inputs[i].data.pline.num_commands);
        } else if (inputs[i].type == INPUT_TYPE_COMMAND){
            pid_t pid = fork();
            if (pid == 0) {
                // Child process
                char** argv = vectorToCharArray(inputs[i].data.cmd);
                execvp(argv[0], argv);
                // free the memory
                freeCharArray(argv);
            }else{
                // Parent process
                waitpid(pid, nullptr, 0);
            }
        }
    }
}

void executeParallel(single_input* inputs, int size){
    std::vector<pid_t> pids;
    for (int i = 0; i < size; i++){
        if (inputs[i].type == INPUT_TYPE_PIPELINE){
            // append the result to the pids
            executePipelineForParallel(inputs[i].data.pline.commands, inputs[i].data.pline.num_commands);
        } else if (inputs[i].type == INPUT_TYPE_COMMAND){
            pid_t pid = fork();
            if (pid == 0) {
                // Child process
                char** argv = vectorToCharArray(inputs[i].data.cmd);
                execvp(argv[0], argv);
                // free the memory
                freeCharArray(argv);
            }else{
                // Parent process
                pids.push_back(pid);
            }
        }
    }

    // wait all pids to finish
    for (int i = 0; i < pids.size(); i++){
        waitpid(pids[i], nullptr, 0);
    }
}

void executeSubshell(single_input line){
    parsed_input input;
    
    parse_line(line.data.subshell, &input);
    std::vector<pid_t> pids;

    pid_t pid = fork();
    pids.push_back(pid);

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
            pids.push_back(pidRepeater);
            if (pidRepeater == 0) {
                for (int j = 0; j < size; j++) {
                    command cmd = input.inputs[j].data.cmd;

                    // Write each argument of the command to the pipe
                    std::string cmdStr;
                    for (int k = 0; cmd.args[k] != NULL; k++) {
                        cmdStr += cmd.args[k];
                        // Write a space or a newline after each argument
                        char separator = (cmd.args[k+1] != NULL) ? ' ' : '\n';
                        cmdStr += separator;
                    }
                    write(pipefds[j][1], cmdStr.c_str(), cmdStr.size());
                    close(pipefds[j][1]);
                    close(pipefds[j][0]);
                }
            
            }

            for (int i = 0; i < size; i++){        
                // read from the pipe
                char buffer[1024];
                int n = read(pipefds[i][0], buffer, 1024);
                close(pipefds[i][0]);
                if (n == -1) {
                    exit(1);
                }
                buffer[n] = '\0';
                parsed_input input2;
                parse_line(buffer, &input2);
                // std::cout << buffer << std::endl;
                for (int j = 0; j < input2.num_inputs; j++){
                    pid_t pid = fork();
                    pids.push_back(pid);
                    if (pid == 0) { // Child process
                        if (input2.inputs[j].type == INPUT_TYPE_PIPELINE){
                            executePipelineForCmd(input2.inputs[j].data.pline.commands, input2.inputs[j].data.pline.num_commands);
                        } else if (input2.inputs[j].type == INPUT_TYPE_COMMAND){
                            // std::cout << "executing: " << input2.inputs[j].data.cmd.args[0] << std::endl;
                            execvp(input2.inputs[j].data.cmd.args[0], input2.inputs[j].data.cmd.args);
                        }
                    }
                }
                close(pipefds[i][1]);
                free_parsed_input(&input2);
            }
        }
    }

    // reap the child processes
    for (int i = 0; i < pids.size(); i++){
        waitpid(pids[i], nullptr, 0);
    }
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
            // kill pid, it shouldn't print Killed message
            kill(pid, SIGKILL);
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
