#include <stdio.h>
#include <stdint.h>

#define CHUNKS 4

typedef struct board {
    unsigned long long matrix[CHUNKS];
    int width;
    int rows;
    int overflow;
    int height;
} board;

void print_board(board* b);
void clear_lines(board* b, int lines);

int main(void) {
    board b;

    b.width = 10;
    b.rows = 6;
    b.overflow = 4;
    b.height = 24;

    b.matrix[3] = 0x0000000000000000;
    b.matrix[2] = 0x0000000000000000;
    b.matrix[1] = 0x000000f000ff000f;
    b.matrix[0] = 0x0556aa556aa556aa;

    print_board(&b);
    clear_lines(&b, 3);
    print_board(&b);
}

void print_board(board* b) {
    // Number of bits in the board
    int n = 64 * CHUNKS;

    unsigned long long a;
    char final[n + 1];

    // Populate string final with bits
    // One chunk at a time
    for (int i = 0; i < CHUNKS; i++) {
        a = b->matrix[i];
        do {
            final[--n] = (a & 0x0000000000000001) ? '1' : '0';
            a >>= 1;
        } while (n % 64);

    }

    // For each chunk
    for (int i = 0; i < CHUNKS; i++) {

        for (int j = 0; j < 64; j++) {
            char c = final[ i * 64 + j ];

            // If we just finished a row
            if (j % b->width == b->overflow && j > b->overflow) {

                printf("\n");
                // Overflow whitespace
                for (int k = 0; k < b->overflow; k++)
                    printf(". ");
            }

            // Print actual character
            printf("%c ", c);
        }
        printf("\n");
    }
    printf("\n");
}

void clear_lines(board* b, int lines) {

    b->matrix[0] >>= (lines * b->width);

    for (int i = 1; i < CHUNKS; i++) {
        b->matrix[i - 1] |= ( b->matrix[i] << ((b->rows - lines) * b->width) );
        b->matrix[i] >>= (lines * b->width);
    }
}






