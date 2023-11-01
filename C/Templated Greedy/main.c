/* Algorithm: 2D Templated Greedy */
/* Description: This algorithm works in by compressing each sub-block in three phases 

It is intended as a stepping stone to 3D templated greedy, which is intended to be a higher compression 
alternative to greedy, and a potential add in to excessively greedy.
2D templated greedy was created largely with a specific test case in mind:
 ooo
ooo 

Regular Greedy will compress this block into 3 blocks, (1,0,0,2,2,1), (3,0,0,1,1,1), (0,1,0,1,1,1).

However, this is sub-optimal, it is possible to compress it into two blocks: (1,0,0,3,1,1), (0,1,0,3,1,1), 
and 2D Templated greedy does.

Templated greedy achieves this in three phases:
1st phase: Template creation
2nd phase: Template reduction
3rd phase: Template compression 

-- TEMPLATE CREATION --
In the template creation phase, a template is generated using a generic 2D greedy algorithm that alternatively 
checks if it can grow in the x and y direction.

For example:
noooo
noooo
nnonn

In this example the template creation algorithm will first check if it can grow in the x
direction, which it can, so the template is now (2,1,1). Note that the z axis is included although not used 
in this algorithm.

Then, it checks if it can grow in the y direction, which it can, because all of the characters
directly underneath the current template are also o's. This means the template is now (2,2,1)

Then, it checks if it can grow in the x direction again, which it can, because all of the characters to the
right of the current template are also o's. This means the template is now (3,2,1)

Then, it checks if the template can grow in the y direction, which at this point it can't, because there are 
n's below the current template. The template stays the same. Because it can't grow any more in the y direction,
at this point it set's the growth_vector.y to false. This informs the program that the template cannot grow
in the y direction any more, so don't check it.

The algorithm then checks if it can grow in the x axis again, and since it can, does. The template is now (4,2,1)

It skips the check of the y axis for potential growth because the growth_vector.y is set to false and checks the x 
axis again. It sees that the template cannot grow in the x axis anymore (because it's at the end of the block), and 
sets growth_vector.x to false.

Upon the next iteration, it notices that both the growth_vector.x and growth_vector.y are false, and returns the 
computed template.

-- TEMPLATE REDUCTION --
In this phase, templated greedy shrinks the template from the bottom if it recognises that it's 
compressing into another block that would be suboptimal. It then re-creates a template with 
a "barrier" so as to not compress into another block.

The main test case associated with the reason for doing this is:
 ooo
ooo 

If we just used the above template to compress this region without reduction, it would result
in compressing the above example into 3 sub blocks.

However, using template reduction, templated greedy notices that it would be compressing
into another parent block at y = 1, and thus creates a barrier at y = 1 and re-creates the 
template with a barrier at y = 1. This means the new template will be (3,1,1), and when
the algorithm reaches the next y line, it can create another template at (0,1,0), being
(3,1,1) again, thus compressing it into two parent blocks instead of three.

How templated greedy notices that it is compressing into another parent block is trivial.
If there is a character to the left of the last y iteration that is the same as the template
character, it is compressing into a parent block. Thus, to calculate the barrier, there is a 
while loop that iterates from the bottom of the template going up, until it reaches a point
in which the character to the left of the template is not the same.

This check is only done on the left becuase we iterate and grow the template to the right, 
and maybe (not proven), it is never sub-optimal to compress to the right as much as you can,
but the left is a different story, because as far as the template is concerned, it knows
nothing about it's left side.

Consider these two examples:

EX 1:
 o 
ooo

EX 2:
 o 
 oo

In both examples, the template will be initially created as (1,2,1), but in the first example
it is sub-optimal to compress this as it will cut the block underneath in half. However, in 
the second example it is not sub optimal because it's not cutting a block in half.

If there is nothing to the left of the template, it is impossible to cut into another block.

However, in this case:
 o
oo
 o

The initial template created would be (1,3,1), if we iterated up the template and found that 
at position 1 (0-indexed), there was a template character to the left, and stopped compression,
that would be allowing a smaller block (0,1,0,2,1,1) to compress into a larger block (1,0,0,1,3,1).
Thus, templated greedy only shrinks the template when there is a template character to the left of
the end of the template, because only then is it never sub-optimal (not proven) 
to make way for a more major template.

-- TEMPLATE COMPRESSION --
Once the template is created, all the characters corresponding to the current template are
set to '\0'. This is so that the algorithm doesn't try to re-compress them, before trying
to compress a region, it iterates the map until a character is non-'\0'.
After this, the template is printed, offset by the current global ox,oy and oz.
*/

