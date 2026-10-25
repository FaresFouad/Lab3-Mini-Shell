
/*
 * 
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <glob.h>
#include <wordexp.h>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include "command.h"
#include "tokenizer.h"


void ctrlc_handler(int num){
    printf("");
    
}

void log(int pid){
    int file = open("logs.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
        std::string log = std::to_string(pid) + " child has been terminated\n";
        if(file != -1){
            write(file, log.c_str(), log.length());
            close(file);
        }
}

void log_handler(int num){
    int pid;
    while((pid = waitpid(-1, NULL, WNOHANG)) > 0){ 
        log(pid);
    }
}
void parse(std::vector<Token> &tokens)
{

    //currentCommand.execute();
    /*
      Implement this function 
      you must parse for all possible tokens scenario
      right now command.execute only print the format , you will inject it in your logic.

    */
    if (tokens.empty() || tokens[0].type == TOKEN_EOF)
    {
        return;
    }
    Command::_currentSimpleCommand = new SimpleCommand();
    for(size_t i = 0; i < tokens.size(); i++){
        if(tokens[i].type == TOKEN_COMMAND || tokens[i].type == TOKEN_ARGUMENT){
            std::string arg = tokens[i].value;
            bool q = false;
            if(tokens[i].type == TOKEN_ARGUMENT && arg.front() == '"'){
                q = true;
                while(arg.back() != '"' && i + 1 < tokens.size()){
                    i++;
                    arg += " ";
                    arg += tokens[i].value; 
                }
    
                arg = arg.substr(1);
                if(!arg.empty() && arg.back() == '"'){
                    arg = arg.substr(0, arg.length() - 1);
                }
            }
            if(!q && (arg.find('*') != std::string::npos || arg.find('?') != std::string::npos)){
                glob_t glob_result;
                glob(arg.c_str(), GLOB_NOCHECK, NULL, &glob_result);
                
                for(size_t j = 0; j < glob_result.gl_pathc; j++){
                    Command::_currentSimpleCommand->insertArgument(strdup(glob_result.gl_pathv[j]));
                }
                globfree(&glob_result);
            }
            else{
                Command::_currentSimpleCommand->insertArgument(strdup(arg.c_str()));
            }
        }
        else if(tokens[i].type == TOKEN_PIPE){
            Command::_currentCommand.insertSimpleCommand(Command::_currentSimpleCommand);
            Command::_currentSimpleCommand = new SimpleCommand();
        }
        else if(tokens[i].type == TOKEN_REDIRECT){
            if(tokens[i + 1].type == TOKEN_EOF){
                perror("Error: No output file specified for redirection.");
                return;
            }
            if(tokens[i + 1].type != TOKEN_ARGUMENT){
                perror("Error: Invalid syntax.");
                return;
            }
            Command::_currentCommand._outFile = strdup(tokens[i + 1].value.c_str());
            Command::_currentCommand._append = 0;
            i++;
        }
        else if(tokens[i].type == TOKEN_APPEND){
            if(tokens[i + 1].type == TOKEN_EOF){
                perror("Error: No output file specified for appending.");
                return;
            }
            if(tokens[i + 1].type != TOKEN_ARGUMENT){
                perror("Error: Invalid syntax.");
                return;
            }
            Command::_currentCommand._outFile = strdup(tokens[i + 1].value.c_str());
            Command::_currentCommand._append = 1;
            i++;
        }
        else if(tokens[i].type == TOKEN_INPUT){
            if(tokens[i + 1].type == TOKEN_EOF){
                perror("Error: No input file specified for redirection.");
                return;
            }
            if(tokens[i + 1].type != TOKEN_ARGUMENT){
                perror("Error: Invalid syntax.");
                return;
            }
            Command::_currentCommand._inputFile = strdup(tokens[i + 1].value.c_str());
            i++;
        }
        else if(tokens[i].type == TOKEN_ERROR){
            if(tokens[i + 1].type == TOKEN_EOF){
                perror("Error: No error file specified for redirection.");
                return;
            }
            if(tokens[i + 1].type != TOKEN_ARGUMENT){
                perror("Error: Invalid syntax.");
                return;
            }
            Command::_currentCommand._errFile = strdup(tokens[i + 1].value.c_str());
            Command::_currentCommand._out_error = 0;
            i++;
        }
        else if(tokens[i].type == TOKEN_REDIRECT_AND_ERROR){
            if(tokens[i + 1].type == TOKEN_EOF){
                perror("Error: No file specified for redirection and error.");
                return;
            }
            if(tokens[i + 1].type != TOKEN_ARGUMENT){
                perror("Error: Invalid syntax.");
                return;
            }
            Command::_currentCommand._errFile = strdup(tokens[i + 1].value.c_str());
            Command::_currentCommand._outFile = strdup(tokens[i + 1].value.c_str());
            Command::_currentCommand._out_error = 1;
            i++;
        }
        else if(tokens[i].type == TOKEN_BACKGROUND){
            Command::_currentCommand._background = 1;
        }
        else if(tokens[i].type == TOKEN_EOF){
            Command::_currentCommand.insertSimpleCommand(Command::_currentSimpleCommand);
            Command::_currentCommand.execute();
            break;
        }
    }
    
   
}

