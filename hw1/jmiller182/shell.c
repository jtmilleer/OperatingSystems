#define _GNU_SOURCE //this is needed to be able to use execvpe
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <fcntl.h>


typedef struct {
  char* binary_path;
  char* stdin;
  char* stdout;
  char* arguments;
  short wait;
} command;

//function prototypes
void print_parsed_command(command);
short parse_command(command*, char*);

// read a line from a file
short getlinedup(FILE* fp, char** value){
  char* line = NULL;
  size_t n = 0;
  //get one line
  int ret = getline(&line, &n, fp);

  if(ret == -1){
    //the file ended
    return 0;
  }
  //remove \n at the end
  line[strcspn(line, "\n")] = '\0';
  //duplicate the string and set value
  *value = strdup(line);
  free(line);

  return 1;
}

//parse a command_file and set a corresponding command data structure
short parse_command(command* parsed_command, char* cmdfile){
  FILE* fp = fopen(cmdfile, "r");
  if(!fp){
    //the file does not exist
    return 0;
  }

  char* value;
  short ret;
  int intvalue;

  ret = getlinedup(fp,&value);
  if(!ret){
    fclose(fp); return 0;
  }
  parsed_command->binary_path = value;

  ret = getlinedup(fp,&value);
  if(!ret){
    fclose(fp); return 0;
  }
  parsed_command->stdin = value;

  ret = getlinedup(fp,&value);
  if(!ret){
    fclose(fp); return 0;
  }
  parsed_command->stdout = value;

  ret = getlinedup(fp,&value);
  if(!ret){
    fclose(fp); return 0;
  }
  parsed_command->arguments = value;

  ret = getlinedup(fp,&value);
  if(!ret){
    fclose(fp); return 0;
  }
  intvalue = atoi(value);
  if(intvalue != 0 && intvalue != 1){
    fclose(fp); return 0;
  }
  parsed_command->wait = (short)intvalue;

  return 1;
}

//useful for debugging
void print_parsed_command(command parsed_command){
  printf("----------\n");
  printf("binary_path: %s\n", parsed_command.binary_path);
  printf("stdin: %s\n", parsed_command.stdin);
  printf("stdout: %s\n", parsed_command.stdout);
  printf("arguments: %s\n", parsed_command.arguments);
  printf("wait: %d\n", parsed_command.wait);
}

void free_command(command cmd){
  free(cmd.binary_path);
  free(cmd.stdin);
  free(cmd.stdout);
  free(cmd.arguments);
}

void process_command(command cmd) {
	// Step 1 (use fork)
  int f = fork();
  if(f == 0){
    // child process
    // Step 2 (set up arguments in child)

    // (redirect input to file if stdin is not empty)
    if(cmd.stdin[0] != '\0'){
      int fd = open(cmd.stdin, O_RDONLY);
      dup2(fd, STDIN_FILENO);
      close(fd);
    }
    // (redirect output to file if stdout is not empty)
    if(cmd.stdout[0] != '\0'){
      int fd = open(cmd.stdout, O_WRONLY|O_CREAT|O_TRUNC, 0644);
      dup2(fd, STDOUT_FILENO);
      close(fd);
    }

    char* myargs[3];
    int nargs = 0;
    myargs[nargs++] = cmd.binary_path;
    if(cmd.arguments[0] != '\0'){
      myargs[nargs++] = cmd.arguments;
    }
    myargs[nargs] = NULL;

    execv(cmd.binary_path, myargs);
    printf("execv failed!\n");
    exit(1);

    // Step 3 (call execvp to replace child process context)
  } else {
    printf("New child process started %d\n", f);
    // parent process
    // Step 4 (if wait of command is 1, must wait in parent before going on to next command)
    if(cmd.wait == 1){
      int status;
      waitpid(f, &status, 0);
      printf("Child process %d terminated with exit code %d\n", f, WEXITSTATUS(status));
    }
    // Step 5 (when child process terminates, print pid of terminal process and exit status)
  }
}


int main(int argc, char *argv[], char* env[]) {

  for(int ncommand=1; ncommand<argc; ncommand++){
    command parsed_command;
    int ret = parse_command(&parsed_command, argv[ncommand]);
    if (!ret){
      printf("command file %s is invalid\n", argv[ncommand]);
      continue;
    }

    // print_parsed_command(parsed_command);

    process_command(parsed_command);
    //process_command(...);
    /*
    process_command will:
    - get a parsed_command variable
    - create a child process
    - set file redirection, arguments, envirionment variables
    - call execve or its variant
    - print when a child process is created and when any child process is terminated
    - if specified, wait for the termination of the program
    */

    free_command(parsed_command);
  }

  //remember to wait for the termination of all the child processes, regardless of the value of parsed_command.wait
  
  int status;
  pid_t pid;
  while((pid = waitpid(-1, &status, 0)) > 0){
    printf("Child process %d terminated with exit code %d\n", pid, WEXITSTATUS(status));
  }

}

