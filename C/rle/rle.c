#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#if  __WIN32__
	#define ENDLINE 2
#else
	#define ENDLINE 1
#endif

typedef struct
{
    // char tag;
    char label[50];
	uint8_t label_size;
} Tag;

/**
 * @name iToS
 * @param num is the number to convert to a string
 * @details The function converts an integer to it's string representation.
 * @return puts the result in a shared buffer num_buf, with it's corresponding
 * size (length of the string representation of the number) in the num_sz
 * variable. This is done so that the function doesn't have to return anything and
 * so that these variables do not have to be allocated at run time, to improve speed
 * _num_sz and tmp are private variables used in iToS.
 * @attention Expects an unsigned integer of at most 32 bits
 * 
 */
char num_buf[5];
uint8_t num_sz;
void iToS(uint32_t num) {
	static uint8_t _num_sz = 0;
	static char tmp;

	// Edge case, number is zero
	if (num == 0) {
		num_buf[0] = '0';
		num_sz = 1;
		return;
	}

	// Convert the number to a string, store it in the buffer
	// Because of the way this is done, it will be put 
	// into the char buffer backwards
	while ( num ) {
		num_buf[_num_sz] = (char) ( (num % 10) + 48 );
		num /= 10;
		_num_sz++;
	}

	num_sz = _num_sz;
	_num_sz--;

	// reverse the buffer
	while ( _num_sz ) {
		tmp = num_buf[_num_sz];
		num_buf[_num_sz] = num_buf[num_sz - 1 - _num_sz];
		num_buf[num_sz - 1 - _num_sz] = tmp;
		_num_sz--;
	}

	return;
}

/* Algorithm description */
/* 
		- Compresses a parent block at a time
		- Moves left to right, then down one: (where x is the parent block)
		x0 -> 0x -> 00 -> 00
		00    00	x0	  0x
		- code for z commented out, waiting for implementation in parseBlock()
		- For every y, starts at an x value, saves it in RLE_base_x, saves the character
		at that position in curr_char and then iterates until the tag is different 
		or until we reach the end of the parent block.
		Whether we are at the end of the parent block or not is 
		determined by whether itr_x < parent_x.
		- Once block is finished, reset itr_x and itr_y and update x_base and y_base
		to point to the new parent block. If the block is out of bounds, finish
*/
int main(void)
{
	//Read and store header information
	uint16_t bx, by, bz;
	uint16_t px, py, pz;
	char nada[ENDLINE]; // bin variable

	Tag *tag_table;

	uint8_t tmp_label_size_count = 0;

	// Read and Parse First Line
    fscanf(stdin, "%hu,%hu,%hu,%hu,%hu,%hu\n", &bx, &by, &bz, &px, &py, &pz);

    // Get number of tags, done by counting the number of lines until the length of the line is less than 2,
	// in practice, this iterates until the newline at the end of the tag table
	int metadata_curr_tag;

    tag_table = (Tag *) malloc(128 * sizeof(Tag));
	metadata_curr_tag = getc(stdin);
    // Read and parse tags
    while (metadata_curr_tag != '\n')
    {
		scanf(", ");
		fgets(tag_table[metadata_curr_tag].label, 50, stdin);

		tag_table[metadata_curr_tag].label_size = 0;

		while (tag_table[metadata_curr_tag].label[tmp_label_size_count] != '\n') {
			tmp_label_size_count++;
		}
		tag_table[metadata_curr_tag].label_size = tmp_label_size_count;
		tmp_label_size_count = 0;

		metadata_curr_tag = getc(stdin);
    }

	/* RLE start */

	uint16_t ox = 0;
	uint16_t oy = 0;
	uint16_t oz = 0;

	char 			*curr_x_line = (char*) malloc(sizeof(char) * px); /* the current line of xs in the current parent block */

	uint16_t 			itr_x = 0; /* x iterator for inside the current block */

	char 			curr_tag; /* tag that the algorithm is currently compressing*/

	uint16_t 			RLE_base_x = 0; /* the x coordinate that the compressed 
										region starts at in the current block */

	fgets(curr_x_line, px + 1, stdin); // gets the first line of xs

	while (1) // while the block base is in bounds for the x-value
	{
		curr_tag = curr_x_line[itr_x];  // save the compressed tag
		RLE_base_x = itr_x;				// sets the RLE_base to the current position in the x line
		itr_x++;

		// iterate until the tag is different or we are at the end of the parent block
		while ( ( itr_x < px ) && ( curr_x_line[itr_x] == curr_tag ) )
		{
			itr_x++;
		}

		// output in specific format, being:
		// pos x, pos y, pos z, size x, size y, size z, label
		iToS(ox + RLE_base_x);
		fwrite(num_buf, 1, num_sz, stdout);
		putc(',', stdout);

		iToS(oy);
		fwrite(num_buf, 1, num_sz, stdout);
		putc(',', stdout);

		iToS(oz);
		fwrite(num_buf, 1, num_sz, stdout);
		putc(',', stdout);

		iToS(itr_x - RLE_base_x);
		fwrite(num_buf, 1, num_sz, stdout);
		putc(',', stdout);

		putc('1', stdout);
		putc(',', stdout);
		putc('1', stdout);
		putc(',', stdout);

		fwrite(tag_table[(uint8_t) curr_tag].label, 1, tag_table[(uint8_t) curr_tag].label_size, stdout);
		putc('\n', stdout);
		// The above code does what the below (commented) code does, but faster

		// printf("%hu,%hu,%hu,%hu,%hu,%hu,%s\n", 
		// 		ox + RLE_base_x,
		// 		oy, 
		// 		oz, 
		// 		itr_x - RLE_base_x, 
		// 		1, 
		// 		1, 
		// 		findLabelForTag(tags, number_of_tags, curr_tag));

		// iterates to the next parent block
		if (itr_x >= px) // if we are at the end of a line in the parent block
		{
			ox += px;
			itr_x = 0;

			if (ox >= bx) // if we are at the of a line in the whole map switch to next y
			{
				ox = 0;
				oy++;

				fgets(nada, ENDLINE, stdin); // ignore the \n\r

				if ( oy >= by ) // if we are at the bottom (highest y), switch to the next z layer
				{
					oy = 0;
					ox = 0;
					oz++;

					fgets(nada, ENDLINE, stdin); // ignore the \n\r

					if (oz >= bz) // if we are at the deepest (highest z), finish
					{
						break;
					}				
				}
			}

			fgets(curr_x_line, px + 1, stdin); // read new parent block x line
		}
	}
}