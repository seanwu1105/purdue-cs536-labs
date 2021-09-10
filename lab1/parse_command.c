#include <stdlib.h>
#include <stdio.h>
#include "parse_command.h"

#define ARGUMENT_SIZE 100

typedef enum
{
    LOOK_FOR_ARG,
    IN_ARG,
    IN_QUOTED_ARG,
    ESCAPED_IN_ARG,
    ESCAPED_IN_QUOTED_ARG,
} State;

typedef enum
{
    WHITESPACE,
    DOUBLE_QUOTE,
    ESCAPE,
    OTHERS,
} NextCharCategory;

typedef enum
{
    CREATE_ARG,
    CREATE_AND_ADD_TO_ARG,
    ADD_TO_ARG,
    CLOSE_ARG,
    IGNORE,
} Action;

typedef struct
{
    State next_state;
    Action action;
} Effect;

const Effect TRANSITIONS[5][4] = {
    // LOOK_FOR_ARG
    {{LOOK_FOR_ARG, IGNORE},           // WHITESPACE
     {IN_QUOTED_ARG, CREATE_ARG},      // DOUBLE_QUOTE
     {ESCAPED_IN_ARG, CREATE_ARG},     // ESCAPE
     {IN_ARG, CREATE_AND_ADD_TO_ARG}}, // OTHERS
    // IN_ARG
    {{LOOK_FOR_ARG, CLOSE_ARG}, // WHITESPACE
     {IN_ARG, ADD_TO_ARG},      // DOUBLE_QUOTE
     {ESCAPED_IN_ARG, IGNORE},  // ESCAPE
     {IN_ARG, ADD_TO_ARG}},     // OTHERS
    // IN_QUOTED_ARG
    {{IN_QUOTED_ARG, ADD_TO_ARG},     // WHITESPACE
     {LOOK_FOR_ARG, CLOSE_ARG},       // DOUBLE_QUOTE
     {ESCAPED_IN_QUOTED_ARG, IGNORE}, // ESCAPE
     {IN_QUOTED_ARG, ADD_TO_ARG}},    // OTHERS
    // ESCAPED_IN_ARG
    {{IN_ARG, ADD_TO_ARG},  // WHITESPACE
     {IN_ARG, ADD_TO_ARG},  // DOUBLE_QUOTE
     {IN_ARG, ADD_TO_ARG},  // ESCAPE
     {IN_ARG, ADD_TO_ARG}}, // OTHERS
    // ESCAPED_IN_QUOTED_ARG
    {{IN_QUOTED_ARG, ADD_TO_ARG},  // WHITESPACE
     {IN_QUOTED_ARG, ADD_TO_ARG},  // DOUBLE_QUOTE
     {IN_QUOTED_ARG, ADD_TO_ARG},  // ESCAPE
     {IN_QUOTED_ARG, ADD_TO_ARG}}, // OTHERS
};

NextCharCategory parse_next_char(const char c)
{
    if (c == ' ' || c == '\t' || c == '\n')
        return WHITESPACE;
    if (c == '\"')
        return DOUBLE_QUOTE;
    if (c == '\\')
        return ESCAPE;
    return OTHERS;
}

int parse_command(const char *const command, char *arguments[])
{
    Effect effect;
    State state = LOOK_FOR_ARG;
    NextCharCategory next_char_category;

    const char *next_char = command;
    char *argument;
    size_t argument_idx;
    size_t arguments_idx = 0;

    while (*next_char)
    {
        next_char_category = parse_next_char(*next_char);
        effect = TRANSITIONS[state][next_char_category];

        switch (effect.action)
        {
        case CREATE_ARG:
            argument = (char *)malloc(sizeof(char) * ARGUMENT_SIZE);
            argument_idx = 0;
            break;

        case CREATE_AND_ADD_TO_ARG:
            argument = (char *)malloc(sizeof(char) * ARGUMENT_SIZE);
            argument_idx = 0;
            argument[argument_idx++] = *next_char;
            break;

        case ADD_TO_ARG:
            argument[argument_idx++] = *next_char;
            break;

        case CLOSE_ARG:
            argument[argument_idx] = '\0';
            arguments[arguments_idx++] = argument;
            break;

        case IGNORE:
        default:
            break;
        }

        state = effect.next_state;
        next_char++;
    }

    if (argument[argument_idx] != '\0')
        argument[argument_idx] = '\0';

    if (effect.action != CLOSE_ARG)
        arguments[arguments_idx++] = argument;

    arguments[arguments_idx] = NULL;

    return 0;
}