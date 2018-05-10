#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdint.h>

#include <ctype.h>


#include "history.h"
#include "utils.h"

char *HOME;
char *directory;
char *last_directory;
size_t dir_size;

/**
 * Changes the current directory that you are in to the directory to the string in
 * args[1], args[0] is the command "cd". size is needed for the getcwd method.
 * returns 1 if executed properly
 * returns 0 if execution errors
 */
int change_directory(char *args[], size_t size)
{
  if (args[1])
  {
    // switch to home directory, stored in HOME
    if (strcmp(args[1], "~") == 0)
    {
      if (!HOME || chdir(HOME) == -1)
      {
        printf("Home directory not found\n");
        return 0;
      }
    }
    // switch to previous directory, stored in last_directory
    else if (strcmp(args[1], "-") == 0)
    {
      if (!last_directory || chdir(last_directory) == -1)
      {
        printf("Last directory not found\n");
        return 0;
      }
    }
    // if there is an error with chdir, print out an error
    else if(chdir(args[1]) == -1)
    {
      printf("Directory does not exist: %s\n", args[1]);
      return 0;
    }

    strcpy(last_directory, directory);
    getcwd(directory, size); // get current directory
    return 1;
  }
  else
  {
    printf("Missing directory\n");
    return 0;
  }
}

/**
 * executes the command passed by input and updates the history with the given command
 */
