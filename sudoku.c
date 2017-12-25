#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define false 0
#define true 1
#define null 0
#define INPUT_CHAR '>'
#define BOARD_SIZE 9
#define TIMEOUT 5
#define NUMBER_SPACING 1
#define EASY_REMOVE 20
#define MED_REMOVE 26
#define HARD_REMOVE 32
#define EMPTY_CHAR ' '
#define ROW_SEP_CHAR '='
#define LEFT_RIGHT_BORDER '|'
#define BOX_BORDER "||"
#define HINT_COST 10
#define CHECK_COST 8
#define TIME_COST_FACTOR 10
#define DIFFICULTY_COST_FACTOR 2
#define ASCII_NUM_DIFF 48
#define ASCII_LETTER_DIFF 65
#define SPACE_VAL 32
#define BASE_SCORE 1000

typedef int bool;

char COL_NAMES[BOARD_SIZE] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I'};
char ROW_NAMES[BOARD_SIZE] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

char *MAIN_MENU_TITLE = "Please select an option from the list below";
const char *MAIN_MENU_OPTIONS[] = {"Start a new game",
                                   "Load game",
                                   "Exit"};
int MAIN_MENU_SIZE = 3;

const char *DIFFICULTY_MENU_OPTIONS[] = {"Easy",
                                         "Medium",
                                         "Hard"};
int DIFFICULTY_MENU_SIZE = 3;
char *DIFFICULTY_MENU_TITLE = "Choose a difficulty";

const char * HELP = "help";
const char * HELP_MSG = "How to play:\n\n"
                        "\tNOTE: Commands are case and whitespace sensitive\n\n"
                        "\tCommands:\n"
                        "\t  1A 1:              This is how to make moves. The 1A indicates\n"
                        "\t                     the square to be changed and the value after\n"
                        "\t                     is the value to change it to. In this example\n"
                        "\t                     the value in row 1, column A, is changed\n"
                        "\t                     to the value of 1\n"
                        "\t  help:              Displays this message\n"
                        "\t  save FILENAME:     Saves game to output file name provided\n"
                        "\t  hint:              Returns a hint (affects score)\n"
                        "\t  checking {on|off}: Turns checking on or off. Checking is a\n"
                        "\t                     feature that tells you if a move violates\n"
                        "\t                     the rules of sudoku and thus prevents you\n"
                        "\t                     from making it (affects score)\n"
                        "\t  quit:              Quits game without saving\n";

const char * WIN_MESSAGE = "Congratulations, you completed the sudoku correctly!";
const char * SCORE_MESSAGE = "Score is based on difficulty, checks, hints, and time.";
const char * FULL_BUT_INCORRECT = "The board is full but there are errors.";

int const BOX_SIZE = BOARD_SIZE / 3;

enum mainEnum{
    NewGame = 1,
    LoadGame,
    Exit
};

enum difficultyEnum{
    Easy = 1,
    Medium,
    Hard
};

enum moveTypeEnum{
    Move,
    Help,
    Quit,
    Hint,
    Check,
    Check_Toggle,
    Save,
    Error
};

typedef struct BuildValue{
    int value;
    struct BuildValue *next;
} BuildValue;

typedef struct GameStats{
    bool checksOn;
    int elapsedTime;
    int numHints;
    int numChecks;
    int difficulty;
    time_t startTime;
} GameStats;

bool moveIsValid(int row, int col, int value, int **board){
    // Check if in row
    for(int i = 0; i < BOARD_SIZE; i++){
        if(i == col){
            continue;
        }

        if(board[row][i] == value){
            return false;
        }
    }

    // Check if in column
    for(int i = 0; i < BOARD_SIZE; i++){
        if(i == row){
            continue;
        }

        if(board[i][col] == value){
            return false;
        }
    }

    // Check if in box
    int rowStart = row / BOX_SIZE;
    int colStart = col / BOX_SIZE;

    for(int i = rowStart * BOX_SIZE; i < rowStart * BOX_SIZE + BOX_SIZE; i++){
        for(int j = colStart * BOX_SIZE; j < colStart * BOX_SIZE + BOX_SIZE; j++){
            if(i == row && j == col){
                continue;
            }

            if(board[i][j] == value){
                return false;
            }
        }
    }

    return true;
}

