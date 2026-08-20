// This is an implementation of dynamically loaded library of recursive stacks.
// Single element on an rstack is either an integer (uint_64) or rstack.
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <ctype.h>
#include "rstack.h"

typedef enum {
    TYPE_INT,
    TYPE_STACK
} element_type_t;

// Colors are used for marking rstacks for safe memory management.
// 1. An active rstack is marked with COLOR_BLACK.
// 2. A rstack being tested whether it is in a cycle is marked with COLOR_GRAY.
// 3. An unused and ready for freeing rstack is marked with COLOR_WHITE.
typedef enum {
    COLOR_BLACK,
    COLOR_GRAY,
    COLOR_WHITE
} color_t;

typedef struct rnode {
    element_type_t type;
    union {
        uint64_t value;
        struct rstack *stack;
    } element;
    bool visited;
    struct rnode *next;
} rnode_t;

typedef struct rstack {
    rnode_t *top;
    int referenceCounter;
    color_t color;
    struct rstack *next_free;   // temporary list for safe freeing
} rstack_t;

// --- Garbage Collector Functions ---

static void free_stack_memory(rstack_t *rs) {
    if (rs == nullptr) return;

    //To free a rstack memory we iterate through all its nodes and free them.
    rnode_t *curr = rs->top;
    while (curr != nullptr) {
        rnode_t *next = curr->next;
        free(curr);
        curr = next;
    }

    //Lastly we need to remember to free a rstack itself.
    free(rs);
}

// Function decreasing accessible rstacks referenceCounter for a trial.
static void mark_gray_and_decrease_reference(rstack_t *rs) {
    if (rs == nullptr) return;

    if (rs->color != COLOR_GRAY) {
        // We mark already visited rstacks as
        // gray so we do not consider them twice.
        rs->color = COLOR_GRAY;

        for (rnode_t *curr = rs->top; curr != nullptr; curr = curr->next) {
            if (curr->type == TYPE_STACK) {
                curr->element.stack->referenceCounter--;
                // We need to perform the trial also on this rstack.
                mark_gray_and_decrease_reference(curr->element.stack);
            }
        }
    }
}

static void restore_reference(rstack_t *rs) {
    if (rs == nullptr) return;

    // We need to mark an rstack as active.
    rs->color = COLOR_BLACK;

    for (rnode_t *curr = rs->top; curr != nullptr; curr = curr->next) {
        if (curr->type == TYPE_STACK) {
            curr->element.stack->referenceCounter++;

            // We need to perform the restoration if an rstack is unrestored.
            if (curr->element.stack->color != COLOR_BLACK) {
                restore_reference(curr->element.stack);
            }
        }
    }
}

// Function verifying whether rstack is ready for deletion after a trial.
static void restore_reference_or_mark_white(rstack_t *rs) {
    if (rs == nullptr) return;

    if (rs->color == COLOR_GRAY) {
        if (rs->referenceCounter > 0) {
            restore_reference(rs);
        }
        else {
            // Rstack is not used anywhere so it is marked for freeing.
            rs->color = COLOR_WHITE;

            // Rstacks inside are also checked whether they can turn white.
            for (rnode_t *curr = rs->top; curr != nullptr; curr = curr->next) {
                if (curr->type == TYPE_STACK) {
                    restore_reference_or_mark_white(curr->element.stack);
                }
            }
        }
    }
}

// Safe collection to a local list to avoid Use-After-Free.
static void collect_white(rstack_t *rs, rstack_t **free_list) {
    if (rs == nullptr) return;

    if (rs->color == COLOR_WHITE) {
        // It is an precaution for cycles.
        rs->color = COLOR_BLACK;

        // Instead of freeing instantly,
        // we add the rstack to a list for later freeing.
        rs->next_free = *free_list;
        *free_list = rs;

        // We need to check if rstacks inside the one checked
        // are ready for deletion.
        for (rnode_t *curr = rs->top; curr != nullptr; curr = curr->next) {
            if (curr->type == TYPE_STACK) {
                collect_white(curr->element.stack, free_list);
            }
        }
    }
}

