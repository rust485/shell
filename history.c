#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "history.h"

/**
 * Initialize a history struct (already allocated)
 */
history_t *init_history(history_t *history)
{
  history->max_size = MAX_SIZE;
  history->max_line = MAX_LINE;
  history->commands = calloc(history->max_size, sizeof(char*));
  history->size = 0;
  return history;
}

/**
 * Deallocates any allocated member of the history struct,
 * and deallocates the history struct
 */
void destory_history(history_t *history)
{
  if (!history) return;
  for (uint i = 0; i < MAX_SIZE; i++)
    if (history->commands[i])
      free(history->commands[i]);
  free(history->commands);
  free(history);
}

/**
 * Adds the passed input into the history structure at
 * the least recently used index, overwriting the existing
 * history if it is already in use
 */
char *add_to_history(history_t *history, char *input)
{
  if (history->commands[history->size % history->max_size])
    free(history->commands[history->size % history->max_size]);
  history->commands[history->size % history->max_size] = malloc(sizeof(char) * history->max_line);
  strcpy(history->commands[history->size++ % history->max_size], input);
  return input;
}

/**
 * prints a single line of history, where num is the index
 * of the history to be printed
 */
void print_history_line(history_t history, int num)
{
  // if num is out of bounds, return immediately
  if (num < (history.size - history.max_size) || num >= history.size) return;
  printf("%s\n", history.commands[(num) % history.max_size]);
}

/**
 * prints the most recently added history command
 */
void print_recent_history(history_t history)
{
  // if size is 0, no history to print
  if (history.size == 0) return;
  printf("%d ", history.size);
  print_history_line(history, history.size - 1);
}

/**
 * iterates through the full history and prints out each
 * command and the number it is
 */
void print_full_history(history_t history)
{
  if (history.size == 0) return;

  int history_num = history.size;
  int iterations = history.max_size; // number of lines to print

  // if we have not yet reached max_size, decrease to the current size for iterations
  if (history.size < history.max_size) iterations = history.size;
  int i = 0;
  while (i < iterations)
  {
    printf("%d ", history_num);
    print_history_line(history, (history_num-- - 1));
    i++;
  }
}

/**
 * retrieves the history at the given index
 */
char *get_history(history_t history, int num)
{
  // if history size is 0, no history to get
  if (history.size == 0)
  {
    printf("No commands in history\n");
    return NULL;
  }

  // if num is out of legal bounds, return null
  if (num < 0 || num > history.size || num < (history.size - history.max_size)) // out of bounds
  {
    printf("No such command in history\n");
    return NULL;
  }
  char *command = calloc(history.max_line, sizeof(char));
  strcpy(command, history.commands[num % history.max_size]);

  return command;
}
