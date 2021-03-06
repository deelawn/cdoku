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
#define SAVE_N_CMP 5
#define CIPHER_OFFSET 30

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

// Check whether a move violates the constraints of a winning sudoku board
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

// Display the board in its current state. This get called
// after most moves/actions
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

// Prompt the user for a move/input action
char *getMove(){
    char *userInput = malloc(16);
    printf("\nEnter a command. Type 'help' for how to play:\n\n");
    printf("%c ", INPUT_CHAR);
    fgets(userInput, 16, stdin);
    return userInput;
}

// Check whether a move is valid
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

// Check if the board is full.  This is used to see if the board
// is full when it is not in a winning state, and will result in
// the user being alerted as such
bool hasFinished(int **board){
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            if(board[i][j] == SPACE_VAL)
                return false;
        }
    }
    return true;
}

// Print a hint that will affect the user's score. A hint looks at the
// solution board and tells users which move to make to get closer
// to the solution
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

// Parse a move entered by the user and take the appropriate action
int doMove(int ***board, const char *move, int **solutionBoard, GameStats **stats){
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

    if(strcmp(move, "checking on\n") == 0){
        (*stats)->checksOn = true;
        printf("\n%s\n", "checking is turned on");
        return Check_Toggle;
    }

    if(strcmp(move, "checking off\n") == 0){
        (*stats)->checksOn = false;
        printf("\n%s\n", "checking is turned off");
        return Check_Toggle;
    }

    if(strncmp(move, "save ", SAVE_N_CMP) == 0){
        return Save;
    }

    // In this sense isMove means "will it change the value on the board?"
    if(isMove(move)){
        int rowPosition = move[0] - ASCII_NUM_DIFF - 1;
        int colPosition = move[1] - ASCII_LETTER_DIFF;
        int value = move[3] - ASCII_NUM_DIFF;

        if((*stats)->checksOn && !moveIsValid(rowPosition, colPosition, value, *board)){
            printf("\nChecks are on. This move violates constraints on a winning board.\n");
            return Check;
        }else{
            (*board)[rowPosition][colPosition] = value;
        }

        return Move;
    }

    return Error;
}

// Called after each move that alters the board to check
// if the board is now in a winning state
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

// Upon winning the game, use the game stats to calculate the score
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

// Save the game by writing out the board, solution board, and stats, all of which are
// rudimentary encrypted using a simple cipher, to a file of the name of the user's choosing
bool saveGame(int **board, int **solutionBoard, GameStats *stats, char *filename){
    // Try to open file
    FILE *fp;
    fp = fopen(filename, "w");

    // File open failed
    if(fp == null){
        return true;
    }

    // Write board all on one line
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            fprintf(fp, "%c", (char)(board[i][j] + CIPHER_OFFSET));
        }
    }

    fprintf(fp, "\n");

    // Write solution board all on one line
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            fprintf(fp, "%c", (char)(solutionBoard[i][j] + CIPHER_OFFSET));
        }
    }

    fprintf(fp, "\n");

    // Write each game stat on a separate line
    fprintf(fp, "%d\n", stats->checksOn + CIPHER_OFFSET);
    fprintf(fp, "%d\n", stats->elapsedTime + (int)(time(null) - stats->startTime) + CIPHER_OFFSET);
    fprintf(fp, "%d\n", stats->numHints + CIPHER_OFFSET);
    fprintf(fp, "%d\n", stats->numChecks + CIPHER_OFFSET);
    fprintf(fp, "%d\n", stats->difficulty + CIPHER_OFFSET);
    fprintf(fp, "\n");

    fclose(fp);

    printf("\nGame saved\n");

    return 0;
}

// If the user chose to save the game, then parse the move text to grab
// the name of the file they entered
char *getSaveFilename(const char *move){
    char *filename = malloc(strlen(move) - SAVE_N_CMP);
    strncpy(filename, &move[SAVE_N_CMP], strlen(&move[SAVE_N_CMP]) - 1);
    return filename;
}

