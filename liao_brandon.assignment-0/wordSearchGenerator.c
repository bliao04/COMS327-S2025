#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SIZE 20

// Directions: Right, Down, Diagonal Down, Diagonal Up
const int directions[4][2] = {
    {0, 1},   // Right
    {1, 0},   // Down
    {1, 1},   // Diagonal Down
    {-1, 1}   // Diagonal Up
};

// Function to initialize the grid with empty spaces
void initializeGrid(char grid[MAX_SIZE][MAX_SIZE], int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            grid[i][j] = '.';  // Placeholder for empty space
        }
    }
}

// Function to check if a word fits in the grid at a given position and direction
int canPlaceWord(char grid[MAX_SIZE][MAX_SIZE], int size, const char *word, int row, int col, int dir) {
    int len = strlen(word);
    int dr = directions[dir][0];
    int dc = directions[dir][1];

    for (int i = 0; i < len; i++) {
        int r = row + i * dr;
        int c = col + i * dc;

        if (r < 0 || r >= size || c < 0 || c >= size) {
            return 0;  // Out of bounds
        }

        if (grid[r][c] != '.' && grid[r][c] != word[i]) {
            return 0;  // Conflict with existing letter
        }
    }

    return 1;  // Word can be placed
}

// Function to place a word in the grid
void placeWord(char grid[MAX_SIZE][MAX_SIZE], const char *word, int row, int col, int dir) {
    int len = strlen(word);
    int dr = directions[dir][0];
    int dc = directions[dir][1];

    for (int i = 0; i < len; i++) {
        grid[row + i * dr][col + i * dc] = word[i];
    }
}

// Function to fill the empty spaces in the grid with random letters
void fillEmptySpaces(char grid[MAX_SIZE][MAX_SIZE], int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (grid[i][j] == '.') {
                grid[i][j] = 'A' + rand() % 26;  // Random letter from A to Z
            }
        }
    }
}

// Function to print the grid
void printGrid(char grid[MAX_SIZE][MAX_SIZE], int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            printf("%c ", grid[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <size> <word1> <word2> ...\n", argv[0]);
        return 1;
    }

    int size = atoi(argv[1]);
    if (size <= 0 || size > MAX_SIZE) {
        fprintf(stderr, "Grid size must be between 1 and %d.\n", MAX_SIZE);
        return 1;
    }

    char grid[MAX_SIZE][MAX_SIZE];
    initializeGrid(grid, size);

    srand(time(NULL));

    // Insert words into the grid
    for (int w = 2; w < argc; w++) {
        const char *word = argv[w];
        int placed = 0;

        for (int attempt = 0; attempt < 100 && !placed; attempt++) {
            int row = rand() % size;
            int col = rand() % size;
            int dir = rand() % 4;

            if (canPlaceWord(grid, size, word, row, col, dir)) {
                placeWord(grid, word, row, col, dir);
                placed = 1;
            }
        }

        if (!placed) {
            fprintf(stderr, "Failed to place word: %s\n", word);
            return 1;
        }
    }

    // Fill remaining spaces with random letters
    fillEmptySpaces(grid, size);

    // Print the grid and word list
    printGrid(grid, size);
    printf("\nWords to find:\n");
    for (int w = 2; w < argc; w++) {
        printf("%s\n", argv[w]);
    }

    return 0;
}