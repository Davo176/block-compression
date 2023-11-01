#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "IO.h"

uint16_t mx, my, mz; // map x, y and z
uint8_t px, pz; // parent x and z

Tag *tags; /* Tags and their corresponding labels*/

uint16_t number_of_tags = 0;

uint16_t buf_y_sz; /* how many lines each thread should buffer*/

void parseMetadata()
{
	//Read and store header information
	uint16_t int_nada; /* bin variable */

	// Read and Parse First Line
    fscanf(stdin, "%hu,%hu,%hu,%hhu,%hu,%hhu", &mx, &my, &mz, &px, &int_nada, &pz);

	// fgets(nada, ENDLINE, stdin);
	scanf("\n");

    // Get number of tags, done by counting the number of lines until the length of the line is less than 2,
	// in practice, this iterates until the newline at the end of the tag table
    char buffer[100];

    while (fgets(buffer, sizeof(buffer), stdin) && strlen(buffer) > 2)
    {
        number_of_tags++;
    }

    fseek(stdin, 0, SEEK_SET);            // Move the file pointer back to the beginning of the file
    fgets(buffer, sizeof(buffer), stdin); // Skip first line, which is the dimensions of the model and the parent block size
    tags = (Tag *)malloc(number_of_tags * sizeof(Tag));
    // Read and parse tags
    for (int i = 0; i < number_of_tags; i++)
    {
        scanf("%c, %s", &tags[i].tag, tags[i].label);
		scanf("\n");
    }

	scanf("\n");

	// max of 100MB per buffer, or my / num_threads if it's smaller than 100MB
	if ((my * mx) <= (104857600 * 4)) { // note 104857600 is the number of bytes in 100MB
		buf_y_sz = ( my / 4 ) + 1; // add one so that any odd numbers of multiples mean that it will still get split relatively evenly between the threads, instead of having one thread have to come back and compress one or two lines after all other compresion for that parent z thickness is done
	}
	else {
		buf_y_sz = 104857600 / mx; // The amount of lines that fit into 100MB, add one 
	}

	return;
}

void output(const uint16_t ox, const uint16_t oy, const uint16_t oz, const uint16_t sx, const uint16_t sy, const uint16_t sz, char tag)
{
	const char *tag_label;

	for (uint16_t i = 0; i < number_of_tags; i++)
    {
        if (tags[i].tag == tag)
        {
            tag_label = tags[i].label; // Return the label for the matching tag
			break;
        }
    }

	printf("%hu,%hu,%hu,%hu,%hu,%hu,%s\n", 
				ox,
				oy, 
				oz, 
				sx, 
				sy, 
				sz, 
				tag_label);

	return;
}