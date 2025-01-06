#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT 100

// Node structure for each symbol in a production
typedef struct Node {
    char symbol;
    struct Node *next;
} Node;

// Grammar structure for each non-terminal
typedef struct Grammar {
    char non_terminal;
    Node *productions;  // Linked list of production rules
    struct Grammar *next;
} Grammar;

// Create a new node for a production rule
Node *createNode(char symbol) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (!newNode) {
        printf("Memory allocation failed for Node\n");
        exit(EXIT_FAILURE);
    }
    newNode->symbol = symbol;
    newNode->next = NULL;
    return newNode;
}

// Add a production to the grammar
void addProduction(Grammar **grammar, char non_terminal, const char *production) {
    // Find or create the grammar rule for the non-terminal
    Grammar *current = *grammar;
    while (current && current->non_terminal != non_terminal) {
        current = current->next;
    }

    if (!current) {
        // Non-terminal doesn't exist; create a new one
        current = (Grammar *)malloc(sizeof(Grammar));
        if (!current) {
            printf("Memory allocation failed for Grammar\n");
            exit(EXIT_FAILURE);
        }
        current->non_terminal = non_terminal;
        current->productions = NULL;
        current->next = *grammar;
        *grammar = current;
    }

    // Convert the production string into a linked list of symbols
    Node *head = NULL, *tail = NULL;
    for (int i = 0; production[i] != '\0'; i++) {
        Node *newNode = createNode(production[i]);
        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
    }

    // Add the production to the grammar
    if (tail) {
        tail->next = current->productions;
    }
    current->productions = head;
}

// Display the grammar for debugging
void displayGrammar(Grammar *grammar) {
    printf("\nGrammar Rules:\n");
    while (grammar) {
        printf("%c -> ", grammar->non_terminal);
        Node *prod = grammar->productions;
        while (prod) {
            Node *temp = prod;
            while (temp) {
                printf("%c", temp->symbol);
                temp = temp->next;
            }
            printf(" | ");
            prod = prod->next;
        }
        printf("\n");
        grammar = grammar->next;
    }
}

// Recursive function to parse the input string
int parseString(Grammar *grammar, char non_terminal, const char *input, int pos, int length) {
    if (pos >= length) {
        return 0; // Prevent out-of-bounds access
    }

    // Find the grammar rule for the given non-terminal
    Grammar *current = grammar;
    while (current && current->non_terminal != non_terminal) {
        current = current->next;
    }

    if (!current) {
        // Non-terminal not found in grammar
        return 0;
    }

    int count = 0;
    Node *prod = current->productions;

    // Iterate through all productions for the current non-terminal
    while (prod) {
        Node *temp = prod;
        int temp_pos = pos; // Temporary position for this production
        int valid = 1;

        // Process the production
        while (temp && temp_pos < length) {
            if (temp->symbol >= 'A' && temp->symbol <= 'Z') {
                // Non-terminal: Recursively parse
                int sub_count = parseString(grammar, temp->symbol, input, temp_pos, length);
                if (sub_count > 0) {
                    count += sub_count;
                } else {
                    valid = 0;
                }
                break;
            } else if (temp->symbol == input[temp_pos]) {
                // Terminal: Match the input
                temp_pos++;
            } else {
                // Terminal does not match
                valid = 0;
                break;
            }
            temp = temp->next;
        }

        // If the entire production was consumed and matches the input
        if (valid && !temp && temp_pos == length) {
            count++;
        }

        prod = prod->next; // Move to the next production
    }

    return count;
}

// Check if the grammar is ambiguous
void checkAmbiguity(Grammar *grammar, char start_symbol, const char *input) {
    int count = parseString(grammar, start_symbol, input, 0, strlen(input));
    printf("\nThe string can be parsed in %d way(s).\n", count);
    if (count > 1) {
        printf("The grammar is ambiguous.\n");
    } else {
        printf("The grammar is not ambiguous.\n");
    }
}

// Free the allocated memory for the grammar
void freeGrammar(Grammar *grammar) {
    while (grammar) {
        Grammar *tempGrammar = grammar;
        grammar = grammar->next;

        Node *prod = tempGrammar->productions;
        while (prod) {
            Node *tempNode = prod;
            prod = prod->next;
            free(tempNode);
        }

        free(tempGrammar);
    }
}

int main() {
    Grammar *grammar = NULL;
    char input[MAX_INPUT];
    char start_symbol;
    int n;

    printf("Enter the number of grammar rules: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Invalid number of grammar rules.\n");
        return EXIT_FAILURE;
    }

    printf("Enter the grammar rules (e.g., S->aA|b):\n");
    for (int i = 0; i < n; i++) {
        char rule[MAX_INPUT];
        if (scanf("%s", rule) != 1) {
            printf("Invalid input for grammar rule.\n");
            freeGrammar(grammar);
            return EXIT_FAILURE;
        }

        char non_terminal = rule[0];
        char *productions = strchr(rule, '>') + 1;

        if (!productions) {
            printf("Invalid grammar rule format.\n");
            freeGrammar(grammar);
            return EXIT_FAILURE;
        }

        char *production = strtok(productions, "|");
        while (production) {
            addProduction(&grammar, non_terminal, production);
            production = strtok(NULL, "|");
        }
    }

    printf("Enter the start symbol: ");
    if (scanf(" %c", &start_symbol) != 1 || start_symbol < 'A' || start_symbol > 'Z') {
        printf("Invalid start symbol.\n");
        freeGrammar(grammar);
        return EXIT_FAILURE;
    }

    displayGrammar(grammar);

    printf("Enter the input string: ");
    if (scanf("%s", input) != 1) {
        printf("Invalid input string.\n");
        freeGrammar(grammar);
        return EXIT_FAILURE;
    }

    checkAmbiguity(grammar, start_symbol, input);

    freeGrammar(grammar);
    return 0;
}
