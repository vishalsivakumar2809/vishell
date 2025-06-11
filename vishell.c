#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <sys/resource.h>

#define LINE_LENGTH 1024 // Max # of characters in an input line
#define MAX_ARGS 5      // Max number of arguments to a command
#define MAX_LENGTH 20   // Max # of characters in an argument
#define MAX_BG_PROC 1   // Max # of processes running in the background
#define DELIMITERS " \"'" // Delimiters to be removed
#define SLASH "/"         // Slash delimiter used later

pid_t bg_pids[MAX_BG_PROC]; // Array to store background process IDs
pid_t sp_pids[MAX_BG_PROC]; // Array to store suspended process IDs
pid_t fg_pid = -1; // Foreground process ID
int bg_count = 0; // Count of background processes
int sp_count = 0; // Count of suspended processes

// Signal handler for SIGINT and SIGTSTP
void signal_callback_handler(int signum) {
  if (signum == SIGINT || signum == SIGTSTP) {
    printf("\n");
  }
}

// Check the status of background and suspended processes
void check_background_suspended_processes() {
  int status;
  for (int i = 0; i < bg_count; i++) {
    pid_t result = waitpid(bg_pids[i], &status, WNOHANG);
    if (result == 0) {continue;} 
    else if (result == -1) {perror("waitpid");} 
    else {
      // Process has finished
      bg_count--;
      i--; // Adjust index to check the next process
    }
  }

  for (int i = 0; i < sp_count; i++) {
    pid_t result = waitpid(sp_pids[i], &status, WNOHANG);
    if (result == 0) {continue;} 
    else if (result == -1) {perror("waitpid");} 
    else {
      // Process has finished
      sp_count--;
      i--; // Adjust index to check the next process
    }
  }
}

// Terminate all background processes
void terminate_background_processes() {
  int status;
  for (int i = 0; i < bg_count; i++) {
    kill(bg_pids[i], SIGTERM);
    waitpid(bg_pids[i], &status, 0);
  }
  if (bg_count > 0) {bg_count = 0;}
}

// Terminate all suspended processes
void terminate_suspended_processes() {
  int status;
  for (int i = 0; i < sp_count; i++) {
    kill(sp_pids[i], SIGTERM);
    waitpid(sp_pids[i], &status, 0);
  }
  if (sp_count > 0) {sp_count = 0;}
}

// Check if a program has finished and update suspended process list
void check_if_program_finished(pid_t pid, int status){
  if (WIFSTOPPED(status) && sp_count == 0) {
    sp_pids[sp_count] = pid;
    sp_count++;
  } 
}

// Initialize the shell
void initialize(){
  struct sigaction sa;
  sa.sa_flags = 0;

  sigemptyset(&sa.sa_mask);

  sa.sa_handler = signal_callback_handler;

  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTSTP, &sa, NULL);

  printf("\nWelcome to Vishell!\n");
  printf("        ______    \n");
  printf(" .'/,-Y\"     \"~-.  \n");
  printf(" l.Y             ^.           \n");
  printf(" /\\               _\\_      \"Doh!\"   \n");
  printf("i            ___/\"   \"\\\n");
  printf("|          /\"   \"\\   o !   \n");
  printf("l         ]     o !__./   \n");
  printf(" \\ _  _    \\.___./    \"~\\  \n");
  printf("  X \\/ \\            ___./  \n");
  printf(" ( \\ ___.   _..--~~\"   ~`-.  \n");
  printf("  ` Z,--   /               \\    \n");
  printf("    \\__.  (   /       ______) \n");
  printf("      \\   l  /-----~~\" /      -Row\n");
  printf("       Y   \\          / \n");
  printf("       |    \"x______.^ \n");
  printf("       |           \\    \n");
  printf("       j            Y\n\n");
}

/**
 * @brief Tokenize a C string 
 * 
 * @param str - The C string to tokenize 
 * @param delim - The C string containing delimiter character(s) 
 * @param argv - A char* array that will contain the tokenized strings
 */