void display(int **board){
    // Allocate various spacing/display variables
    int numColSeparators = 2;
    int boardWidth = (NUMBER_SPACING * 2 + 1) * BOARD_SIZE + (NUMBER_SPACING + 1) * numColSeparators + 1;
    char *spacer = malloc(sizeof(char) * NUMBER_SPACING + 1);
    char *separator = malloc(sizeof(char) * boardWidth);

    // Build spacing and separator strings to the width of the board
    for(int i = 0; i < NUMBER_SPACING; i++){
        spacer[i] = ' ';
    }
    spacer[NUMBER_SPACING] = null;

    for(int i = 0; i < boardWidth - 1; i++){
        separator[i] = ROW_SEP_CHAR;
    }
    separator[boardWidth] = null;

    printf("\n");

    // Print column identifier letters
    printf("   "); // for the left border char and row names
    for(int i = 0; i < BOARD_SIZE; i++){
        if(i % BOX_SIZE == 0 && i != 0)
            printf("%s%s", spacer, spacer);

        printf("%s%c%s", spacer, COL_NAMES[i], spacer);
    }
    printf("\n");

    // Print the top border
    printf("  %c%s%c\n", LEFT_RIGHT_BORDER, separator, LEFT_RIGHT_BORDER);

    // Print each row
    for(int i = 0; i < BOARD_SIZE; i++){
        // If divisible by 3, print the box separator
        if(i % BOX_SIZE == 0 && i != 0){
            printf("  %c%s%c\n", LEFT_RIGHT_BORDER, separator, LEFT_RIGHT_BORDER);
        }

        // Print the left border character
        printf("%c %c", ROW_NAMES[i], LEFT_RIGHT_BORDER);

        // Print each column
        for(int j = 0; j < BOARD_SIZE; j++){
            // If divisible by 3, print the box separator
            if(j % BOX_SIZE == 0 && j != 0){
                printf("%s", BOX_BORDER);
            }

            // If the space is blank, then we need to change the print format string
            // so that it doesn't print out 32
            if(board[i][j] == ' '){
                printf("%s%s%s", spacer, (char*) &(board[i][j]), spacer);
            }else{
                printf("%s%d%s", spacer, board[i][j], spacer);
            }
        }

        //Print right border
        printf("%c\n", LEFT_RIGHT_BORDER);
    }

    // Print the bottom border
    printf("  %c%s%c\n", LEFT_RIGHT_BORDER, separator, LEFT_RIGHT_BORDER);

    // Free those space/separator strings
    free(spacer);
    free(separator);
}

char *getMove(){
    char *userInput = malloc(16);
    printf("\nEnter a command. Type 'help' for how to play:\n\n");
    printf("%c ", INPUT_CHAR);
    fgets(userInput, 16, stdin);
    return userInput;
}

bool isMove(const char *move){
    bool validRow = false;
    bool validCol = false;
    bool validVal = false;

    if(strlen(move) - 1 != 4){
        return false;
    }

    if(move[2] != ' '){
        return false;
    }

    for(int i = 0; i < BOARD_SIZE; i++){
        if(ROW_NAMES[i] == move[0])
            validRow = true;

        if(COL_NAMES[i] == move[1])
            validCol = true;

        if(i + 1 + ASCII_NUM_DIFF == move[3])
            validVal = true;
    }

    return validRow && validCol && validVal;

}

bool hasFinished(int **board){
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            if(board[i][j] == SPACE_VAL)
                return false;
        }
    }
    return true;
}

void printHint(int **board, int **solutionBoard){
    int rowCount = 0;
    int colCount = 0;
    bool hintFound = false;
    bool isFull = hasFinished(board);
    time_t t;

    // Generate a random position on the board to start searching for hints
    srand((unsigned) time(&t));
    int curRow = rand() % BOARD_SIZE;
    int origCol = rand() % BOARD_SIZE;

    // Needs to be BOARD_SIZE + 1 because we will end in the same column we started in,
    // processing the columns skipped the first time around
    while(rowCount < BOARD_SIZE + 1 && !hintFound){
        colCount = 0;
        while(colCount < BOARD_SIZE){
            // On the first time through, start at the column we randomly chose
            if(rowCount == 0 && colCount == 0)
                colCount = origCol;

            // If in this state, then we reached our starting point and can stop
            // No hints available (May be a bug, but put this here anyway)
            if(rowCount == BOARD_SIZE + 1 && colCount == origCol)
                break;

            int rowPosition = curRow % BOARD_SIZE;
            int colPosition = origCol % BOARD_SIZE;
            char colLetter;
            if((isFull && solutionBoard[rowPosition][colPosition] !=
                    board[rowPosition][colPosition]) ||
                    (!isFull && board[rowPosition][colPosition] == SPACE_VAL)){
                colLetter = (char) colPosition + ASCII_LETTER_DIFF;
                printf("\nChange the value at %d%c to %d\n", rowPosition + 1, colLetter, solutionBoard[rowPosition][colPosition]);
                hintFound = true;
                break;
            }

            colCount++;
        }
        rowCount++;
        curRow++;
    }

    if(!hintFound){
        printf("\nNo hints available");
    }
}

