#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_RULES 100
#define MAX_STRING_LENGTH 100

typedef struct {
    char left;
    char right[MAX_STRING_LENGTH];
} Rule;

Rule grammar[MAX_RULES];
int num_rules = 0;

void read_grammar() {
    printf("Enter the grammar rules (e.g., S->aS|a). Type 'end' to finish:\n");
    while (1) {
        char line[MAX_STRING_LENGTH];
        printf("Rule: ");
        fgets(line, sizeof(line), stdin);
        line[strcspn(line, "\n")] = 0; // Remove newline

        if (strcmp(line, "end") == 0) break;

        Rule *rule = &grammar[num_rules++];
        rule->left = line[0];
        strcpy(rule->right, line + 3);
    }
}

int check_derivation(const char *input, int pos, const char *current) {
    if (pos == strlen(input) && current[0] == '\0') return 1;
    if (pos >= strlen(input)) return 0;

    for (int i = 0; i < num_rules; i++) {
        if (grammar[i].left == current[0]) {
            char new_current[MAX_STRING_LENGTH];
            strcpy(new_current, grammar[i].right);
            strcat(new_current, current + 1);

            if (check_derivation(input, pos, new_current)) return 1;
        }
    }

    if (input[pos] == current[0]) {
        if (check_derivation(input, pos + 1, current + 1)) return 1;
    }

    return 0;
}

int main() {
    char input[MAX_STRING_LENGTH];

    read_grammar();

    printf("Enter a string to check: ");
    scanf("%s", input);

    if (check_derivation(input, 0, "S")) {
        printf("The string can be derived from the grammar.\n");
    } else {
        printf("The string cannot be derived from the grammar.\n");
    }

    return 0;
}