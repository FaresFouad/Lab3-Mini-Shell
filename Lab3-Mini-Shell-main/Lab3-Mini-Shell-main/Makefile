## you have command.cc command.h tokenizer.cc tokenizer.h you need compile them and link
## them to one object file called  : "myshell" 
## so we should run make , that will create our object myshell ,
## ./myshell should run your application 

CC = g++
CXXFLAGS = -Wall -g -std=c++11The final executable nameTARGET = myshellDefault rule to build the executableall: $(TARGET)Link the object files to create the executable$(TARGET): command.o tokenizer.o
$(CC) $(CXXFLAGS) -o$(TARGET) command.o tokenizer.oCompile command.cc into an object filecommand.o: command.cc command.h
$(CC)$(CXXFLAGS) -c command.ccCompile tokenizer.cc into an object filetokenizer.o: tokenizer.cc tokenizer.h
$(CC)$(CXXFLAGS) -c tokenizer.ccClean up built filesclean:
rm -f *.o $(TARGET)
