# Line Clears

Recall our variable definitions:

- The number of bits per chunk `b` (64)
- The number of chunks in the set `n`
- The width of the board `w`
- The rows per chunk `r`
- The number of overflow bits `p`
- The height of the board `h`

Line clears aren't terribly complex, but the addition of multiple consecutive `long long`s in memory does complicate them slightly. In order to properly implement them, I take advantage of the key assumption that I will never want to clear more than four lines at once (`l <= 4`), but each chunk within the board stores six lines (`r = 6`). In other words, I can always assume that `l < r`.

Because of this, I can effectively "cascade" the effects of a line clear up the matrix, one chunk at a time. For a single chunk, a line clear of `l` lines is merely executed as a right bitshift on the chunk of `l` rows, or `l * w` bits. This brings in 0's from the left to fill the gap which, in a multi-chunk matrix, we must then repopulate with the bottom `l` rows of the chunk immediately above this one (the next chunk in our array).

One mechanism for this repopulation is to imagine a *new* chunk C that contains the necessary `l` missing rows at the top, and empty (all-0) rows at the bottom. Such a chunk can be created by bitshifting the next chunk to the **left** instead of the right, and doing so for `r - l` rows, or `(r - l) * w` bits. This operation would yield the chunk C that we imagined, which we can then combine with our first chunk using a bitwise OR operator to achieve the desired final state. This process can be repeated for each chunk, skipping the repopulation step for the last one.

So, to recap, our steps are as follows:

1. `B[1] >>= (l * w)` - Bitshift `B[1]`, our first chunk, `l * w` bits to the right
2. `C = B[2] << ((r - l) * w)` - Create a new chunk `C`, defined as `B[2]` (the next chunk) bitshifted `(r - l) * w` bits to the left
3. `B[1] |= C` - Define `B[1]` as the union of itself with `C`
4. Repeat 1-3 for all chunks, skipping 2 and 3 for the final chunk

In practice, we rearrange slightly - steps 2 and 3 are combined into a single line of code, and we group step 1 with steps 2 and 3 of the previous iteration, allowing the first step of the first iteration to stand alone. We also abstract slightly to allow the function to start from any chunk, not just the bottom, and in doing so, the following function is produced:

```c
void clear_lines(board* b, int lines, int i) {

    b->matrix[i++] >>= (lines * b->width);

    while (i < CHUNKS) {
        b->matrix[i - 1] |= ( b->matrix[i] << ((b->rows - lines) * b->width) );
        b->matrix[i++] >>= (lines * b->width);
    }
}
```

Given a board pointer and a number of lines to clear, it will do exactly as our algorithm described, and as such, this line clear function is complete. However, in practice this function only works if we're clearing lines off the bottom of the stack, and that's not the only circumstance in which lines are cleared. Therefore, we have some more thinking to do.

For the sake of efficiency, we will combine the line clear *checking* function with the line *clearing* function, because in practice doing one without the other isn't necessary, and the algorithm flows quite nicely serving this dual purpose.

We imagine executing this one chunk at a time, and as such we design it to act on a single chunk. The basic process will be as follows:

1. Iteratively, starting from the bottom row and going up, we check to see if the row is complete. If not, we "store" a copy of the row in a new temporary chunk from which it can later be restored. If it is, we check the rows above it for the total number of consecutive clearble lines, storing that value (the number of lines to clear) as `l` and stopping the loop.

2. Once we have a value `l`, we clear that many lines off of the *bottom* of the chunk using our established algorithm (see above).

3. Finally, we replace the bottom `l` rows of the new chunk with the preserved rows from step 1.

This is not overly complex, but it does present a few challenges that we have to think carefully about in order to implement the algorithm with bitwise operators alone. For us, it is adventageous to define a series of constants `X[1], X[2]... X[n]`, where each one is a chunk that is all empty, except for one row that is completely full, and `n = r` for our board's `r`. For our application, X[1] has the bottom row filled and X[n] has the top row filled.

With this construct, we can implement the algorithm concretely as follows:

```
To clear lines from some chunk C:
--------------------------------

Define A, B, L as empty chunks
Define <lines> as 0

For each element X[i] of the array of line constants:
    Set L = X[i] & C
    if (L = X[i]):
        set <lines> = <lines> + 1
    else:
        if (<lines>  >  0), BREAK loop
        Set A = A | X[i]
        Set B = B | L

if (<lines> = 0)
    return 0

Call CLEAR function with chunk C and line count <lines>

Set C = (C & (~A)) | B
return <lines>
```

Essentially, what we do is use `A` and `B` to preserve uncleared rows at the bottom. `A` is filled with complete rows, while `B` is filled with copies of rows, as we move up the chunk. At each iteration,`L` is an exact copy of the current row, surrounded by empty ones, and is acquired by masking (intersecting) `C` with `X[i]`. We determine whether a row can be cleared based on whether its representation in `L` is identical to the current `X[i]`. Finally, at the end, we take the new value of C post-clears and **empty the bottom `l` rows** by masking it with the *complement* of `A`, then take the union of that with B, our chunk with preserved rows, to finally reach our final product.

In C, that looks like this:

```c
const chunk LINES = {
    0x00000000000003ff,
    0x00000000000ffc00,
    0x000000003ff00000,
    0x000000ffc0000000,
    0x0003ff0000000000,
    0x0ffc000000000000
};

int clear_check(board* b, int i) {
    chunk mask, pres, row;
    mask = 0x0000000000000000;
    pres = 0x0000000000000000;

    int lines = 0;

    for (int j = 0; j < b->rows; j++) {
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
```

The main drawback of this method is that it only detects and clears *consecutive* lines within the chunk, and needs to be called multiple times in some cases - specifically, a maximum of two times, at least with tetrominos. It is rather complex, but it is also effective and quick, and it means that line clearing is (finally) fully implemented.