int doMove(int ***board, const char *move, int **solutionBoard, GameStats *stats){
    if(strcmp(move, "quit\n") == 0){
        return Quit;
    }

    if(strcmp(move, "help\n") == 0){
        printf("\n%s\n", HELP_MSG);
        return Help;
    }

    if(strcmp(move, "hint\n") == 0){
        printHint(*board, solutionBoard);
        return Hint;
    }

    // In this sense isMove means "will it change the value on the board?"
    if(isMove(move)){
        int rowPosition = move[0] - ASCII_NUM_DIFF - 1;
        int colPosition = move[1] - ASCII_LETTER_DIFF;
        int value = move[3] - ASCII_NUM_DIFF;

        if(stats->checksOn && !moveIsValid(rowPosition, colPosition, value, *board)){
            printf("\nChecks are on. This move violates constraints on a winning board.");
            return Check;
        }else{
            (*board)[rowPosition][colPosition] = value;
        }

        return Move;
    }
}

bool hasWon(int **board){
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            if(board[i][j] == SPACE_VAL)
                return false;
            if(!moveIsValid(i, j, board[i][j], board))
                return false;
        }
    }
    return true;
}

int calculateScore(GameStats *stats){
    time_t endTime = time(null);
    int totalTime = (int)(endTime - stats->startTime) + stats->elapsedTime;
    int score = BASE_SCORE;
    score -= totalTime / TIME_COST_FACTOR;
    score -= (BASE_SCORE - score) * DIFFICULTY_COST_FACTOR * (Hard - stats->difficulty);
    score -= stats->numHints * HINT_COST;
    score -= stats->numChecks * CHECK_COST;
    return score;
}

void play(int **board, int **solutionBoard, GameStats *stats){
    display(board);
    while(true){
        bool isQuit = false;
        bool isSaved = false;
        bool isHelp = false;
        bool isMove = false;

        const char *move = getMove();
        enum moveTypeEnum moveType = doMove(&board, move, solutionBoard, stats);
        free((void *) move);

        switch(moveType){
            case Move:
                isMove = true;
                break;
            case Help:
                isHelp = true;
                break;
            case Quit:
                isQuit = true;
                break;
            case Hint:
                stats->numHints++;
                break;
            case Check:
                stats->numChecks++;
                break;
            case Check_Toggle:
                stats->checksOn = !stats->checksOn;
                break;
            case Save:
                isSaved = true;
                break;
            default:
                break;
        }
        
        if(isQuit){
            break;
        }

        if(isSaved){
            break;
        }

        if(!isHelp)
            display(board);

        if(isMove && hasWon(board)){
            printf("\n%s", WIN_MESSAGE);
            printf("\nScore is %d/%d", calculateScore(stats), BASE_SCORE);
            printf("\n%s\n", SCORE_MESSAGE);
            break;
        } else if(isMove && hasFinished(board)){
            printf("\n%s\n", FULL_BUT_INCORRECT);
        }
    }
}

void freeBuildValue(BuildValue *value){
    BuildValue *nextValue;
    while(value != null){
        nextValue = value->next;
        free(value);
        value = nextValue;
    }
}

int getBuildValue(BuildValue *value, int index){
    BuildValue *currValue = value;
    for(int i = 0; i < index; i++){
        currValue = currValue->next;
    }

    return currValue->value;
}

BuildValue *initBuildValue(){
    BuildValue *returnValue = malloc(sizeof(BuildValue));
    returnValue->value = -1;
    returnValue->next = null;
    return returnValue;
}

void addBuildValue(BuildValue **value, int addValue){
    if((*value)->value == -1){
        (*value)->value = addValue;
        return;
    }

    BuildValue *curr = *value;
    while(curr->next != null){
        curr = curr->next;
    }

    curr->next = malloc(sizeof(BuildValue));
    curr->next->value = addValue;
    curr->next->next = null;
}

void freeBoard(int **board){
    for(int i = 0; i < BOARD_SIZE; i++){
        free(board[i]);
    }

    free(board);
}

int getNextCol(int **board, int row, BuildValue **colChoices, int *numChoices){
    int leastChoices = BOARD_SIZE + 1;
    int nextCol;
    BuildValue *nextColVals = null;

    // Loop row
    for(int i = 0; i < BOARD_SIZE; i++){
        int numChoices = 0;
        BuildValue *currColVals = initBuildValue();
        // Loop 1 to 9
        for(int j = 1; j <= BOARD_SIZE; j++){
            if(board[row][i] != -1)
                break;

            if(moveIsValid(row, i, j, board)){
                numChoices++;
                addBuildValue(&currColVals, j);
            }
        }

        if(numChoices < leastChoices && numChoices != 0){
            leastChoices = numChoices;
            nextCol = i;
            freeBuildValue(nextColVals);
            nextColVals = currColVals;
        }else{
            freeBuildValue(currColVals);
        }
    }

    *colChoices = nextColVals;
    *numChoices = leastChoices;
    if(*numChoices == BOARD_SIZE + 1)
        *numChoices = 0;

    return nextCol;
}

