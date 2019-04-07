/****************************************************************
 * Name        :                                                *
 * Class       : CSC 415                                        *
 * Date        :                                                *
 * Description :  Writting a simple bash shell program          *
 *                that will execute simple commands. The main   *
 *                goal of the assignment is working with        *
 *                fork, pipes and exec system calls.            *
 ****************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>

#define BUFFERSIZE 256
#define PROMPT "myShell >> "
#define PROMPTSIZE sizeof(PROMPT)

char buffer[BUFFERSIZE];

/* function declaration */
void current_directory(); 
void parse(char *myargv[],int *myargc);
void clear_args(char *myargv[], int *myargc);
void clear_buffer();
void execute_command(char *myargv[]);
void cd_command(char *myargv[]);
void pwd_command(); 
void redirect_input(char *myargv[], int *myargc);
void redirect_output(char *myargv[], int *myargc);
void redirect_output_append(char *myargv[], int *myargc);
void background_command(char *myargv[], int *myargc);
void multiple_command(char *myargv[], int *myargc);
int check(char *myargv[], int *myargc, char *flag);
void execute(char *myargv[], int *myargc);


int main(int argc, char** argv)
{ 	
    char *myargv[128];
    int myargc = 0;

   	while(true){ 
      current_directory(); // Always show the current directory before PROMPT
   		printf("%s", PROMPT);

   		clear_buffer();
   		// Exit myshell until detects EOF(CTL-D)
   		if(fgets(buffer, BUFFERSIZE, stdin) == NULL){
        printf("\n");
   			return 0;
   		}

      clear_args(myargv, &myargc);
      parse(myargv, &myargc);
      execute(myargv, &myargc); 		
   	}   
return 0;
}

/* Functions*/
void current_directory(){
  char cwd[256];
  if(getcwd(cwd, sizeof(cwd)) == NULL){
    printf("getcwd() error!");
  }
  else{
    char *last_token = strrchr(cwd, '/');
    if(last_token != NULL){
      printf("%s ", last_token+1);
    }   
  }
}

void clear_args(char *myargv[], int *myargc){
 	for(int i = 0; i < *myargc; i++){
    myargv[i] = NULL;
  }
  *myargc = 0;
}

void clear_buffer(){
  for (int i = 0; i < BUFFERSIZE; i++){
        buffer[i] = '\0';
      }   
}

void parse(char *myargv[],int *myargc){
  // parse commamnd line
  char *token = strtok(buffer, " \n");  
  while(token != NULL){ 
    myargv[*myargc] = token;
    (*myargc)++;
    token = strtok(NULL, " \n");  
  }
  myargv[*myargc] = NULL;
  (*myargc)++;
  // // print sub-strings for testing!
  // for(int i = 0; i < *myargc ; i++){
  //   printf("The sub-strings in myargv[%d]: %s\n", i, myargv[i]);
  // }
  // printf("%d\n", *myargc);
}

void execute_command(char *myargv[]){
	pid_t pid;
  int status;

  pid = fork(); // fork a child process

  if (pid < 0) {
  /* error occurred */ 
    printf("Forking child process error!!!\n"); 
    exit(1);
  } 
  /* child process */
  else if (pid == 0) { 
    write(1, "In child!\n", 11);
    execvp(myargv[0], myargv);    
  }
  /* parent process */
  /* parent will wait for the child to complete */
  else {
    do{
      printf("Child process Complete and back to parent:\n");
      waitpid(pid, &status, WUNTRACED);
    }while (!WIFEXITED(status) && !WIFSIGNALED(status));
   } 
}

void cd_command(char *myargv[]){
  if(myargv[1] == NULL ){
    printf("cd command needs arguments!\n");
  }
  else{
    if(chdir(myargv[1]) == -1){
      printf(" %s: no such directory or file\n", myargv[1]);
    }
    else{
      printf("cd: %s\n", myargv[1]);
    }
  }    
}

void pwd_command(){
  char cwd[256];
  if(getcwd(cwd,sizeof(cwd)) == NULL){
    printf("getcwd() error");
  }
  else{
    printf("%s\n", cwd);
  }
}

void redirect_input(char *myargv[], int *myargc){

  int stdin_fd = dup(STDIN_FILENO); // make a fd copy of stdin
  char *outfile = myargv[(*myargc)-2];
  int fd = open(outfile, O_RDONLY);
  if(fd == -1) {
    printf("Error open the file!\n");
  }
  else {
    dup2(fd, STDIN_FILENO);
  }
  int i = *myargc - 3;
  while(myargv[i] != NULL) {
    myargv[i] = NULL;
    i++;
  }
  execute_command(myargv);
  close(fd);
  dup2(stdin_fd, STDIN_FILENO);
  }

