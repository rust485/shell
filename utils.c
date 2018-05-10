#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "utils.h"

// returns true if passed character should be trimmed
#define should_trim(c) (c == ' ' || c == '\n' || c == '\t')

/**
 * command should already be trimmed!
 * RULES:
 *  1. Command should not start with a semicolon
 *  2. An ampersand must be at the end of the list of commands:
 *    i.e. cal; who &; ps; and cal; who; ps &; is illegal, should be cal; who; ps &
 */
int validate_input(char *command)
{
  char c;
  int last_was_semi = 0;
  int amp = 0;
  // empty or semicolon at start
  if (command[0] == ';')
  {
    printf("Invalid ; at position start of input\n");
    return 0;
  }
  for (int i = 0; (c = command[i]) != '\0'; i++)
  {
    if (c == '&' && !amp)
    {
      amp = 1;
      continue;
    }
    else if (c == ';' && !last_was_semi)
    {
      last_was_semi = 1;
      continue;
    }
    else if (c == ';' && last_was_semi)
    {
      printf("Semicolons should not be entered in a row\n");
      return 0;
    }
    if (amp && c != ' ')
    {
      printf("Invalid &: not at end of input: %c\n", c);
      return 0;
    }
    if (last_was_semi && c != ' ')
    {
      last_was_semi = 0;
    }
  }

  return 1;
}

// resets the args array so all arguments are null
void reset_args(char *args[])
{
  for (int i = 0; i < MAX_LINE/2 + 1; i++)
    args[i] = NULL;
}

/** trims \n, \t, and ' ' from the beginning and end of the passed string
 *  returns null if there is no string left after trimming
 */
char *trim(char *str)
{
  char *trimmed;
  int t = 0;
  int i = 0;
  while (i < strlen(str) && should_trim(str[i])) i++;
  int j = strlen(str) - 1;

  while (j >= 0 && should_trim(str[j])) j--;

  if (i > j) return NULL;
  trimmed = calloc(j - i + 2, sizeof(char)); // [i, j] inclusive + 1 for '\0'
  for (; i <= j; i++)
    trimmed[t++] = str[i];
  trimmed[t] = '\0';
  return trimmed;
}

/** starts at start, iterates through string, stops when
 *  a character matching delim, then it returns the string
 *  between start and that delim and sets start to the position
 *  after the delim (or after a chain of the delim)
 */
char *tokenize(char *str, char delim, int *start)
{
  // if start is passed than last legal index of str, return null
  if (str && (*start) >= strlen(str)) return NULL;

  char *token = calloc(strlen(str) + 1, sizeof(char));

  int j;
  int i = (*start);

  while (i < strlen(str) && str[i] == delim) i++; // trim delim at start

  // copy up until delim detected
  for (j = 0; i < strlen(str) && str[i] != '\0'; i++, j++)
  {
    if (str[i] == delim) break;
    token[j] = str[i];
  }
  token[j] = '\0'; // append EOF
  while (i < strlen(str) && str[i] == delim) i++; // trim delim at end

  (*start) = i; // next time, the start will be at the point we just stopped at
  return token;
}

/**
 * returns a malloced string from user input
 */
char *get_input()
{
  char *in = calloc(MAX_LINE, sizeof(char));
  fgets(in, MAX_LINE, stdin);
  return in;
}