void removeValues(int difficulty, int ***board){
    int numToRemove;
    time_t t;
    srand((unsigned) time(&t));
    int row;
    int col;
    int numRemoved = 0;


    if(difficulty == Easy)
        numToRemove = EASY_REMOVE;
    else if(difficulty == Medium)
        numToRemove = MED_REMOVE;
    else
        numToRemove = HARD_REMOVE;

    while(numRemoved < numToRemove){
        row = rand() % BOARD_SIZE;
        col = rand() % BOARD_SIZE;
        if((*board)[row][col] != EMPTY_CHAR){
            (*board)[row][col] = EMPTY_CHAR;
            numRemoved++;
        }
    }
}

void initBoard(int ***board){
    for(int i = 0; i < BOARD_SIZE; i++){
        int *row = malloc(sizeof(int) * BOARD_SIZE);
        (*board)[i] = row;
        for(int j = 0; j < BOARD_SIZE; j++){
            (*board)[i][j] = -1;
        }
    }
}

int **generateBoard(int difficulty, int ***solutionBoard){
    time_t start = time(null);
    time_t t;
    int **board = malloc(sizeof(int *) * BOARD_SIZE);
    *solutionBoard = malloc(sizeof(int *) * BOARD_SIZE);
    initBoard(&board);
    initBoard(solutionBoard);

    srand((unsigned) time(&t));

    for(int i = 0; i < BOARD_SIZE; i++){
        BuildValue *rowChoices = initBuildValue();
        int maxRange = BOARD_SIZE;
        for(int j = 0; j < BOARD_SIZE; j++){
            int nextCol;
            int numChoices;
            int rNum;
            BuildValue *colChoices;

            // Looks like we're stuck. Let's try again
            time_t now = time(null);
            if((now - start) > TIMEOUT){
                initBoard(&board);
                i = -1;
                start = time(null);
                break;
            }

            nextCol = getNextCol(board, i, &colChoices, &numChoices);

            // If no choices, clear out row and start over
            if(numChoices == 0){
                for(int k = 0; k < BOARD_SIZE; k++){
                    board[i][k] = -1;
                    (*solutionBoard)[i][k] = -1;
                }
                j = -1;
            }else{
                rNum = (rand() % numChoices);
                board[i][nextCol] = getBuildValue(colChoices, rNum);
                (*solutionBoard)[i][nextCol] = board[i][nextCol];
            }

            freeBuildValue(colChoices);
        }
    }

    removeValues(difficulty, &board);

    return board;
}

int getListOption(char *menu_title, const char **list_options, int numOptions){
    bool validOption = false;
    int optionChoice;

    while(!validOption){
        printf("%s:\n\n", menu_title);
        for(int i = 0; i < numOptions; i++){
            printf("\t%d) %s\n", i + 1, list_options[i]);
        }
        printf("\n");
        printf("%c ", INPUT_CHAR);
        scanf("%d", &optionChoice);
        scanf("%*[^\n]%*c");

        if(optionChoice < 1 || optionChoice > numOptions){
            printf("\nInvalid choice\n\n");
            continue;
        }else{
            validOption = true;
        }

    }

    return optionChoice;
}

int main(){
    while(true){
        printf("Welcome to Dylan's Sudoku!\n\n");
        enum mainEnum listOption = 
            getListOption(MAIN_MENU_TITLE, MAIN_MENU_OPTIONS, MAIN_MENU_SIZE);

        switch(listOption){
            case NewGame:
                printf("\n");
                enum difficultyEnum difficulty = 
                    getListOption(DIFFICULTY_MENU_TITLE, DIFFICULTY_MENU_OPTIONS, DIFFICULTY_MENU_SIZE);
                printf("\nPlease wait while board generates...\n");
                int **solutionBoard;
                int **board = generateBoard(difficulty, &solutionBoard);

                GameStats stats;
                memset(&stats, sizeof(GameStats), '\0');
                stats.startTime = time(null);
                stats.difficulty = difficulty;

                play(board, solutionBoard, &stats);
                freeBoard(board);
                freeBoard(solutionBoard);
                break;
            case LoadGame:
                break;
            case Exit:
                exit(0);
        }

        printf("\n");
    }

}