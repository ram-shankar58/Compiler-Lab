#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_RULES 100
#define MAX_STRING_LENGTH 100

typedef struct {
    char left;
    char *productions[MAX_RULES];
    int num_productions;
} Rule;

Rule grammar[MAX_RULES];
int num_rules = 0;

// Function to split a production rule into individual productions (separated by '|')
void split_productions(char *right, char *productions[], int *num_productions) {
    char *token = strtok(right, "|");
    while (token != NULL) {
        productions[(*num_productions)++] = strdup(token);
        token = strtok(NULL, "|");
    }
}

void read_grammar() {
    printf("Enter the grammar rules (e.g., S->aS|ab). Type 'end' to finish:\n");
    while (1) {
        char line[MAX_STRING_LENGTH];
        printf("Rule: ");
        fgets(line, sizeof(line), stdin);
        line[strcspn(line, "\n")] = 0; // Remove newline

        if (strcmp(line, "end") == 0) break;

        Rule *rule = &grammar[num_rules++];
        rule->left = line[0];
        rule->num_productions = 0;
        split_productions(line + 3, rule->productions, &rule->num_productions);
    }
}

int check_derivation(const char *input, int pos, const char *current) {
    if (pos == strlen(input) && current[0] == '\0') return 1;
    if (pos > strlen(input)) return 0;

    // Process each character in `current`
    if (current[0] >= 'A' && current[0] <= 'Z') { // If it's a non-terminal
        for (int i = 0; i < num_rules; i++) {
            if (grammar[i].left == current[0]) { // Match the rule's left-hand side
                for (int j = 0; j < grammar[i].num_productions; j++) {
                    char new_current[MAX_STRING_LENGTH];
                    snprintf(new_current, sizeof(new_current), "%s%s", grammar[i].productions[j], current + 1);

                    if (check_derivation(input, pos, new_current)) return 1;
                }
            }
        }
    } else if (current[0] == input[pos]) { // If it's a terminal and matches the input
        return check_derivation(input, pos + 1, current + 1);
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
