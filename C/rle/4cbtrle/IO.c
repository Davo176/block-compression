#include "IO.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

uint16_t mx, my, mz; // map x, y and z
uint8_t px, pz; // parent x, y and z

Tag *tags;

uint16_t number_of_tags = 0;

// function used to get the metadata about the current map
// puts all of the information in shared variables defined in IO.h
void parseMetadata()
{
	//Read and store header information
	uint16_t int_nada;

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

	return;
}

void output(const uint16_t ox, const uint16_t oy, const uint16_t oz, const uint16_t sx, const uint16_t sy, const uint16_t sz, char tag)
{
	const char *tag_label;

	for (uint16_t i = 0; i < number_of_tags; i++)
    {
		// printf("Comparing inputted tag %c with tag %c\n", tag, tags[i].tag);
        if (tags[i].tag == tag)
        {
			// printf("Thread %lld found label %s for tag %c\n", pthread_self(), tags[i].label, tag);
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