// Main function of Garbage Collector called by rstack_delete(rstack_t *rs).
static void release(rstack_t *rs) {
    if (rs == nullptr) return;

    rs->referenceCounter--;

    if (rs->referenceCounter == 0) { // We perform deletion of a rstack.
        // Firstly we need to perform same operation on internal rstacks.
        for (rnode_t *curr = rs->top; curr != nullptr; curr = curr->next) {
            if (curr->type == TYPE_STACK) {
                release(curr->element.stack);
            }
        }

        // Afterwards we are good to free initial rstack memory.
        free_stack_memory(rs);
    }
    else { // We need to perform trial deletion.
        mark_gray_and_decrease_reference(rs);

        restore_reference_or_mark_white(rs);

        rstack_t *free_list = nullptr;

        collect_white(rs, &free_list);

        // After a preparation of a list of white rstacks we perform freeing.
        while (free_list != nullptr) {
            rstack_t *to_free = free_list;
            free_list = free_list->next_free;
            free_stack_memory(to_free);
        }
    }
}

// --- Helper Functions ---

// Function finding a value of a first topmost integer unless rstack is empty.
// It performs a DFS traversal on elements of a rstack.
// When called by rstack_empty(rs) it has value = nullptr.
static void empty_or_front(rnode_t *curr, uint64_t *value, bool *found) {
    if (curr == nullptr || curr->visited || *found) return;

    if (curr->type == TYPE_INT) {
        if (value != nullptr) {
            *value = curr->element.value;
        }
        *found = true;
        return;
    }

    curr->visited = true;

    empty_or_front(curr->element.stack->top, value, found);
    empty_or_front(curr->next, value, found);
}


// Function to reset visited flags after DFS traversal.
static void reset_visits(rnode_t *curr) {
    if (curr == nullptr || !curr->visited) return;

    curr->visited = false;

    if (curr->type == TYPE_STACK)
        reset_visits(curr->element.stack->top);

    reset_visits(curr->next);
}

// Function to print rstack's elements
// It prints them from the bottom until top or cycle is reached.
static void print_stack(rnode_t *curr, FILE *out, bool *cycle) {
    if (curr == nullptr || *cycle) return;

    if (curr->visited) {
        *cycle = true;
        return;
    }

    // We need to mark node as visited for the later recursive call.
    curr->visited = true;

    // We firstly go to the bootem
    print_stack(curr->next, out, cycle);

    if (!(*cycle)) { // We print the contents of a rstack.
        if (curr->type == TYPE_INT) {
            fprintf(out, "%" PRIu64 "\n", curr->element.value);
        }
        else {
            print_stack(curr->element.stack->top, out, cycle);
        }
    }

    // After a recursive call we can unmark the node for later print.
    curr->visited = false;
}

// --- Library Functions ---

// Creates a new, empty recursive stack.
// At creation, the stack's reference counter is set to one.
// Returns a pointer to the newly allocated stack structure,
// or nullptr if memory allocation fails, setting errno to ENOMEM.
rstack_t* rstack_new() {
    rstack_t *rs = malloc(sizeof(rstack_t));
    if (rs == nullptr) {
        errno = ENOMEM;
        return nullptr;
    }

    rs->top = nullptr;
    rs->referenceCounter = 1;
    rs->color = COLOR_BLACK;
    rs->next_free = nullptr;

    return rs;
}

// Deletes the given stack by decreasing its reference counter by one.
// If the counter reaches zero, the stack is permanently removed from
// memory along with its internal nodes. If the provided pointer is
// nullptr, the function does nothing. The pointer must not be used
// after calling this function.
void rstack_delete(rstack_t *rs) {
    release(rs);
}

// Pushes an integer value onto the stack.
// Returns 0 on success. Returns -1 if the stack pointer is nullptr
// (sets errno to EINVAL) or if memory allocation fails (sets errno to
// ENOMEM).
int rstack_push_value(rstack_t *rs, uint64_t value) {
    if (rs == nullptr) {
        errno = EINVAL;
        return -1;
    }

    rnode_t *new_node = malloc(sizeof(rnode_t));
    if (new_node == nullptr) {
        errno = ENOMEM;
        return -1;
    }

    new_node->type = TYPE_INT;
    new_node->element.value = value;
    new_node->next = rs->top;
    new_node->visited = false;
    rs->top = new_node;

    return 0;
}

// Pushes a reference of one stack (rs2) onto another stack (rs1).
// This does not copy the pushed stack, but only pushes its reference,
// meaning changes to it are visible everywhere it is referenced.
// Increases the reference counter of the pushed stack by one.
// Returns 0 on success. Returns -1 if any of the pointers is nullptr
// (sets errno to EINVAL) or if memory allocation fails (sets errno to
// ENOMEM).
int rstack_push_rstack(rstack_t *rs1, rstack_t *rs2) {
    if (rs1 == nullptr || rs2 == nullptr) {
        errno = EINVAL;
        return -1;
    }

    rnode_t *new_node = malloc(sizeof(rnode_t));
    if (new_node == nullptr) {
        errno = ENOMEM;
        return -1;
    }

    rs2->referenceCounter++;

    new_node->type = TYPE_STACK;
    new_node->element.stack = rs2;
    new_node->next = rs1->top;
    new_node->visited = false;

    rs1->top = new_node;

    return 0;
}