/* 
--PARSING--
Parent blocks are read into seperate 'cubes'. To buffer a whole parent block thickness
at a time, all cubes in that thickness are read into a 'cube array'.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define NEW_LINE_WIDTH 1
#define LABEL_LENGTH 200
#define ASCII_COUNT 256

typedef struct {
	bool x;
	bool y;
	bool z;
} Vector; /* essentially a unit vector that can tell a direction */

typedef struct {
	unsigned int x;
	unsigned int y;
	unsigned int z;
} Block; /* block of size x, y, z */

// typedef struct {
// 	bool L,R;
// } WingSignature;

unsigned int bx, by, bz; /* map x, y and z*/
unsigned int px, py, pz; /* parent block x, y and z*/
unsigned int wx, wy, wz; /* width x, y, and z, how many parent blocks in the x/y/z dimension there are*/
unsigned int ox = 0; /* offset x/y/z - current global position in map */
unsigned int oy = 0;
unsigned int oz = 0;
unsigned int cube_ox = 0; /* offset x/y/z in current cube*/
unsigned int cube_oy = 0;
unsigned int cube_oz = 0;

char curr_char; /* current character the algorithm is compressing */
unsigned int curr_cube = 0; /* current cube we are in */

unsigned int cube_count; /* current cube number */

char ****cubes; /* all of the parent block cubes */

char **map; /* tag table map */

#ifdef PRINTCUBES
/* prints a cube to stdout */
void printCube(char ***cube) {
	for (unsigned int z = 0; z < pz; z++) {
		for (unsigned int y = 0; y < py; y++) {
			for (unsigned int x = 0; x < px; x++) {
				putc(cube[z][y][x], stdout);
			}
			putc('\n', stdout);
		}
		putc('\n', stdout);
	}

	return;
}
#endif

//Allocates a cube buffer
char ***allocateCubeBuffer()
{
    char ***cube_buffer = (char ***)malloc(pz * sizeof(char **));
    for (unsigned int z = 0; z < pz; z++) {
        cube_buffer[z] = (char **)malloc(py * sizeof(char *));
        for (unsigned int y = 0; y < py; y++) {
            cube_buffer[z][y] = (char *)malloc(px * sizeof(char));
        }
    }
    return cube_buffer;
}

//Allocates an aray of cube buffers
char ****allocateCubeArray() {
	char ****cubes = (char ****) malloc(sizeof(char ***) * cube_count);
	for(unsigned int i=0;i<cube_count;i++) {
		cubes[i] = allocateCubeBuffer();
	}
	return cubes;
}

//Skips n newline chars
void skipNewLine() {
	for(unsigned int s=0;s<NEW_LINE_WIDTH;s++) {
		fgetc(stdin);
	}
}

//Fills the array of cube buffers reading from stdin
void readIntoCubeArray() {
	for(unsigned int z=0;z<pz;z++) {
		for(unsigned int y=0;y<by;y++) {
			for(unsigned int x=0;x<wx;x++) {
				fgets(cubes[x + (y/py) * wx][z%pz][y%py], (int) px+1, stdin);
			}
			skipNewLine();
		}
		skipNewLine();
	}
}

//Allocates the tag table map
char **allocateMap() {
	char **map = (char **) malloc(sizeof(char *) * ASCII_COUNT);
	for(unsigned int i=0;i<ASCII_COUNT;i++) {
		map[i] = (char *) malloc(sizeof(char) * LABEL_LENGTH);
	}
	return map;
}

