#include <stdio.h>
#include <stdint.h>

#define CHUNKS 4

typedef uint64_t chunk;

typedef struct board {
    chunk matrix[CHUNKS];
    int width;
    int rows;
    int overflow;
    int height;
} board;

const chunk LINES[6] = {
    0x00000000000003ff,
    0x00000000000ffc00,
    0x000000003ff00000,
    0x000000ffc0000000,
    0x0003ff0000000000,
    0x0ffc000000000000
};

const chunk SANATIZE = 0x0fffffffffffffff;

void print_board(board* b);
void clear_lines(board* b, int lines, int i);
int clear_check(board* b, int i);

int main(void) {
    board b;

    b.width = 10;
    b.rows = 6;
    b.overflow = 4;
    b.height = 24;

    b.matrix[3] = 0x0000000000000000;
    b.matrix[2] = 0x0000000000000000;
    b.matrix[1] = 0x000000f000ff000f;
    b.matrix[0] = 0x0ffaaafffff556aa;

    print_board(&b);
    int x = clear_check(&b, 0);
    printf("%i\n", x);
    x = clear_check(&b, 0);
    printf("%i\n", x);
    print_board(&b);
}


void print_board(board* b) {
    // Number of bits in the board
    int n = 64 * CHUNKS;

    unsigned long long a;
    char final[n + 1];

    // Populate string final with bits
    // One chunk at a time
    for (int i = 0; i < CHUNKS; ++i) {
        a = b->matrix[i];
        do {
            final[--n] = (a & 0x0000000000000001) ? '1' : '0';
            a >>= 1;
        } while (n % 64);

    }

    // For each chunk
    for (int i = 0; i < CHUNKS; ++i) {

        for (int j = 0; j < 64; ++j) {
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

void clear_lines(board* b, int lines, int i) {

    b->matrix[i++] >>= (lines * b->width);

    while (i < CHUNKS) {
        b->matrix[i - 1] |= ( b->matrix[i] << ((b->rows - lines) * b->width) );
        b->matrix[i++] >>= (lines * b->width);
    }
}

int clear_check(board* b, int i) {
    chunk mask, pres, row;
    mask = 0x0000000000000000;
    pres = 0x0000000000000000;

    int lines = 0;

    for (int j = 0; j < b->rows; ++j) {
        row = LINES[j] & b->matrix[i];
        printf("%016llx\n", row);

        if (row == LINES[j])
            ++lines;
        else {
            if (lines)
                break;
            mask = mask | LINES[j];
            pres = pres | row;
        }
    }

    if (!lines)
        return 0;

    clear_lines(b, lines, i);

    b->matrix[i] = (b->matrix[i] & (~mask)) | pres;

    return lines;
}

