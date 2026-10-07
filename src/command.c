#include "command.h"

#include <ctype.h>
#include <string.h>

#define INPUT_SIZE 128

static int handle_new(SudokuGame *game, const char *arguments, FILE *output);
static int handle_set(SudokuGame *game, const char *arguments, FILE *output);
static int handle_clear(SudokuGame *game, const char *arguments, FILE *output);
static int handle_undo(SudokuGame *game, const char *arguments, FILE *output);
static int handle_print(SudokuGame *game, const char *arguments, FILE *output);
static int handle_solution(SudokuGame *game, const char *arguments, FILE *output);
static int handle_help(SudokuGame *game, const char *arguments, FILE *output);
static int handle_quit(SudokuGame *game, const char *arguments, FILE *output);

static const CommandEntry COMMANDS[] = {
    {"new", handle_new},
    {"set", handle_set},
    {"place", handle_set},
    {"clear", handle_clear},
    {"undo", handle_undo},
    {"print", handle_print},
    {"board", handle_print},
    {"solution", handle_solution},
    {"reveal", handle_solution},
    {"help", handle_help},
    {"quit", handle_quit},
    {"exit", handle_quit}
};

const CommandEntry *command_table(size_t *count) {

    if (count != NULL) {
        *count = sizeof(COMMANDS) / sizeof(COMMANDS[0]);
    }
    return COMMANDS;
}

void command_print_help(FILE *output) {
    if (output == NULL) {
        return;
    }

    fprintf(output, "Commands:\n");
    fprintf(output, "  new game          Start a new medium game\n");
    fprintf(output, "  new easy          Start an easy game\n");
    fprintf(output, "  new medium        Start a medium game\n");
    fprintf(output, "  new hard          Start a hard game\n");
    fprintf(output, "  set R C V         Put value V at row R, column C\n");
    fprintf(output, "  clear R C         Clear a non-fixed cell\n");
    fprintf(output, "  undo              Undo the most recent change\n");
    fprintf(output, "  print             Print the current board\n");
    fprintf(output, "  solution          Reveal the solved board\n");
    fprintf(output, "  help              Show this command list\n");
    fprintf(output, "  quit              Exit the program\n");
    fprintf(output, "\nRows, columns, and values are numbered 1 through 9.\n");
}

static void lowercase_string(char *text) {
    while (text != NULL && *text != '\0') {
        *text = (char)tolower((unsigned char)*text);
        text++;
    }
}

static char *skip_spaces(char *text) {
    while (text != NULL && *text != '\0' &&
           isspace((unsigned char)*text)) {
        text++;
    }
    return text;
}

static int parse_difficulty(const char *word, Difficulty *difficulty) {
    if (word == NULL || difficulty == NULL) {
        return 0;
    }

    if (strcmp(word, "easy") == 0) {
        *difficulty = DIFFICULTY_EASY;
        return 1;
    }
    if (strcmp(word, "medium") == 0 || strcmp(word, "game") == 0 ||
        *word == '\0') {
        *difficulty = DIFFICULTY_MEDIUM;
        return 1;
    }
    if (strcmp(word, "hard") == 0) {
        *difficulty = DIFFICULTY_HARD;
        return 1;
    }

    return 0;
}

static int handle_new(SudokuGame *game, const char *arguments, FILE *output) {
    char difficulty_word[INPUT_SIZE] = "";
    Difficulty difficulty = DIFFICULTY_MEDIUM;
    int holes;

    if (sscanf(arguments, "%127s", difficulty_word) == 1 &&
        !parse_difficulty(difficulty_word, &difficulty)) {
        fprintf(output, "Unknown difficulty. Choose easy, medium, or hard.\n");
        return 1;
    }

    holes = game_start_new(game, difficulty);
    if (holes <= 0) {
        fprintf(output, "Could not generate a new puzzle. Please try again.\n");
        return 1;
    }

    fprintf(output, "Started a new %s game with %d empty cells.\n",
            sudoku_difficulty_name(difficulty), holes);
    game_print_to(game, output);
    return 1;
}

static int handle_set(SudokuGame *game, const char *arguments, FILE *output) {
    int row;
    int column;
    int value;
    MoveResult result;

    if (sscanf(arguments, "%d %d %d", &row, &column, &value) != 3) {
        fprintf(output, "Usage: set ROW COLUMN VALUE\n");
        return 1;
    }

    result = game_place_value(game, row - 1, column - 1, value);
    fprintf(output, "%s\n", game_move_result_message(result));

    if (result == MOVE_OK) {
        game_print_to(game, output);
        if (game_has_won(game)) {
            fprintf(output, "Game win!\n");
        }
    }

    return 1;
}

static int handle_clear(SudokuGame *game,
                        const char *arguments,
                        FILE *output) {
    int row;
    int column;
    MoveResult result;

    if (sscanf(arguments, "%d %d", &row, &column) != 2) {
        fprintf(output, "Usage: clear ROW COLUMN\n");
        return 1;
    }

    result = game_clear_value(game, row - 1, column - 1);
    fprintf(output, "%s\n", game_move_result_message(result));

    if (result == MOVE_OK) {
        game_print_to(game, output);
    }

    return 1;
}

static int handle_undo(SudokuGame *game,
                       const char *arguments,
                       FILE *output) {
    MoveResult result;

    (void)arguments;
    result = game_undo(game);
    fprintf(output, "%s\n", game_move_result_message(result));

    if (result == MOVE_OK) {
        game_print_to(game, output);
    }

    return 1;
}

static int handle_print(SudokuGame *game,
                        const char *arguments,
                        FILE *output) {
    (void)arguments;
    game_print_to(game, output);
    return 1;
}

static int handle_solution(SudokuGame *game,
                           const char *arguments,
                           FILE *output) {
    (void)arguments;
    game_print_solution_to(game, output);
    return 1;
}

static int handle_help(SudokuGame *game,
                       const char *arguments,
                       FILE *output) {
    (void)game;
    (void)arguments;
    command_print_help(output);
    return 1;
}

static int handle_quit(SudokuGame *game,
                       const char *arguments,
                       FILE *output) {
    (void)game;
    (void)arguments;
    (void)output;
    return 0;
}

int command_dispatch(SudokuGame *game, char *input, FILE *output) {
    char *command_name;
    char *arguments;
    size_t command_count;
    const CommandEntry *commands;

    if (input == NULL || output == NULL) {
        return 0;
    }

    lowercase_string(input);
    command_name = skip_spaces(input);
    arguments = command_name;

    while (*arguments != '\0' && !isspace((unsigned char)*arguments)) {
        arguments++;
    }

    if (*arguments != '\0') {
        *arguments = '\0';
        arguments = skip_spaces(arguments + 1);
    }

    if (*command_name == '\0') {
        return 1;
    }

    commands = command_table(&command_count);
    for (size_t index = 0; index < command_count; index++) {
        if (strcmp(commands[index].name, command_name) == 0) {
            return commands[index].handler(game, arguments, output);
        }
    }

    fprintf(output,
            "Unknown command: '%s'. Enter 'help' for a list of commands.\n",
            command_name);
    return 1;
}