// Loop that processes each turn/action made while playing the 
// sudoku game
void play(int **board, int **solutionBoard, GameStats *stats){
    display(board);
    while(true){
        bool isQuit = false;
        bool isSaved = false;
        bool isHelp = false;
        bool isMove = false;
        bool isError = false;

        const char *move = getMove();
        enum moveTypeEnum moveType = doMove(&board, move, solutionBoard, &stats);

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
            case Save:
                isSaved = true;
                break;
            case Error:
                isError = true;
                break;
            default:
                break;
        }

        if(isError){
            printf("\nInvalid input\n");
        }

        if(isSaved){
            char *saveFile = getSaveFilename(move);
            bool saveFailed = saveGame(board, solutionBoard, stats, saveFile);
            free(saveFile);

            if(saveFailed){
                printf("\nFailed to save game\n");
            }else{
                free((void *) move);
                break;
            }
        }

        free((void *) move);
        
        if(isQuit){
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

// Free memory allocated for a build value linked list
void freeBuildValue(BuildValue *value){
    BuildValue *nextValue;
    while(value != null){
        nextValue = value->next;
        free(value);
        value = nextValue;
    }
}

// Get the ith element from a build value linked list
int getBuildValue(BuildValue *value, int index){
    BuildValue *currValue = value;
    for(int i = 0; i < index; i++){
        currValue = currValue->next;
    }

    return currValue->value;
}

// Initialize a build value linked list
BuildValue *initBuildValue(){
    BuildValue *returnValue = malloc(sizeof(BuildValue));
    returnValue->value = -1;
    returnValue->next = null;
    return returnValue;
}

// Add a value to the build value linked list
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

// Free all memory allocated for a board
void freeBoard(int **board){
    if(board == null)
        return;

    for(int i = 0; i < BOARD_SIZE; i++){
        if(board[i] != null)
            free(board[i]);
    }

    free(board);
}

// Returns the next column that will be filled in with a value when generating a board
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

// After generating a board, remove a certain number of values
// depending on the difficulty chosen
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

void initStats(GameStats *stats, int difficulty){
    memset(stats, sizeof(GameStats), '\0');
    stats->startTime = time(null);
    stats->difficulty = difficulty;
}

bool loadBoard(FILE *fp, int ***board){
    *board = malloc(sizeof(int *) * BOARD_SIZE);
    initBoard(board);
    char boardLine[BOARD_SIZE * BOARD_SIZE + 2];
    void *result = (void *)fgets(boardLine, sizeof(boardLine), fp);
    if(result == null){
        return true;
    }

    int count = 0;
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            (*board)[i][j] = (int)boardLine[count] - CIPHER_OFFSET;

            // Validate values that are being read in
            if(!(((*board)[i][j] >= 1 && (*board)[i][j] <= 9) || (*board)[i][j] == (int)SPACE_VAL))
                return true;

            count++;
        }
    }

    return false;
}

bool readStatsInt(FILE *fp, int *stat){
    char *nextLine = malloc(64);
    void *result = (void *)fgets(nextLine, sizeof(nextLine), fp);
    if(result == null){
        free(nextLine);
        return true;
    }

    // Remove the newline character
    nextLine[strlen(nextLine) - 1] = '\0';

    // Read the integer
    int readInt = sscanf(nextLine, "%d", stat);
    if(!readInt){
        return true;
    }

    *stat -= CIPHER_OFFSET;
    return false;
}

bool loadStats(FILE *fp, GameStats *stats){    
    return (readStatsInt(fp, &stats->checksOn) || readStatsInt(fp, &stats->elapsedTime) ||
        readStatsInt(fp, &stats->numHints) || readStatsInt(fp, &stats->numChecks) ||
        readStatsInt(fp, &stats->difficulty));
}

bool loadGame(int ***board, int ***solutionBoard, GameStats *stats){
    // Prompt for and read in game name
    char *gameName = malloc(256);
    printf(" ");
    // Do an fgets to clear the input buffer
    fgets(gameName, 256, stdin);
    printf("\nType the name of the name you'd like to load:\n\n");
    printf("%c ", INPUT_CHAR);
    fgets(gameName, 256, stdin);
    gameName[strlen(gameName) - 1] = '\0';

    // Try to open the file
    FILE *fp;
    fp = fopen(gameName, "r");
    free(gameName);

    // File open failed
    if(fp == null){
        return true;
    }

    // Read in boards and return if an error is encountered
    if(loadBoard(fp, board) || loadBoard(fp, solutionBoard))
        return true;

    // Read in the stats
    if(loadStats(fp, stats))
        return true;

    // Set the new start time
    stats->startTime = time(null);

    fclose(fp);
    return false;
}

// A generic routine for displaying a list of options and returning the user's choice
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

// Can be used for debugging purposes
void printStats(GameStats *stats){
    printf("Checks on: %d\n", stats->checksOn);
    printf("Elapsed time: %d\n", stats->elapsedTime);
    printf("Num hints: %d\n", stats->numHints);
    printf("Num checks: %d\n", stats->numChecks);
    printf("Difficulty: %d\n", stats->difficulty);
    printf("Start time: %d\n", (int)stats->startTime);
}

int main(){
    while(true){
        printf("Welcome to Dylan's Sudoku!\n\n");
        enum mainEnum listOption = 
            getListOption(MAIN_MENU_TITLE, MAIN_MENU_OPTIONS, MAIN_MENU_SIZE);

        int **board = null;
        int **solutionBoard = null;
        GameStats stats;

        switch(listOption){
            case NewGame:
            {
                printf("\n");
                enum difficultyEnum difficulty = 
                    getListOption(DIFFICULTY_MENU_TITLE, DIFFICULTY_MENU_OPTIONS, DIFFICULTY_MENU_SIZE);
                printf("\nPlease wait while board generates...\n");
                board = generateBoard(difficulty, &solutionBoard);

                initStats(&stats, difficulty);
                char junk[256];
                // Clear stdin buffer
                fgets(junk, 256, stdin);
                play(board, solutionBoard, &stats);
                freeBoard(board);
                freeBoard(solutionBoard);
                break;
            }
            case LoadGame:
            {
                bool loadOkay = !loadGame(&board, &solutionBoard, &stats);
                if(loadOkay){
                    play(board, solutionBoard, &stats);
                }else{
                    printf("\n\nInvalid or nonexistent file.\n\n");
                }

                freeBoard(board);
                freeBoard(solutionBoard);
                break;
            }
            case Exit:
                exit(0);
        }

        printf("\n");
    }

}