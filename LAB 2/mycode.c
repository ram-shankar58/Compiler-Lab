#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_STRING_LENGTH 100

// Structure for each production
typedef struct Production {
    char production[MAX_STRING_LENGTH];
    struct Production *next;
} Production;

// Structure for each grammar rule
typedef struct Rule {
    char left;
    Production *productions;
    struct Rule *next;
} Rule;

Rule *grammar = NULL; // Head of the linked list of rules

// Function to add a production to a rule
void add_production(Rule *rule, const char *production) {
    Production *new_production = (Production *)malloc(sizeof(Production));
    strcpy(new_production->production, production);
    new_production->next = rule->productions;
    rule->productions = new_production;
}

// Function to add a rule to the grammar
void add_rule(char left, const char *productions) {
    Rule *new_rule = (Rule *)malloc(sizeof(Rule));
    new_rule->left = left;
    new_rule->productions = NULL;
    new_rule->next = grammar;
    grammar = new_rule;

    // Split the productions and add them to the rule
    char *token = strtok(strdup(productions), "|");
    while (token != NULL) {
        add_production(new_rule, token);
        token = strtok(NULL, "|");
    }
}

// Function to read grammar from the user
void read_grammar() {
    printf("Enter the grammar rules (e.g., S->aS|ab). Type 'end' to finish:\n");
    while (1) {
        char line[MAX_STRING_LENGTH];
        printf("Rule: ");
        fgets(line, sizeof(line), stdin);
        line[strcspn(line, "\n")] = 0; // Remove newline

        if (strcmp(line, "end") == 0) break;

        add_rule(line[0], line + 3); // Add the rule to the grammar
    }
}

// Recursive function to check derivation
int check_derivation(const char *input, int pos, const char *current, char rule_sequence[MAX_STRING_LENGTH][MAX_STRING_LENGTH], int rule_count, char valid_sequences[MAX_STRING_LENGTH][MAX_STRING_LENGTH], int *valid_count) {
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

    if (current[0] >= 'A' && current[0] <= 'Z') { // Non-terminal
        for (Rule *rule = grammar; rule != NULL; rule = rule->next) {
            if (rule->left == current[0]) {
                for (Production *prod = rule->productions; prod != NULL; prod = prod->next) {
                    char new_current[MAX_STRING_LENGTH];
                    snprintf(new_current, sizeof(new_current), "%s%s", prod->production, current + 1);

                    // Record the rule being applied
                    char rule_string[MAX_STRING_LENGTH];
                    snprintf(rule_string, sizeof(rule_string), "%c->%s", rule->left, prod->production);
                    strcpy(rule_sequence[rule_count], rule_string);

                    if (check_derivation(input, pos, new_current, rule_sequence, rule_count + 1, valid_sequences, valid_count)) {
                        found = 1;
                    }
                }
            }
        }
    } else if (current[0] == input[pos]) { // Terminal matches input
        return check_derivation(input, pos + 1, current + 1, rule_sequence, rule_count, valid_sequences, valid_count);
    }

    return found;
}

// Function to free the memory used by the grammar
void free_grammar() {
    Rule *rule = grammar;
    while (rule != NULL) {
        Production *prod = rule->productions;
        while (prod != NULL) {
            Production *temp_prod = prod;
            prod = prod->next;
            free(temp_prod);
        }
        Rule *temp_rule = rule;
        rule = rule->next;
        free(temp_rule);
    }
}

int main() {
    char input[MAX_STRING_LENGTH];
    char valid_sequences[MAX_STRING_LENGTH][MAX_STRING_LENGTH];
    int valid_count = 0;

    read_grammar();

    printf("Enter a string to check: ");
    scanf("%s", input);

    char rule_sequence[MAX_STRING_LENGTH][MAX_STRING_LENGTH];
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

    free_grammar(); // Free the allocated memory
    return 0;
}