int tokenize(char* str, const char* delim, char ** argv){
  char* token;
  int count = 0;
  token = strtok(str, delim);
  for(size_t i = 0; token != NULL; ++i){
    argv[i] = token;
    count++;
    token = strtok(NULL, delim);
  }
  return count;  
}

int main(int argc, char *argv[]){
  struct rusage usage;
  struct rusage child_usage;

  getrusage(RUSAGE_SELF, &usage);
  initialize();

  while (1) {    
    char input[LINE_LENGTH];

    // Reset the input array to null
    memset(input, '\0', LINE_LENGTH);

    // Allocate memory for tokens
    char* tokens[MAX_ARGS * 3] = {NULL};
    
    printf("vishell > ");

    if (fgets(input, LINE_LENGTH, stdin) != 0) {
      size_t len = strlen(input);

      // resetting the newline character (if exists) to null character
      if (len > 0 && input[len - 1] == '\n') {input[len - 1] = '\0';}
    }

    int count = tokenize(input, DELIMITERS, tokens);
    char path[LINE_LENGTH];

    // we get the path, and make sure there's no error with it
    if (getcwd(path, LINE_LENGTH) == NULL) {perror("getcwd() error");}

    // if user's inputted no arguments, we reset the program
    if (count == 0) {continue;}

    // checking if any background processes have finished
    check_background_suspended_processes();

    // checking if user input is pwd
    if (strcmp(tokens[0], "pwd") == 0) {printf("%s\n", path);}

    // checking if user input is cd
    else if (strcmp(tokens[0], "cd") == 0) {

      // checking if number of tokens is 2, otherwise too few arguments
      if (count == 1) {printf("vishell: Expected argument to \"cd\"\n");}

      // changing directory if we can change it, otherwise displaying error message
      else {
        if (chdir(tokens[1]) == 0) {continue;}
        else {printf("vishell: No such file or directory\n");}
      }
    }

    // checking if user input is exit
    else if (strcmp(tokens[0], "exit") == 0) {
      terminate_background_processes();
      terminate_suspended_processes();

      getrusage(RUSAGE_SELF, &usage);
      getrusage(RUSAGE_CHILDREN, &child_usage);

      double total_user_time = (double)usage.ru_utime.tv_sec + (double)usage.ru_utime.tv_usec / 1000000 +
                                (double)child_usage.ru_utime.tv_sec + (double)child_usage.ru_utime.tv_usec / 1000000;

      double total_system_time = (double)usage.ru_stime.tv_sec + (double)usage.ru_stime.tv_usec / 1000000 +
                               (double)child_usage.ru_stime.tv_sec + (double)child_usage.ru_stime.tv_usec / 1000000;

      printf("User time: %lf\n", total_user_time);
      printf("System time: %lf\n", total_system_time);

      break;
    }

    // otherwise, we start a new process to execute the command
    else {
      // File descriptors for input and output redirection
      int file_desc = -1; 
      int file_desc1 = -1;
      
      // Save the current stdin and stdout file descriptors
      int saved_stdin = dup(STDIN_FILENO);
      int saved_stdout = dup(STDOUT_FILENO);
      
      // Flag to check if there are multiple commands (piping)
      int multiple_commands = 0;

      // Loop through the tokens to check for redirection or piping
      for (int i = 0; i < count; i++) {
        // Check for input redirection
        if (strcmp(tokens[i], "<") == 0) {
          // Ensure there is a file specified for input redirection
          if (tokens[i + 1] == NULL) {
            printf("failure in input\n");
            continue;
          }
          // Open the file for reading
          file_desc = open(tokens[i + 1], O_RDONLY);
          if (file_desc < 0) {
            printf("Error opening the file\n"); 
            continue;
          } 
          else {
            // Redirect stdin to the file
            dup2(file_desc, STDIN_FILENO);
          }
          // Nullify the token to avoid passing it to execve
          tokens[i] = NULL;
        } 
        // Check for output redirection
        else if (strcmp(tokens[i], ">") == 0) {
          // Ensure there is a file specified for output redirection
          if (tokens[i + 1] == NULL) {
            printf("failure in input\n");
            continue;
          }
          // Open the file for writing (create if it doesn't exist, truncate if it does)
          file_desc1 = open(tokens[i + 1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
          if (file_desc1 < 0) {
            printf("Error opening the file\n"); 
            continue;
          } 
          else {
            // Redirect stdout to the file
            dup2(file_desc1, STDOUT_FILENO);
          }
          // Nullify the token to avoid passing it to execve
          tokens[i] = NULL;
        }
        // Check for piping
        else if (strcmp(tokens[i], "|") == 0) {
          multiple_commands = 1;
        }
      }
      
      // If there are multiple commands, handle piping
      if (multiple_commands) {
        int encounter = 0;
        int status1;
        int status2;
        char *argument1[MAX_ARGS] = {NULL};
        char *argument2[MAX_ARGS] = {NULL};
        
        // Split the tokens into two sets of arguments for the two commands
        for (int i = 0; i < count; i++) {
          if (strcmp(tokens[i], "|") != 0 && !encounter) {
            argument1[i] = tokens[i];
          }
          else if (strcmp(tokens[i], "|") == 0) {
            encounter = i;
          }
          else {
            argument2[i - encounter - 1] = tokens[i];
          }
        }
        
        int fd[2];
        // Create a pipe
        if (pipe(fd) == -1) {
          perror("pipe() failed");
        }
        pid_t pid1 = fork();

        if (pid1 < 0) {
          perror("fork() failed");
        }
        // First child process to execute the first command
        else if (pid1 == 0) {
          // Redirect stdout to the write end of the pipe
          dup2(fd[1], STDOUT_FILENO);
          close(fd[0]);
          close(fd[1]);
          // Execute the first command
          status1 = execve(argument1[0], argument1, NULL);
          if (status1 == -1) {
            perror("vishell: Command not found");
          }
          _exit(EXIT_FAILURE);
        }
        else {
          pid_t pid2 = fork();
          if (pid2 < 0) {
            perror("fork() failed");
          }
          // Second child process to execute the second command
          else if (pid2 == 0) {
            // Redirect stdin to the read end of the pipe
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
            close(fd[1]);
            // Execute the second command
            status2 = execve(argument2[0], argument2, NULL);
            if (status2 == -1) {
              perror("vishell: Command not found");
            }
            _exit(EXIT_FAILURE);
          }

          // Close pipe file descriptors in the parent process
          close(fd[0]);
          close(fd[1]);
          // Check if the child processes have finished
          check_if_program_finished(pid1, status1);
          check_if_program_finished(pid2, status2);
          // Wait for the child processes to finish
          waitpid(pid1, &status1, WUNTRACED);
          waitpid(pid2, &status2, WUNTRACED);
        }
      }
      // If there are no multiple commands, handle single command execution
      else {
        pid_t pid = fork();
        int status; 
        int background_process = 0;

        // Check if the last token is "&" for background execution
        if (strcmp(tokens[count - 1], "&") == 0) {
          tokens[count - 1] = NULL; // Remove "&" from the arguments
          background_process = 1;
        }

        if (pid == 0) {
          // Child process to execute the command
          status = execvp(tokens[0], tokens);
          if (status == -1) {
            perror("vishell: Command not found");
          }
          _exit(EXIT_FAILURE);
        } 
        else if (pid > 0) {
          // Parent process
          if (!background_process) {
            fg_pid = pid;
            waitpid(pid, &status, WUNTRACED);
            fg_pid = -1;
            check_if_program_finished(pid, status);
          }

          // Close file descriptors if they were opened
          if (file_desc >= 0) {
            close(file_desc);
          }
          if (file_desc1 >= 0) {
            close(file_desc1);
          }

          // Restore the original stdin and stdout
          dup2(saved_stdout, STDOUT_FILENO);
          dup2(saved_stdin, STDIN_FILENO);
          close(saved_stdout);
          close(saved_stdin);

          // Handle background process
          if (background_process) {
            if (bg_count < MAX_BG_PROC) {
              bg_pids[bg_count++] = pid;
            }
            printf("PID %d is sent to background.\n", pid);
          } 
        } 
        else {
          perror("fork() failed.");
        }
      }
    }
  }
  return 0;
}
