#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAXLINE 80 /* The maximum length command */

char** parse(char* s) {
  static char* words[MAXLINE];
  memset(words, 0, sizeof(words));
  
  char break_chars[] = " \t\n&<>|";

  int i = 0;
  char* p = strtok(s, break_chars);
  words[i++] = p;
  while (p != NULL) {
    p = strtok(NULL, break_chars);
    words[i++] = p;
  }
  return words;
}

int check_input(char* buf){ //assume input is in proper format
  char* key[4] = {"&", "<", ">", "|"};
  
  for(int j = 0; j < 3; j++){
      if(strstr(buf, key[j])!= NULL){ return ++j;} //printf("\nfound %s", key[j]);
  }

   return 0;
  }

void do_output(char** input, int state){

  int pid = fork();                 //fork a child process using fork()
  int fd[2];
  pipe(fd);      

    if(pid==0){                     //Child

      if(state == 2){//input has >
        fd[0] = open(input[1], O_RDONLY, 0);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]); //0 = read end
      }

      if(state == 3){//input has <
        fd[1] = open(input[1], O_WRONLY, 0);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]); //1 = write end
      }

      if(state == 4){//input has |
        int fd1[2];

        if(pipe(fd1) == -1){
          fprintf(stderr, "\nError: pipe failed");
          return;
        }

        //int pid_pipe = fork();

        if(pid > 0){
          wait(NULL);
          close(fd1[1]);
          dup2(fd1[0], STDIN_FILENO);
          close(fd1[0]);
          if(execvp(input[0],input) == -1){
            fprintf(stderr, "\nError: invalid input");
          }
        }

      }

        execvp(input[0],input);
        
        fprintf(stderr, "\nError: invalid input");
    }else{                          //Parent
        if (state != 1) wait(NULL); //Wait for child process to terminate if input has &
    }
  return;
}

void print_input(char ** input){//used for debugging
  for(int i = 0; i < MAXLINE; i++){
    if(input[i]!=NULL) printf("|%s|\n", input[i]);
    else break;
  }
}

int main(int argc, char * argv[]) {
  char buf[MAXLINE];
  char bufhist[MAXLINE];
  char *args[MAXLINE/2 + 1];    /* command line arguments */
  char *hist[MAXLINE/2 + 1];
  
  int shouldrun = 1;            /* flag to determine when to exit program */
  int state = 0;                //0 = "idlestate" 1 = "&state" 2 = "<state" 3 = ">state" 4 = "|state"
  
  int fd[2];
  pipe(fd);


  if(argc > 1) {
    int curr_index, i;
    
    for (i=1, curr_index = i-1; i < (argc); curr_index = ++i-1) { 
      args[curr_index] = argv[i];
    }
    
    args[curr_index] = NULL; 
    //execvp(args[0], args);
    do_output(args, state);
  }

  else{
    int hist_len = 0;

    while (shouldrun) {
    printf("\nosh>");
  
    fgets(buf, MAXLINE, stdin);

    state = check_input(buf);

    if(strstr(buf, "!!\n") == NULL){ //make history
      strcpy(bufhist,buf);
      char **hist = parse(bufhist); //despite bufhist being a copy of buf, the values of hist changes when buf is parsed
      hist_len = 1;
      //print_input(hist);
    }

    char **args = parse(buf);

    if(args[0]==NULL) continue;

    if(strstr(args[0], "exit()") || strstr(buf, "exit()\n")) shouldrun = 0; //does not work after invalid inputs, only correct ones
    else if(strstr(args[0], "!!")){ 
      if(hist_len == 0) printf("no commands in history");
      else {
        //print_input(hist);
        do_output(hist, state);
      }
    }
    else { 
     do_output(args, state);
    }
  

  fflush(stdout);
 }
 }
 
 return 0;
} 