// Non-recursively removes the top element from the stack.
// If the removed element is another stack, its reference counter is
// decreased by one. If the stack pointer is nullptr or the stack is
// already empty, the function does nothing.
void rstack_pop(rstack_t *rs) {
    if (rs == nullptr || rs->top == nullptr) return;

    rnode_t *curr = rs->top;
    rs->top = rs->top->next;

    if (curr->type == TYPE_STACK) {
        release(curr->element.stack);
    }

    free(curr);
}

// Recursively checks whether the stack contains any integer number.
// Returns true if the stack pointer is nullptr or if the stack
// (including nested stacks) does not contain any integer.
// Returns false if an integer is found.
bool rstack_empty(rstack_t *rs) {
    if (rs == nullptr) return true;

    bool found = false;
    empty_or_front(rs->top, nullptr, &found);
    reset_visits(rs->top);

    return !found;
}

// Recursively finds the integer that is closest to the top of the stack.
// Returns a result_t structure. If flag == true, the value field
// contains the found integer. If flag == false, it means the stack
// pointer is nullptr, the stack is empty, or no integer exists within
// the stack and its nested stacks.
result_t rstack_front(rstack_t *rs) {
    result_t result;
    result.flag = false;

    if (rs == nullptr) {
        return result;
    }

    empty_or_front(rs->top, &result.value, &result.flag);
    reset_visits(rs->top);

    return result;
}

// Creates a new stack and pushes numbers read from a specified file.
// Numbers in the file must be in base 10, separated by any amount of
// whitespace. The function strictly validates the file contents.
// Returns a pointer to the newly created stack. Returns nullptr if the
// path is nullptr, if file operations fail, if memory allocation fails,
// or if invalid characters are encountered. Sets the appropriate errno
// in case of failure.
rstack_t* rstack_read(char const *path) {
    if (path == nullptr) {
        errno = EINVAL;
        return nullptr;
    }

    rstack_t *rs = rstack_new();
    if (rs == nullptr) {
        return nullptr;
    }

    FILE *input = fopen(path, "r");
    if (input == nullptr) {
        rstack_delete(rs);
        return nullptr;
    }

    char *line = nullptr;
    size_t len = 0;
    bool success = true; // Flag controlling if read is successful

    while (success && getline(&line, &len, input) != -1) {
        char *p = line;
        char *end = nullptr;

        while (isspace((unsigned char)*p)) p++;

        while (success && *p != '\0') {
            if (*p == '-') { // There cannot be negative numbers
                errno = EINVAL;
                success = false;
            }
            else {
                errno = 0;
                uint64_t value = strtoull(p, &end, 10);
                // p == end means we found a character that is not a digit
                if (p == end) {
                    errno = EINVAL;
                    success = false;
                }
                else if (errno == ERANGE) {
                    success = false;
                }
                else if (rstack_push_value(rs, value) != 0) {
                    success = false;
                }
                else {
                    p = end;
                    while (isspace((unsigned char)*p)) p++;
                }
            }
        }
    }

    free(line);
    fclose(input);

    if (!success) {
        int saved_errno = errno;
        rstack_delete(rs);
        errno = saved_errno;
        return nullptr;
    }

    return rs;
}

// Writes all integers from the stack to a specified file.
// Numbers are written from bottom to top, each on a separate line in
// base 10 format without leading zeros. If a cycle is detected during
// traversal, writing is immediately aborted to prevent infinite loops.
// Returns 0 on success. Returns -1 if the path or stack pointer is
// nullptr, or if a file writing error occurs. Sets the appropriate
// errno in case of failure.
int rstack_write(char const *path, rstack_t *rs) {
    if (rs == nullptr) {
        errno = EINVAL;
        return -1;
    }

    if (path == nullptr) {
        errno = EINVAL;
        return -1;
    }

    FILE *output = fopen(path, "w");
    if (output == nullptr) {
        return -1;
    }

    bool cycle = false;
    print_stack(rs->top, output, &cycle);

    fclose(output);

    return 0;
}
