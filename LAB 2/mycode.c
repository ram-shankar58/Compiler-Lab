#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define MAX_SYMBOLS 1000
#define MAX_RULES 1000
#define MAX_ALTERNATIVES 100
#define MAX_TOKENS 100
#define MAX_STRING_LENGTH 100
// Structure to represent a sentential form
typedef struct sentential_form {
    struct sentential_form *parent;
    size_t parent_symbol_id;
    size_t parent_alternative_id;
    size_t *symbols;
    size_t num_symbols;
    size_t depth;
} sentential_form;
// Structure for the grammar rules
typedef struct grammar {
    char **symbols;
    size_t num_symbols;
    size_t ***rules;
    size_t *num_rules;
    size_t **rule_sizes;
} grammar;
// Function prototypes
void print_sentential_form(grammar *g, sentential_form *s);
void print_derivation(grammar *g, sentential_form *s, size_t continuing_indentation);
size_t register_symbol(grammar *g, const char *symbol);
bool nonterminal_parsable(grammar *g, size_t *visited_symbols, size_t symbol);
bool equivalent_helper(grammar *g, sentential_form **frame_a, sentential_form **frame_b,
                       size_t frame_pos_a, size_t frame_pos_b, size_t symbol_pos_a, size_t symbol_pos_b);
bool equivalent(grammar *g, sentential_form *sentential_form_a, sentential_form *sentential_form_b);
void parse_grammar(grammar *g);
void check_empty_language(grammar *g);
void eliminate_non_parsable_nonterminals(grammar *g);
void check_grammar_ambiguity(grammar *g);
// Function to read the grammar input from the user
void read_grammar_from_user(grammar *g) {
    printf("Enter the grammar. Type 'end' to finish.\n");
    while (1) {
        char line[1024];
        printf("Enter a production rule: ");
        fgets(line, sizeof(line), stdin);
        line[strcspn(line, "\n")] = 0; // Remove trailing newline
        if (strcmp(line, "end") == 0) {
            break;
        }
        // Tokenize the input
        char *tokens[MAX_TOKENS];
        size_t num_tokens = 0;
        char *token = strtok(line, " \t\r\n");
        while (token) {
            tokens[num_tokens++] = token;
            token = strtok(NULL, " \t\r\n");
        }
        if (num_tokens < 2) continue;
        char *nonterminal = tokens[0];
        if (tokens[1][0] != '=') {
            printf("Bad production rule: %s\n", line);
            continue;
        }
        size_t left_symbol_id = register_symbol(g, nonterminal);
        size_t alt_start = 2;
        while (alt_start < num_tokens) {
            size_t alt_end = alt_start;
            while (alt_end < num_tokens && strcmp(tokens[alt_end], "|") != 0) {
                alt_end++;
            }
            size_t *alternative = malloc((alt_end - alt_start) * sizeof(size_t));
            for (size_t i = alt_start; i < alt_end; ++i) {
                alternative[i - alt_start] = register_symbol(g, tokens[i]);
            }
            g->rules[left_symbol_id][g->num_rules[left_symbol_id]] = alternative;
            g->rule_sizes[left_symbol_id][g->num_rules[left_symbol_id]] = alt_end - alt_start;
            g->num_rules[left_symbol_id]++;
            alt_start = alt_end + 1;
        }
    }
}
void print_sentential_form(grammar *g, sentential_form *s) {
    for (size_t i = 0; i < s->num_symbols; i++) {
        printf("%s ", g->symbols[s->symbols[i]]);
    }
    printf("\n");
}
void print_derivation(grammar *g, sentential_form *s, size_t continuing_indentation) {
    if (s->parent) {
        print_derivation(g, s->parent, continuing_indentation);
        for (size_t i = 0; i < continuing_indentation; i++) {
            printf(" ");
        }
    }
    printf("%zu: ", s->depth);
    print_sentential_form(g, s);
}
size_t register_symbol(grammar *g, const char *symbol) {
    for (size_t i = 0; i < g->num_symbols; i++) {
        if (strcmp(symbol, g->symbols[i]) == 0) {
            return i;
        }
    }
    g->symbols[g->num_symbols] = strdup(symbol);
    g->rules[g->num_symbols] = malloc(MAX_ALTERNATIVES * sizeof(size_t *));
    g->rule_sizes[g->num_symbols] = malloc(MAX_ALTERNATIVES * sizeof(size_t));
    g->num_rules[g->num_symbols] = 0;
    return g->num_symbols++;
}

bool nonterminal_parsable(grammar *g, size_t *visited_symbols, size_t symbol) {
    if (visited_symbols[symbol]) return false; // Avoid infinite loops in recursion
    visited_symbols[symbol] = 1;

    if (g->num_rules[symbol] == 0) {
        // A terminal symbol or no rules: consider parsable
        return true;
    }

    for (size_t alt_id = 0; alt_id < g->num_rules[symbol]; alt_id++) {
        bool alternative_parsable = true;
        for (size_t i = 0; i < g->rule_sizes[symbol][alt_id]; i++) {
            size_t alt_symbol = g->rules[symbol][alt_id][i];
            if (g->num_rules[alt_symbol] > 0) {
                // Recursively check nonterminal symbols
                if (!nonterminal_parsable(g, visited_symbols, alt_symbol)) {
                    alternative_parsable = false;
                    break;
                }
            }
        }
        if (alternative_parsable) {
            return true; // Found a parsable alternative
        }
    }
    return false; // No parsable alternatives found
}


