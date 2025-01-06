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

int check_derivation(const char *input, int pos, const char *current, char rule_sequence[MAX_RULES][MAX_STRING_LENGTH], int rule_count, char valid_sequences[MAX_RULES][MAX_STRING_LENGTH], int *valid_count) {
    if (pos == strlen(input) && current[0] == '\0') {
        // Valid derivation, store the rule sequence
        char sequence[MAX_STRING_LENGTH] = "";
        for (int i = 0; i < rule_count; i++) {
            strcat(sequence, rule_sequence[i]);
            if (i < rule_count - 1) strcat(sequence, " -> ");
        }
        strcpy(valid_sequences[(*valid_count)++], sequence);
        return 1;
    }
    if (pos > strlen(input)) return 0;

    int found = 0;

    // Process each character in `current`
    if (current[0] >= 'A' && current[0] <= 'Z') { // If it's a non-terminal
        for (int i = 0; i < num_rules; i++) {
            if (grammar[i].left == current[0]) { // Match the rule's left-hand side
                for (int j = 0; j < grammar[i].num_productions; j++) {
                    char new_current[MAX_STRING_LENGTH];
                    snprintf(new_current, sizeof(new_current), "%s%s", grammar[i].productions[j], current + 1);

                    // Record the rule being applied
                    char rule_string[MAX_STRING_LENGTH];
                    snprintf(rule_string, sizeof(rule_string), "%c->%s", grammar[i].left, grammar[i].productions[j]);
                    strcpy(rule_sequence[rule_count], rule_string);

                    if (check_derivation(input, pos, new_current, rule_sequence, rule_count + 1, valid_sequences, valid_count)) {
                        found = 1;
                    }
                }
            }
        }
    } else if (current[0] == input[pos]) { // If it's a terminal and matches the input
        return check_derivation(input, pos + 1, current + 1, rule_sequence, rule_count, valid_sequences, valid_count);
    }

    return found;
}

int main() {
    char input[MAX_STRING_LENGTH];
    char valid_sequences[MAX_RULES][MAX_STRING_LENGTH];
    int valid_count = 0;

    read_grammar();

    printf("Enter a string to check: ");
    scanf("%s", input);

    char rule_sequence[MAX_RULES][MAX_STRING_LENGTH];
    int is_derivable = check_derivation(input, 0, "S", rule_sequence, 0, valid_sequences, &valid_count);

    if (is_derivable) {
        printf("The string can be derived from the grammar.\n");
        printf("Derivation sequences:\n");
        for (int i = 0; i < valid_count; i++) {
            printf("%d: %s\n", i + 1, valid_sequences[i]);
        }

        if (valid_count > 1) {
            printf("The grammar is ambiguous.\n");
        } else {
            printf("The grammar is not ambiguous.\n");
        }
    } else {
        printf("The string cannot be derived from the grammar.\n");
    }

    return 0;
}