uint8_t execute(history_t *history, char *input)
{
  int pipe_fd[2];
  char *args[MAX_LINE / 2 + 1];
  int status, args_size, pipec, last_was_piped, should_pipe;
  int error = 0;
  int semic = 0;
  uint8_t should_wait = 1;
  uint8_t should_run = 1;
  pid_t pid;

  // validate the input
  if (!validate_input(input))
    return should_run;

  char *semi_tok; // tokenized by ';'
  while (should_run && (semi_tok = tokenize(input, ';', &semic)))
  {
    pipec = last_was_piped = 0;

    char *tmp = trim(semi_tok);
    free(semi_tok);
    semi_tok = tmp;

    char *pipe_tok = tokenize(semi_tok, '|', &pipec); // tokenized by pipes

    pipe(pipe_fd);

    // iterate over input segments delimited by '|'
    while (should_run && pipe_tok)
    {
      int spacec;
      should_pipe = args_size = spacec = 0;
      reset_args(args); // set all indicies of args to null
      char *space_tok;

      char *tmp = trim(pipe_tok);
      free(pipe_tok);
      pipe_tok = tmp;

      // execute from history
      if (pipe_tok[0] == '!' && pipe_tok[1])
      {
        int num = 0;
        // execute most recent history
        if (pipe_tok[1] == '!') num = history->size;
        else
        {
          // get number after the !
          char c;
          for (int i = 1; (c = pipe_tok[i]) != ' ' && c != '\0'; i++)
          {
            if (!isdigit(c))
            {
              printf("Error, not a number\n");
              error = 1;
              break;
            }
            else
            {
              num *= 10;
              num += c - '0';
            }
          }
        }
        if (!error)
        {
          free(pipe_tok);
          if (!(pipe_tok = get_history(*history, num - 1)))
          {
            error = 1;
            break;
          }
          printf("%s\n", pipe_tok); // print out the retrieved command
          char *tmp = calloc(MAX_LINE, sizeof(char)); // represents everything after the |

          int lock = 1;
          int j = 0;
          for (int i = 0; i < strlen(semi_tok); i++)
          {
            if (semi_tok[i] == '|' && lock)
            {
              lock = 0;
              tmp[j++] = ' ';
            }
            if (!lock) tmp[j++] = semi_tok[i];
          }
          free(semi_tok);
          semi_tok = calloc(MAX_LINE, sizeof(char));
          // make semi_tok into <history command> | <end of command>
          strcat(semi_tok, pipe_tok);
          strcat(semi_tok, tmp);


          pipec = 0;
          pipe_tok = tokenize(semi_tok, '|', &pipec);

          free(tmp);
        }
      }

      if (error) break;

      // tokenize by space and place tokens into args
      while ((space_tok = tokenize(pipe_tok, ' ', &spacec)))
      {
        // if there is an ampersand, ignore it and set should_wait to 0
        if (strcmp(space_tok, "&") != 0)
        {
          args[args_size] = calloc(MAX_LINE, sizeof(char));
          strcpy(args[args_size++], space_tok);
        }
        else should_wait = 0;
        free(space_tok);
      }

      free(pipe_tok);
      // if there is another argument after this pipe token, that means this
      // tokens output should be piped for the next one
      if ((pipe_tok = tokenize(semi_tok, '|', &pipec)))
        should_pipe = 1;
      else
        close(pipe_fd[1]);

      // if this is the last argument, then add the semicolon delimited command
      // to the history
      if (!should_pipe)
        add_to_history(history, semi_tok);

      // change directory
      if (args[0] && strcmp(args[0], CD) == 0)
      {
        // should not have a pipe or another argumnet after cd <path>
        if (should_pipe || args[2])
        {
          printf("Invalid argument after change directory\n");
          error = 1;
          break;
        }
        change_directory(args, dir_size);
        for (int i = 0; i < args_size; i++)
          free(args[i]);
        free(semi_tok);
        free(pipe_tok);
        return should_run;
        break;
      }
      // print history
      else if (args[0] && strcmp(args[0], HISTORY) == 0)
      {
        // should be no argument after history
        if (args[1])
        {
          printf("Invalid argument after history\n");
          error = 1;
          break;
        }
        print_full_history((*history));
        break;
      }


      // fork
      if ((pid = fork()) == -1)
      {
        printf("Error: fork failed\n");
        exit(-1);
      }
      else if (pid == 0)
      {
        should_run = 0;

        // overlay pipes if needed
        if (should_pipe) dup2(pipe_fd[1], 1); // if there is a piped section after this command, write to pipe
        else close(pipe_fd[1]);
        if (last_was_piped) dup2(pipe_fd[0], 0); // if the last command was piped, read from the pipe
        else close(pipe_fd[0]);
        // execute
        if (execvp(args[0], args) == -1)
        {
          printf("Invalid command: %s\n", args[0]);
          should_run = 0;
        }

        // close pipes
        if (last_was_piped) close(pipe_fd[0]);
        if (should_pipe) close(pipe_fd[1]);
      }
      else if (pid > 0)
      {
        // wait if there was an ampersand
        if (should_wait)
          waitpid(pid, &status, 0);
        last_was_piped = should_pipe; // last piped is now the current pipe
        if (!last_was_piped) close(pipe_fd[0]);


      }
    }
    for (int i = 0; i < args_size; i++)
      free(args[i]);
    free(pipe_tok);
    free(semi_tok);
    if (error) break;
  }
  return should_run;
}

int main(int argc, char *argv[])
{
  HOME = getenv("HOME"); // get home directory
  char *input;
  char *trimmed_in;
  uint8_t should_run = 1;

  // get and set up current path
  dir_size = (size_t) pathconf(".", _PC_PATH_MAX);
  directory = malloc(dir_size);
  last_directory = malloc(dir_size);
  getcwd(directory, dir_size);

  // initialize history
  history_t *history = malloc(sizeof(history_t));
  init_history(history);

  while (should_run)
  {
    // print current directory in bold blue followed by normal '$'
    printf(BOLD_BLUE "%s" NO_ATTR "$ ", directory);
    fflush(stdout);

    input = get_input();

    if (!(trimmed_in = trim(input)))
    {
      free(input);
      continue;
    }

    // if the user entered exit, exit the loop and terminate function
    if (strcmp(trimmed_in, EXIT) == 0)
    {
      should_run = 0;
      free(trimmed_in);
      break;
    }

    // execute the passed function
    should_run = execute(history, trimmed_in);
    free(input);
    free(trimmed_in);
  }

  // clean up
  if (!should_run)
  {
    destory_history(history);
    free(directory);
    free(last_directory);
  }
  return 0;
}