bool equivalent_helper(grammar *g, sentential_form **frame_a, sentential_form **frame_b,
                       size_t frame_pos_a, size_t frame_pos_b, size_t symbol_pos_a, size_t symbol_pos_b) {
    sentential_form *a = frame_a[frame_pos_a];
    sentential_form *b = frame_b[frame_pos_b];
    size_t symbol_a = a->symbols[symbol_pos_a];
    size_t symbol_b = b->symbols[symbol_pos_b];
    if (symbol_a != symbol_b) return false;
    if (frame_pos_a == 0 || frame_pos_b == 0) {
        return frame_pos_a == frame_pos_b;
    }
    for (size_t i = 0; i < g->rule_sizes[symbol_a][a->parent_alternative_id]; ++i) {
        if (!equivalent_helper(g, frame_a, frame_b, frame_pos_a - 1, frame_pos_b - 1, symbol_pos_a + i, symbol_pos_b + i)) {
            return false;
        }
    }
    return true;
}
bool equivalent(grammar *g, sentential_form *sentential_form_a, sentential_form *sentential_form_b) {
    sentential_form *frame_a[MAX_SYMBOLS];
    sentential_form *frame_b[MAX_SYMBOLS];
    size_t frame_pos_a = 0;
    size_t frame_pos_b = 0;
    sentential_form *current = sentential_form_a;
    while (current->parent) {
        frame_a[frame_pos_a++] = current;
        current = current->parent;
    }
    frame_a[frame_pos_a++] = current;
    current = sentential_form_b;
    while (current->parent) {
        frame_b[frame_pos_b++] = current;
        current = current->parent;
    }
    frame_b[frame_pos_b++] = current;
    return equivalent_helper(g, frame_a, frame_b, frame_pos_a - 1, frame_pos_b - 1, 0, 0);
}
void check_empty_language(grammar *g) {
    size_t visited_symbols[MAX_SYMBOLS] = { 0 };
    if (!nonterminal_parsable(g, visited_symbols, 0)) {
        printf("The language generated by the grammar is empty.\n");
        exit(1);
    }
}
// void eliminate_non_parsable_nonterminals(grammar *g) {
//     size_t visited_symbols[MAX_SYMBOLS] = { 0 };
//     nonterminal_parsable(g, visited_symbols, 0);
//     printf("Non-parsable nonterminals eliminated.\n");
// }
// void check_grammar_ambiguity(grammar *g) {
//     printf("Checking for ambiguity...\n");
//     // Try to derive the same string in more than one way
//     for (size_t i = 0; i < g->num_rules[0]; i++) {
//         for (size_t j = i + 1; j < g->num_rules[0]; j++) {
//             size_t *alternative1 = g->rules[0][i];
//             size_t *alternative2 = g->rules[0][j];
            
//             // Check if these alternatives generate the same output
//             if (equivalent(g, alternative1, alternative2)) {
//                 printf("Ambiguity detected!\n");
//                 return;
//             }
//         }
//     }
//     printf("No ambiguity detected.\n");
//)

void eliminate_non_parsable_nonterminals(grammar *g) {
    size_t visited_symbols[MAX_SYMBOLS] = { 0 };
    nonterminal_parsable(g, visited_symbols, 0);
    printf("Non-parsable nonterminals eliminated.\n");
}

void check_grammar_ambiguity(grammar *g) {
    printf("Checking for ambiguity...\n");
    // Try to derive the same string in more than one way
    for (size_t i = 0; i < g->num_rules[0]; i++) {
        for (size_t j = i + 1; j < g->num_rules[0]; j++) {
            size_t *alternative1 = g->rules[0][i];
            size_t *alternative2 = g->rules[0][j];

            // Create sentential_form structures for alternatives
            sentential_form sf1 = {NULL, 0, 0, alternative1, g->rule_sizes[0][i], 0};
            sentential_form sf2 = {NULL, 0, 0, alternative2, g->rule_sizes[0][j], 0};
            
            // Check if these alternatives generate the same output
            if (equivalent(g, &sf1, &sf2)) {
                printf("Ambiguity detected!\n");
                return;
            }
        }
    }
    printf("No ambiguity detected.\n");
}

int main() {
    grammar g = {0};
    g.symbols = malloc(MAX_SYMBOLS * sizeof(char*));
    g.rules = malloc(MAX_SYMBOLS * sizeof(size_t**));
    g.num_rules = malloc(MAX_SYMBOLS * sizeof(size_t));
    g.rule_sizes = malloc(MAX_SYMBOLS * sizeof(size_t*));
    read_grammar_from_user(&g);
    check_empty_language(&g);
    eliminate_non_parsable_nonterminals(&g);
    check_grammar_ambiguity(&g);
    return 0;
}