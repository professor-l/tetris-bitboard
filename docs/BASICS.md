# A Tetris Bit Matrix

The goal of this project is to establish a so-called "bit matrix" for representing a Tetris playfield such that it can be manipulated entirely with bitwise operators, which provide several advantages over the traditional data structures used in Tetris implementations (such as a two-dimensional boolean or integer array). In addition to being implementable on nearly all hardware (a 10x40 matrix as specified in the Tetris Guideline requires only seven unsigned 64-bit integers), bitwise operators are at least as fast as standard integer addition or multiplication on modern CISC processors, and even faster on low-cost microcontrollers and SBC's.

To determine how a system like this is best implemented, we need to understand what operations are performed on a Tetris playfield. We imagine a matrix of squares with the following capabilities:

1. Shifting portions of a matrix in any cardinal direction
2. Rotating portions of a matrix about a dynamic axis
2. Collision detection between two distinguishable "sections" of the matrix (i.e. active piece vs. existing playfield)

Any more complex manipulations (like SRS wallkicks or topout conditions) can be written as abstractions of these four core features. Therefore, if we can implement all four through pure bit-twiddling, we should be able to use them to build an entire Tetris engine.

## The protocol

As suggested by community member fractal161, I take inspiration from the bitboards of Chess programming, a well-established research field sapanning several decades. They use bit manipulation to represent and alter chess boards as unsigned 64-bit integers, where each variable represents specific information about the board, such as color or piece type. Their techniquies are documented extensively, and I skimmed many of their articles prior to and throughout this project.

A matrix of dimensions `m,n` will be defined here. In practice, we can assume that `m = 10` and `20 <= n <= 40`, but for simplicity I start by assuming `m=5` and `n=9`. This allows simpler illustrations, but still forces me to account for overflow bits. For instance, a 5x9 matrix contains 45 bits, and again for simplicity I'll use unsigned 16-bit integers in memory. One potential layout of such a matrix is presented below:

```
0 0 0 0 0
0 0 0 0 0
1 0 0 0 0 [0]
_________
1 0 0 0 0
1 0 0 0 0
1 1 0 0 0 [0]
_________
1 1 1 0 0
1 1 1 1 0
1 1 1 1 0 [0]

```

Which would appear in memory as:
```
{0000000000100000, 1000010000110000, 1110011110111100}
{32, 33840, 59324}
```

Where the [bracketed] bits are "overflow" bits from a particular memory word that exist but are not part of the matrix, and the barriers denote the end of one word and the beginning of another. Another potential representation, with an alternate layout between bits, would be as follows:

```
[0] 0 0 0 0 0
    0 0 0 0 0
    1 0 0 0 0
    _________
[0] 1 0 0 0 0
    1 0 0 0 0
    1 1 0 0 0
    _________
[0] 1 1 1 0 0
    1 1 1 1 0
    1 1 1 1 0
```

I showed two possible bit matrix representations here, but in my implementation I use the latter. Aside from helping me think about things, as far as I know this provides no meaningful advantage, but it's what I went with. So what we have is a board with a few properties:

- The number of bits `b` in each "chunk" (this will be 64 here, and probably should be at least 64 everywhere else too)
- The number of chunks `n` in the set (this will be the number of distinct memory words required to represent the board)
- The width of the board `w`
- The rows per chunk `r`
- The number of overflow bits `p` (The bits at the beginning of each chunk that are not part of the matrix itself)
- The height of the board `h`

This allows us to observe a number of mathematical relationships between properties, including:

- `w = (b - p) / r`, or simply `w = b / r` with truncation (this relationship holds swapping `r` and `w`
- `p - b % w`
- `h = r * n`, `r = h / n`, and `n = h / r`

Using this, we could theoretically derive all properties purely from `n` and `w`, but in practice, when we introduce piece spawn locations and other game-related functionality, additional variables will be added, so for now we assume explicit declaration of each one.

Now, *finally*, we can write just a bit of code:

```c
#include <stdint.h>

#define CHUNKS 4

typedef struct board {
    unsigned long long matrix[CHUNKS];
    int width;
    int rows;
    int overflow;
    int height;
} board;
```

I also implemented a function to print the board out in a legible manner with the signature `void print_board(board* b)`, but it exists for debugging and development purposes, and it's kind of long and ugly, so for now it exists only in the codebase, and it's not overviewed here.

This is the basics taken care of - we have a matrix now. The next step is the simplest and most frequenty board manipulation in the game of Tetris: the line clear.


