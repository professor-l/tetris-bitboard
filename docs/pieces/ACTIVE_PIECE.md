# The Active Piece

In Tetris, it's useful to think of the playfield as largely static at any given point in time, and distinguish it from the **active piece** - the one that user inputs are actively maneuvering. For our application, we want the active piece to indeed be totally distinct from the playfield, for lightning-quick manipulation, but we also want fast collision detection between the actice piece and the rest of the matrix. The solution to this is one that may seem space-inefficient, but which in fact gives us the greatest freedom and efficiency when dealing with shifting and rotating the active piece: To have an entirely separate board, identical in all fundamental properties (`n`, `w`, `r`, etc), but which is empty aside from the active piece.

The details of the techniques here are important. Each decision is made deliberately with speed and ease of implementation in mind. Within a chunk, shifting a piece horizontally is trivial (merely a bitshift left or right), and shifting a piece vertical only marginally less so (a bitshift by `w * x` bits for `x` rows). Furthermore, vertical shifts *between* chunks have already been implemented in the form of [line clears](https://github.com/professor-l/tetris-bitboard/blob/main/docs/CLEARS.md), a function we will likely abstract to allow clearing from the base of any chunk, not just `B[0]`.

The basic idea behind it is that **rotating a piece within a matrix, particularly around a variable axis, is difficult and slow, but shifting a piece around the matrix is fast.** To take advantage of this, we predefine each of the four rotations of all pieces as they would appear in the top left of a chunk, and define rules for shifting those pieces over and down such that we can simulate a rotation in place.

## Padding and limits

In practice, there are a few things to keep track of here, so each rotation (called the "rotation index" of each piece comes with constants called "padding" and "limit" constants - one of each for each axis, horizontal and vertical. It works like this:

```
0 1 0   0 0 0
0 1 0   1 1 1
0 1 1   1 0 0
```

Above are two of the four possible rotation indecies of the L piece. We imagine those pieces, presented there, pressed up against a top left "wall" on the matrix. As you can see, in the first rotation, there exists a sort of "gap" between the piece and the left wall, and a similar gap exists on the top in the second rotation. This is where the padding comes in.

Both of those pieces are, in that state, in what I call their "rotatable positions." They can be maneuvered, but there may be gaps on one side to allow room for rotations. We pair rotatable positions with **padding** values - in thise case the first example would have horizontal padding of 1 and vertical padding of 0, and the second example would have horizontal padding of 0 and vertical padding of 1. We define padding only for the top and left - values describing movement to the bottom or right of a chunk are known as "limits."

The rotatable position of a particular piece/rotation index differ from that pieces "true position" by a number of bits equal to the padding on each axis. The true position of a piece is that piece as it would exist pressed fully against both walls, with no gaps. To move a piece from its "true position" to its "rotatable position", we shift right by the horizontal padding, and down by the vertical padding.

The "limit" values, on the other hand, are the **maximum possible shift** on each axis while staying within the chunk. The focus here is on the horizontal limits, since vertical limits are usually ignored due to chunk cascading, but the latter is also useful when the playfield is empty or near-empty, and they are defined similarly.

Here, limits are defined as **the maximum number of cells a piece can shift from its intial rotatable position before it hits a wall**. Note that they are defined in relation to rotatable positions, not true positions - this is done so that rotations in place can occur by simply substituting the active piece for a *new* one in the desired rotation index, shifted over and down. That shifting can be done with values equivalent to the shifting of the previous rotation index, since all shifts are defined just like the limits are, with respect to rotatable rather than true position.

These specifications allow us to do the following:

- To place a piece somewhere, we shift left and down the specified number of cells, adding padding as necessary. We then keep track of the horizontal shift `h`, and the vertical shift `v`.
- To rotate a piece, we spawn in the piece with its *new* rotation index, then apply the same shifts `h` and `v`, adjusted with the padding of the new rotation index rather than the old one.
- To check for permissibility, we need only check that `h` is less than the rotation index's horizontal limit, and that `v` is either less than the vertical limit, or less than 6 if chunk cascading is allowed (i.e. if we're not at the bottom of the playfield)

For a concrete example of padding and limits in action, see [PADS_SHIFTS.md](https://github.com/professor-l/tetris-bitboard/blob/main/docs/pieces/PADS_SHIFTS.md).
For a complete SRS padding/limit table, see [tables/SRS.md](https://github.com/professor-l/tetris-bitboard/blob/main/docs/pieces/tables/SRS/md)