void redirect_output(char *myargv[], int *myargc){
 
  int stdout_fd = dup(STDOUT_FILENO); // make a fd copy of stdout
  char *outfile = myargv[(*myargc)-2];
  int fd = open(outfile, O_RDWR | O_CREAT | O_TRUNC, 0666);
  if(fd == -1) {
    printf("Error open the file!\n");
  }
  else {
    dup2(fd, STDOUT_FILENO);
  }
  int i = *myargc - 3;
  while(myargv[i] != NULL) {
    myargv[i] = NULL;
    i++;
  }
  execute_command(myargv);
  close(fd);
  dup2(stdout_fd, STDOUT_FILENO);
 
  }

void redirect_output_append(char *myargv[], int *myargc){
  
  int stdout_fd = dup(STDOUT_FILENO); // make a fd copy of stdout
  char *outfile = myargv[(*myargc)-2];
  int fd = open(outfile, O_WRONLY | O_CREAT | O_APPEND, 0666);
  if(fd == -1) {
    printf("Error open the file! %i \n", fd);
  }
  else {
    dup2(fd, STDOUT_FILENO);
    close(fd);
  }

  int i = *myargc - 3;
  while(myargv[i] != NULL) {
    myargv[i] = NULL;
    i++;
  }
  execute_command(myargv);
  dup2(stdout_fd, STDOUT_FILENO);  
  }

void background_command(char *myargv[], int *myargc){
  int status;
  myargv[(*myargc)-2] = NULL;
  pid_t pid;
  pid = fork(); // fork a child process

  if (pid < 0) {
  /* error occurred */ 
    printf("Forking child process error!!!\n"); 
    exit(1);
  } 
  /* child process */
  else if (pid == 0){
    execvp(myargv[0], myargv);    
  }
  /* parent process */
  /* parent will not wait for the child to complete */
  else {
    do{
      waitpid(pid, &status, WNOHANG); // WNOHANG flag means don't wait
    }while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
}

void multiple_command(char *myargv[], int *myargc){
   /* • The array pipe_fd is used to return
       two file descriptors referring to the ends of the pipe
     • pipefd[0] refers to the read end of the pipe.
     • pipefd[1] refers to the write end of the pipe.
  */
  char *myargv1[128], *myargv2[128], *myargv3[128];
  int myargc1 = 0, myargc2 = 0, myargc3 = 0;
  int pipe_fd[2];
  pid_t pid[2];
  int status;
  int index;
  
  /* 
    First, find the index of "|", 
    divide the left and right of "|" into two char arrays
  */ 
  for(int i = 0; i < (*myargc)-1; i++){ 
    if(strcmp(myargv[i], "|") == 0){
      index = i;
    }
  }

  for(int i = 0; i < (*myargc)-1; i++){
    if(i < index){
      myargv1[myargc1] = myargv[i];
      myargc1++;
    }
    
    if(i > index){
      myargv2[myargc2] = myargv[i];
      myargc2++;   
    }    
  }

  myargv1[myargc1] = NULL;
  myargc1++;
  myargv2[myargc2] = NULL;
  myargc2++;

  // use pipe() to create pipe
  if(pipe(pipe_fd) < 0){
    printf("pipe error!");
    exit(1);
  }

  pid[0] = fork();
  if(pid[0] < 0){
    printf("Fork error!");
    exit(1);
  }
  if(pid[0] == 0){ // child
    close(pipe_fd[0]);
    dup2(pipe_fd[1], STDOUT_FILENO);
    close(pipe_fd[1]);
    execvp(myargv1[0], myargv1); 
  }
  else{ // parent 
    pid[1] = fork();
    if(pid[0] < 0){
    printf("Fork error!");
    exit(1);
  }
  if(pid[1] == 0){ 
    close(pipe_fd[1]);
    dup2(pipe_fd[0], STDIN_FILENO);
    close(pipe_fd[0]);
    execvp(myargv2[0], myargv2); 
   }
   else{
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    waitpid(pid[1], &status, 0);
   }
  }
}

int check(char *myargv[], int *myargc, char *flag){

  int check_flag = false;
  for(int i = 0; i < (*myargc - 1); i++){
    if(strcmp(myargv[i], flag) == 0){
      check_flag = true;
    }
  }
  return check_flag;
}

void execute(char *myargv[], int *myargc){
  if(strncmp("exit", myargv[0], 4) == 0){
    exit(1);
  } 
  else if(strncmp("cd", myargv[0], 2) == 0){
    cd_command(myargv);
  }
  else if(strncmp("pwd", myargv[0], 3) == 0){
    pwd_command();
  }
  else if(check(myargv, myargc, "&") == true){
   // printf("testing!");
    background_command(myargv, myargc);
  }
  else if(check(myargv, myargc, ">") == true){
    redirect_output(myargv, myargc);
  }
  else if(check(myargv, myargc, ">>") == true){
    redirect_output_append(myargv, myargc);
  }
  else if(check(myargv, myargc, "<") == true){
    redirect_input(myargv, myargc);
  }
  else if(check(myargv, myargc, "|") == true){
    multiple_command(myargv, myargc);
  }
  else{ 
    execute_command(myargv); 
  } 
}