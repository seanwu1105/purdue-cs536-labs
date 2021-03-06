#include "parse_command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARGUMENT_SIZE 4096

typedef enum
{
    LOOK_FOR_ARG,
    IN_ARG,
    IN_QUOTE,
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
     {IN_QUOTE, CREATE_ARG},           // DOUBLE_QUOTE
     {ESCAPED_IN_ARG, CREATE_ARG},     // ESCAPE
     {IN_ARG, CREATE_AND_ADD_TO_ARG}}, // OTHERS
    // IN_ARG
    {{LOOK_FOR_ARG, CLOSE_ARG}, // WHITESPACE
     {IN_QUOTE, IGNORE},        // DOUBLE_QUOTE
     {ESCAPED_IN_ARG, IGNORE},  // ESCAPE
     {IN_ARG, ADD_TO_ARG}},     // OTHERS
    // IN_QUOTE
    {{IN_QUOTE, ADD_TO_ARG},          // WHITESPACE
     {IN_ARG, IGNORE},                // DOUBLE_QUOTE
     {ESCAPED_IN_QUOTED_ARG, IGNORE}, // ESCAPE
     {IN_QUOTE, ADD_TO_ARG}},         // OTHERS
    // ESCAPED_IN_ARG
    {{IN_ARG, ADD_TO_ARG},  // WHITESPACE
     {IN_ARG, ADD_TO_ARG},  // DOUBLE_QUOTE
     {IN_ARG, ADD_TO_ARG},  // ESCAPE
     {IN_ARG, ADD_TO_ARG}}, // OTHERS
    // ESCAPED_IN_QUOTED_ARG
    {{IN_QUOTE, ADD_TO_ARG},  // WHITESPACE
     {IN_QUOTE, ADD_TO_ARG},  // DOUBLE_QUOTE
     {IN_QUOTE, ADD_TO_ARG},  // ESCAPE
     {IN_QUOTE, ADD_TO_ARG}}, // OTHERS
};

NextCharCategory parse_next_char(const char c)
{
    if (c == ' ' || c == '\t' || c == '\n') return WHITESPACE;
    if (c == '\"') return DOUBLE_QUOTE;
    if (c == '\\') return ESCAPE;
    return OTHERS;
}

void parse_command(const char *const command, char *arguments[])
{
    Effect effect;
    State state = LOOK_FOR_ARG;
    NextCharCategory next_char_category;

    const char *next_char = command;
    char *argument = "\0";
    size_t argument_idx = 0;
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

    if (argument[argument_idx] != '\0') argument[argument_idx] = '\0';

    if (effect.action != CLOSE_ARG && strlen(argument))
        arguments[arguments_idx++] = argument;

    arguments[arguments_idx] = NULL;
}

void clear_arguments(char *arguments[])
{
    size_t i = 0;
    while (arguments[i])
        free(arguments[i++]);
}