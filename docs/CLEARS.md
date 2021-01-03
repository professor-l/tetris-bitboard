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

In practice, we rearrange slightly - steps 2 and 3 are combined into a single line of code, and we group step 1 with steps 2 and 3 of the previous iteration, allowing the first step of the first iteration to stand alone. In doing so, the following function is produced:

```c
void clear_lines(board* b, int lines) {

    b->matrix[0] >>= (lines * b->width);

    for (int i = 1; i < CHUNKS; i++) {
        b->matrix[i - 1] |= ( b->matrix[i] << ((b->rows - lines) * b->width) )
        b->matrix[i] >>= (lines * b->width);
    }
}
```

Given a board pointer and a number of lines to clear, it will do exactly as our algorithm described, and as such, our line clear function is complete.