SimpleCommand::SimpleCommand()
{
    _numberOfAvailableArguments = 5;
    _numberOfArguments = 0;
    _arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
    if (_numberOfAvailableArguments == _numberOfArguments + 1)
    {
        _numberOfAvailableArguments *= 2;
        _arguments = (char **)realloc(_arguments,
                                      _numberOfAvailableArguments * sizeof(char *));
    }

    _arguments[_numberOfArguments] = argument;
    _arguments[_numberOfArguments + 1] = NULL;

    _numberOfArguments++;
}

Command::Command()
{
    _numberOfAvailableSimpleCommands = 1;
    _numberOfSimpleCommands = 0;
    _simpleCommands = (SimpleCommand **)
        malloc(_numberOfSimpleCommands * sizeof(SimpleCommand *));

    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
    if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
    {
        _numberOfAvailableSimpleCommands *= 2;
        _simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
                                                    _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
    }

    _simpleCommands[_numberOfSimpleCommands] = simpleCommand;
    _numberOfSimpleCommands++;
}

void Command::clear()
{
    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            free(_simpleCommands[i]->_arguments[j]);
        }

        free(_simpleCommands[i]->_arguments);
        free(_simpleCommands[i]);
    }

    if (_outFile)
    {
        free(_outFile);
    }

    if (_inputFile)
    {
        free(_inputFile);
    }

    if (_errFile)
    {
        free(_errFile);
    }

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
}

void Command::print()
{
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        printf("  %-3d ", i);
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
        }
    }

    printf("\n\n");
    printf("  Output       Input        Error        Err&Out       Background\n");
    printf("  ------------ ------------ ------------ ------------ ------------\n");
    printf("  %-12s %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
           _inputFile ? _inputFile : "default", _errFile ? _errFile : "default", _out_error == 1 ? _errFile : "default"
           ,_background ? "YES" : "NO");
    printf("\n\n");
}

void Command::execute()
{
    /*
        Write your code here
    */
    int currentInput = 0;
    if(_numberOfSimpleCommands == 1 && std::string(_simpleCommands[0]->_arguments[0]) == "exit"){
        printf("\n  Good bye!!\n");
        exit(0);
    }
    else if(_numberOfSimpleCommands == 1 && std::string(_simpleCommands[0]->_arguments[0]) == "cd"){
        if(_simpleCommands[0]->_arguments[1] != NULL){
            if(chdir(_simpleCommands[0]->_arguments[1]) == -1){
                perror("Path does not exist");
            } 
        }
        else{
            chdir(getenv("HOME"));
        }
        clear();
        return;
    }

    print();

    for(int i = 0; i < _numberOfSimpleCommands; i++){
        int fd[2] = {-1, -1};
        if(i + 1 < _numberOfSimpleCommands){
            pipe(fd);
        }
        int pid = fork();
        if(pid == 0){
            if(i + 1 < _numberOfSimpleCommands || currentInput!= 0){
                if(i + 1 < _numberOfSimpleCommands){
                    dup2(fd[1], 1);
                }
                if(currentInput != 0){
                    dup2(currentInput, 0);
                    close(currentInput);
                }
                if(fd[0] != -1){
                    close(fd[0]);
                    close(fd[1]);
                }
                
            }
            if(_outFile && i == _numberOfSimpleCommands - 1){
                int out;
                if(_append == 0 && _out_error != 1){
                    out = open(_outFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                }
                else{
                    out = open(_outFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
                }
                    
                    dup2(out, 1);
                if(_out_error == 1){
                    dup2(out, 2);
                }
                    close(out);
            }

            if(_inputFile && i == 0){//it would have been better if _inputFile was specified in which simple command it belongs to.
                int in = open(_inputFile, O_RDONLY);
                dup2(in, 0);
                close(in);
            }

            if(_errFile && _out_error == 0 && i == _numberOfSimpleCommands - 1){
                int err = open(_errFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                dup2(err, 2);
                close(err);

            }
            if(execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments) == -1){
                perror("Error: Command does not exist.");
                exit(1);
            }
        }
        else if(pid < 0){
            perror("Error: Fork failed.");
            exit(1);
        }
        else{
            if(i + 1 < _numberOfSimpleCommands){
                currentInput = fd[0];
                close(fd[1]);
            }
            if(!_background){
                waitpid(pid, NULL, 0);
                log(pid);
           }
        }
}
    clear();
}

void Command::prompt()
{
    signal(SIGINT, ctrlc_handler);
    while (true)
    {
        printf("myshell>");
        fflush(stdout);
        std::string input;
        std::getline(std::cin, input); 
        std::vector<Token> tokens = tokenize(input);
        parse(tokens);
    }
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int main()
{
    Command::_currentCommand.prompt();
    return 0;
}
