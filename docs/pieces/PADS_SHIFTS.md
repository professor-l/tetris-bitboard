# Padding and Shifting

Because of the implementation of piece rotation, certain rotations will have x and/or y padding to allow them to exist on the far left wall, and rotations will have upper limits on horizontal and vertical shifting, with the latter being enforced only at the bottom of the playfield. This will work as follows:

```
L PIECE:

0 1 0 . . . . . . .
0 1 0 . . . . . . .
0 1 1 . . . . . . .
. . . . . . . . . .
. . . . . . . . . .
```

If we imagine the L piece existing as seen above (its "rotatable position"), we can see that it's padded on the left by 1 column, and on the top by 0 rows. In practice, it will be "spawned in" prior to any manipulation on the far left (its "true position"):

```
1 0 . . . . . . . .
1 0 . . . . . . . .
1 1 . . . . . . . .
```
However, this position will be given a horizontal index of -1, due to the rotation behavior of the L piece in SRS. In order to place it here, we would bitshift to the right by -1 bits (a 1 bit shift to the left), then by the number of bits equal to its horizontal padding (1). In this case, the -1 and 1 cancel out, so no shift takes place.

Here, we also establish a maximum horizontal and vertical shift for this L within a chunk. The maximum horizontal shift holds true for all chunks, but the vertical restriction holds true only for the first chunk - the one at the bottom of the playfield. Otherwise, overflow will just enter the chunk below the current one, as is intended behavior (chunk cascading). Each rotation index has its own maximum shifting, and the values are measured from the piece's rotatable position. The actual limit, in execution, is calculated by adding the shift limits to the piece's padding in that direction (and potentially multiplied by `w` in the case of vertical shifts).

In practice, each of these values must be determined manually based on the base rotation table of whatever system is being implemented (See [SRS](https://tetris.wiki/Super_Rotation_System) or [Right-handed NRS](https://tetris.wiki/Nintendo_Rotation_System#Right-handed). My full padding and limits tables for each rotation system are found in [docs/pieces/tables](https://github.com/professor-l/tetris-bitboard/tree/main/docs/pieces/tables).
