#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_TOKENS 64
#define TOKEN_DELIM " \t\r\n\a"

// Hàm loại bỏ khoảng trắng đầu và cuối chuỗi
char *trim_whitespace(char *str) {
    while (isspace(*str)) str++; // Bỏ khoảng trắng đầu

    if (*str == 0) return str; // chuỗi rỗng

    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;

    *(end + 1) = '\0'; // kết thúc chuỗi

    return str;
}

// Hàm tách dòng lệnh thành danh sách đối số
char **split_line(char *line) {
    int bufsize = MAX_TOKENS;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;
    int position = 0;

    token = strtok(line, TOKEN_DELIM);
    while (token != NULL) {
        tokens[position++] = token;

        if (position >= bufsize) {
            bufsize += MAX_TOKENS;
            tokens = realloc(tokens, bufsize * sizeof(char *));
        }

        token = strtok(NULL, TOKEN_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

// Xử lý lệnh built-in "cd"
int handle_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "cd: missing argument\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("cd");
        }
    }
    return 1;
}

// Thực thi một lệnh đơn
void execute_command(char *line) {
    line = trim_whitespace(line);
    if (strlen(line) == 0) return; // dòng trống

    char **args = split_line(line);

    if (args[0] == NULL) {
        free(args);
        return;
    }

    // Lệnh exit
    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting shell...\n");
        free(args);
        exit(0);
    }

    // Lệnh cd
    if (strcmp(args[0], "cd") == 0) {
        handle_cd(args);
        free(args);
        return;
    }

    // Các lệnh hệ thống khác
    pid_t pid = fork();

    if (pid == 0) {
        // Tiến trình con
        if (execvp(args[0], args) == -1) {
            perror("shell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("shell");
    } else {
        wait(NULL);
    }

    free(args);
}

// Tách và thực thi từng lệnh trong dòng (dựa trên `;` và `&&`)
void parse_and_execute_multiple(char *input) {
    char *token;
    token = strtok(input, ";");

    while (token != NULL) {
        char *subtoken = strtok(token, "&&");
        while (subtoken != NULL) {
            execute_command(subtoken);
            subtoken = strtok(NULL, "&&");
        }
        token = strtok(NULL, ";");
    }
}

int main() {
    char input[MAX_COMMAND_LENGTH];

    while (1) {
        printf("Shell> ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }
        parse_and_execute_multiple(input);
    }

    return 0;
}
