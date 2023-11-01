#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <stdio.h>

typedef unsigned short int uint16_t;

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
	int bx, by, bz;
	int px, py, pz;
	char nada;
	std::cin >> bx >> nada >> by >> nada >> bz >> nada >> px >> nada >> py >> nada >> pz >> std::ws;
	std::string line;
	char symbol;
	std::string label; 
	std::unordered_map<char, std::string> tags;

	while(true) {
		getline(std::cin, line);
		if(line.empty() || line == "\r") break;
		std::stringstream line_stream(line);
		line_stream >> symbol >> nada >> label;
		tags[symbol] = label;
	}

	/* RLE start */

	int ox = 0;
	int oy = 0;
	int oz = 0;

	char *curr_x_line = new char[px];

	uint16 			itr_x = 0; /* x iterator for inside the current block */
	// uint16 			itr_y = 0; /* y iterator for inside the current block */
	// uint16 itr_z = 0;

	// int 			x_base = 0; /* current base x of the parent block */
	// int				y_base = 0; /* current base y of the parent block */
	// uint16 z_base = 0;

	char 			curr_tag; /* tag that the algorithm is currently compressing*/

	uint16_t 			RLE_base_x = 0; /* the x coordinate that the compressed 
										region starts at in the current block */

	// while (z_base < z_count)
	// {
		// while (y_base < by) // while the block base is in bounds for the y-value
		// {
			fgets(curr_x_line, px + 1, stdin);

			while (true) // while the block base is in bounds for the x-value
			{
				curr_tag = curr_x_line[itr_x]; // save the compressed tag
				RLE_base_x = itr_x;
				itr_x++;

				// iterate until the tag is different or we are at the end of the parent block
				while ( ( itr_x < px ) && ( curr_x_line[itr_x] == curr_tag ) )
				{
					itr_x++;
				}
        
				// output in specific format, being:
				// pos x, pos y, pos z, size x, size y, size z, label
				std::cout << ox + RLE_base_x << "," << oy << "," << oz << "," << itr_x - RLE_base_x << ",1,1," << tags[curr_tag] << "\n";

        // If we are at the end of a parent block (in the x direction), 
        // reset the x iterator and move to the next y line
				if (itr_x >= px)
				{
					ox += px;
					itr_x = 0;

					if (ox >= bx)
					{
						ox = 0;
						oy++;

						std::cin.ignore(1);
						
						if ( oy >= by )
						{
							oy = 0;
							ox = 0;
							oz++;

							std::cin.ignore(1);

							if (oz >= bz)
							{
								break;
							}

						// itr_z++;					
						}
					}

					fgets(curr_x_line, px + 1, stdin);
				}
				// printf("%i,%i,%i,%i,%i,%i,%s\n", 
				// 		ox + RLE_base_x, 
				// 		oy, 
				// 		0, 
				// 		itr_x - RLE_base_x, 
				// 		1, 
				// 		1, 
				// 		tags.at(curr_tag));

				// If we are at the end of a parent block (in the x direction), 
				// reset the x iterator and move to the next y line
				// if ( ox >= bx ) 
				// {
				// 	ox = 0;

				// 	oy++;
				// }

				// // If we are at the end of a parent block in the y direction,
				// // reset both the x and y iterators and break out of while loop,
				// // which will move on to the next parent block (or terminate)
				// if ( oy >= by )
				// {
				// 	oy = 0;
				// 	ox = 0;
				// 	oz++;

				// 	std::cin.ignore(1);

				// 	// itr_z++;					
				// }

				// if (oz >= bz)
				// {
				// 	break;
				// }

				// if ( itr_z >= parent_z )
				// {
				// 	itr_x = 0;
				// 	itr_y = 0;
				// 	itr_z = 0;

				// 	break;
				// }


				/* 
				set RLE_base_x to the itr_x value that has not yet been
				checked by itr_x. Note that itr_x lands on the element after 
				its last checked position.
					*/
				// RLE_base_x = itr_x;

				// x_base += parent_x; // Move to the parent block one to the right
			}

			// Move to the next row of parent blocks, starting at the left
			// x_base = 0;

			// y_base += parent_y;
		// }

		// x_base = 0;
		// y_base = 0;

		// z_base += parent_z;
	// }
}