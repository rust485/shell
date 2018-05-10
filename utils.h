#ifndef UTILS_H
#define UTILS_H

#define ERROR_CODE -1

#define REG_EXEC_CODE -2

#define HISTORY "history"
#define HISTORY_CODE -3

#define RECENT_EXEC "!!"
#define RECENT_EXEC_CODE -4

#define MULTI_LINE_CODE -5

#define CD "cd"
#define CD_CODE -6

#define PIPE "|"

#define EXIT "exit"

#define MAX_LINE 80
#define MAX_HISTORY 10

#define BOLD_BLUE "\x1B[1;34m"
#define NO_ATTR "\x1B[0m"
#define SHARED_NAME "__history"

#define NORMAL_CODE 1
#define SEMICOLON_CODE 2
#define PIPE_CODE 3

int validate_input(char *);
void reset_args(char *[]);
char *trim(char *);
char *tokenize(char *str, char, int *);
char *get_input();
int parse_input(char *[], char *, uint8_t *);

#endif
