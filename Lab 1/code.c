#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Dynamic buffer size for reading tokens
#define INITIAL_BUFFER_SIZE 64

// Function to read items from a file into a dynamically allocated array
char **read_list_from_file(const char *filename, int *count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    char **list = NULL;
    char buffer[INITIAL_BUFFER_SIZE];
    *count = 0;

    while (fscanf(file, "%s", buffer) != EOF) {
        list = realloc(list, (*count + 1) * sizeof(char *));
        if (!list) {
            perror("Failed to allocate memory");
            exit(EXIT_FAILURE);
        }
        list[*count] = strdup(buffer);
        if (!list[*count]) {
            perror("Failed to duplicate string");
            exit(EXIT_FAILURE);
        }
        (*count)++;
    }

    fclose(file);
    return list;
}

// Function to check if a string is in a list
int is_in_list(const char *token, char **list, int count) {
    for (int i = 0; i < count; i++) {
        if (strcmp(token, list[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Function to determine if a character is a delimiter
int is_delimiter(char ch, char **delimiters, int count) {
    char temp[2] = {ch, '\0'};
    return is_in_list(temp, delimiters, count);
}

// Main token parsing function
void parse_tokens(const char *input_file, const char *output_file, char **keywords, int keyword_count,
                  char **delimiters, int delimiter_count, char **operators, int operator_count) {
    FILE *input = fopen(input_file, "r");
    FILE *output = fopen(output_file, "w");

    if (!input || !output) {
        perror("Failed to open input/output file");
        exit(EXIT_FAILURE);
    }

    char *token = malloc(INITIAL_BUFFER_SIZE);
    if (!token) {
        perror("Failed to allocate memory for token");
        exit(EXIT_FAILURE);
    }

    size_t token_size = INITIAL_BUFFER_SIZE;
    size_t token_index = 0;
    char ch;

    while ((ch = fgetc(input)) != EOF) {
        if (isspace(ch) || is_delimiter(ch, delimiters, delimiter_count)) {
            if (token_index > 0) {
                token[token_index] = '\0';

                if (is_in_list(token, keywords, keyword_count)) {
                    fprintf(output, "Keyword: %s\n", token);
                } else if (is_in_list(token, operators, operator_count)) {
                    fprintf(output, "Operator: %s\n", token);
                } else if (isalpha(token[0])) {
                    fprintf(output, "Identifier: %s\n", token);
                } else if (isdigit(token[0])) {
                    fprintf(output, "Number: %s\n", token);
                } else {
                    fprintf(output, "Unknown: %s\n", token);
                }

                token_index = 0;
            }

            if (is_delimiter(ch, delimiters, delimiter_count)) {
                fprintf(output, "Delimiter: %c\n", ch);
            }
        } else {
            if (token_index >= token_size - 1) {
                token_size *= 2;
                token = realloc(token, token_size);
                if (!token) {
                    perror("Failed to reallocate memory for token");
                    exit(EXIT_FAILURE);
                }
            }
            token[token_index++] = ch;
        }
    }

    // Handle the last token
    if (token_index > 0) {
        token[token_index] = '\0';
        if (is_in_list(token, keywords, keyword_count)) {
            fprintf(output, "Keyword: %s\n", token);
        } else if (is_in_list(token, operators, operator_count)) {
            fprintf(output, "Operator: %s\n", token);
        } else if (isalpha(token[0])) {
            fprintf(output, "Identifier: %s\n", token);
        } else if (isdigit(token[0])) {
            fprintf(output, "Number: %s\n", token);
        } else {
            fprintf(output, "Unknown: %s\n", token);
        }
    }

    free(token);
    fclose(input);
    fclose(output);
}

int main() {
    char **keywords, **delimiters, **operators;
    int keyword_count, delimiter_count, operator_count;

    keywords = read_list_from_file("keywords.txt", &keyword_count);
    delimiters = read_list_from_file("delimiters.txt", &delimiter_count);
    operators = read_list_from_file("operators.txt", &operator_count);

    parse_tokens("input_program.txt", "output_tokens.txt", keywords, keyword_count, delimiters, delimiter_count, operators, operator_count);

    for (int i = 0; i < keyword_count; i++) free(keywords[i]);
    for (int i = 0; i < delimiter_count; i++) free(delimiters[i]);
    for (int i = 0; i < operator_count; i++) free(operators[i]);

    free(keywords);
    free(delimiters);
    free(operators);

    printf("Tokens parsed and written to output_tokens.txt\n");

    return 0;
}