// Creates and reduces a template
Block createTemplate(unsigned int tx, unsigned int ty, unsigned int tz, unsigned int y_barrier) {
	Block block_template = { 1, 1, 1 };

	Vector growth_vector = { 1, 1, 0 };

	curr_char = cubes[curr_cube][tz][ty][tx];

	unsigned int tx_itr;
	unsigned int ty_itr;

	// grow initial template in a greedy fashion
	while (growth_vector.x || growth_vector.y ) {
		
		// if growing in the x direction has not previously been disallowed
		if (growth_vector.x) {
			ty_itr = 0;

			// if the template isn't at the boundary of a parent block
			if ( (tx + block_template.x) != px) {

				// check that all characters in the next x-layer are 
				// the same as the template character
				while ( ty_itr < block_template.y) {

					// if not, we can't grow in the x direction any more
					if ( cubes[curr_cube][tz][ty + ty_itr][tx + block_template.x] != curr_char) {
						growth_vector.x = false;
						break;
					}

					ty_itr++;
				}
			}
			else {
				growth_vector.x = false;
			}

			// increment the block template in the x direction
			// if the growth vector survived this x iteration
			if (growth_vector.x) {
				block_template.x++;
			}
		}

		// if growing in the y direction has not been disallowed
		if (growth_vector.y) {

			// If we have hit the y barrier, disallow growing in the y direction
			if ( ty + block_template.y == y_barrier ) {
				growth_vector.y = false;
			}
			else {

				// check if we are good to grow to another y level
				tx_itr = 0;

				// check if we are at the end of a parent block
				if ( (ty + block_template.y) != py ) {

					// check if we can move to the next y level
					while ( tx_itr < block_template.x ) {

						// check every x value in the next y level is the same as curr_char
						if (cubes[curr_cube][tz][ty + block_template.y][tx + tx_itr] != curr_char) {
							growth_vector.y = false;
							break;
						}

						tx_itr++;
					}
				}
				else {
					growth_vector.y = false;
				}

				// if the growth vector survived growing in the y direction, increment
				// the block template in the y direction
				if (growth_vector.y) {
					block_template.y++;
				}
			}

		}
	}

	// reduce template
	// if the template is not at the left most side of the cube
	if (tx != 0) {

		// if there is in fact a template character to the left of the end of 
		// the current cube
		if (cubes[curr_cube][tz][ty + block_template.y - 1][tx - 1] == curr_char) {

			// decrement block_template.y until there is no template character to the 
			// left of the current template
			while (cubes[curr_cube][tz][ty + block_template.y - 1][tx - 1] == curr_char) {
				block_template.y--;
			}

			// create new template
			// useful in this case:
			/* 
			 ooo
			ooo 
			The new template created will be (3,1,1) @ position (1,0,0)
			*/
			block_template = createTemplate(tx, ty, tz, ty + block_template.y);
		}
	}

	return block_template;
}

// compresses the template
void compressTemplate(unsigned int tx, unsigned int ty, unsigned int tz, Block block_template) {
	
	// set all corresponding map positions corresponding to the block template to "already compressed"
	for (unsigned int coy = ty; coy < ( block_template.y + ty ); coy++) {
		for (unsigned int cox = tx; cox < ( block_template.x + tx ); cox++) {
			cubes[curr_cube][tz][coy][cox] = '\0';
		}
	}

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wchar-subscripts"
	printf("%u,%u,%u,%u,%u,%u,%s\n", ox + tx, oy + ty, oz + tz, block_template.x, block_template.y, 1, map[curr_char]);
	#pragma GCC diagnostic pop

	return;
}

// Driver for creating template and compressing it
unsigned int compress(unsigned int cx, unsigned int cy, unsigned int cz, unsigned int y_barrier) {
	static Block block_template;

	block_template = createTemplate(cx, cy, cz, y_barrier);
	compressTemplate(cx, cy, cz, block_template);

	return block_template.x;
}

int main() {

	fscanf(stdin,"%u,%u,%u,%u,%u,%u\n",&bx,&by,&bz,&px,&py,&pz);
	char line[LABEL_LENGTH+3];
	char symbol;
	map = allocateMap();

	// scan in tag table
	while(1) {
		symbol = (char) getc(stdin);
		if (symbol == '\0' || symbol == '\n' || line[1] == '\r') break;
		fgets(line, LABEL_LENGTH+4, stdin);

		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wchar-subscripts"
		sscanf(line,", %s\n", map[symbol]);
		#pragma GCC diagnostic pop
	}

	wz = bz/pz;
	wx = bx/px;
	wy = by/py;
	cube_count = (bx/px) * (by/py);

	cubes = allocateCubeArray();

	//Main loop
	readIntoCubeArray();

	while (1) {

		// read new cubes into array if we have compressed all of them
		if (curr_cube == cube_count) {
			readIntoCubeArray();
			curr_cube = 0;
		}

		#ifdef PRINTCUBES
		printCube(cubes[curr_cube]);
		#endif

		// iterate over current cube
		while (cube_oz != pz) {
			while (cube_oy != py) {

				// skip over already compressed regions
				while ( ( cube_ox != px ) && cubes[curr_cube][cube_oz][cube_oy][cube_ox] == '\0') {
					cube_ox++;
				}

				// compress an x layer
				while (cube_ox != px) {
					cube_ox += compress(cube_ox, cube_oy, cube_oz, by);

					// skip over already compressed regions
					while ( ( cube_ox != px ) && cubes[curr_cube][cube_oz][cube_oy][cube_ox] == '\0') {
						cube_ox++;
					}
				}

				// move to the next y layer
				cube_ox = 0;
				cube_oy++;
			}

			// move to the next z layer	
			cube_oy = 0;
			cube_oz++;
		}

		// move to the next parent block
		ox += px;

		if (ox == bx) {
			ox = 0;
			oy += py;

			if (oy == by) {
				oy = 0;
				oz += pz;

				if (oz == bz) {
					break;
				}
			}
		}

		cube_ox = 0;
		cube_oy = 0;
		cube_oz = 0;

		curr_cube++;
	}

	return 0;
}
