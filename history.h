#ifndef HISTORY_H
#define HISTORY_H

#include <stdint.h>

#define MAX_LINE 80
#define MAX_SIZE 10

typedef struct history {
  char **commands;
  int32_t size;
  int32_t max_size;
  int32_t max_line;
} history_t;

history_t *init_history(history_t *);
void destory_history(history_t *);
char *add_to_history(history_t *, char *);
void print_full_history(history_t);
void print_recent_history(history_t);
void overwrite_history(history_t *, char *, int, char *);
char *get_history(history_t, int);

#endif
