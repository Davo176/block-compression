#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <cstdio>
#include <stdio.h>

#define NEW_LINE_WIDTH 1

#if defined(_WIN32)
#define NEW_LINE_WIDTH 2
#endif

// Allocates a cube buffer
char ***allocateCubeBuffer(int px, int py, int pz)
{
	/*
Function Name: allocateCubeBuffer
Function Arguments:
	px: parent size in x-axis
	py: parent size in y-axis
	pz: parent size in z-axis
Function Description:
	this function allocates the memory to cubebuffer
Function Return:
	char***: return a char buffer of px,py,pz size
*/
	char ***cube_buffer = (char ***)malloc(pz * sizeof(char **)); // Allocate z_count z levels
	for (int z = 0; z < pz; z++)
	{
		cube_buffer[z] = (char **)malloc(py * sizeof(char *)); // For each z level allocate y_count y levels
		for (int y = 0; y < py; y++)
		{
			cube_buffer[z][y] = (char *)malloc(px * sizeof(char)); // For each y level allocate x_count x levels
		}
	}
	return cube_buffer;
}

// Free a cube buffer
void freeCubeBuffer(char ***cube_buffer, int py, int pz)
{
	/*
Function Name: freeCubeBuffer
Function Arguments:
	cube_buffer: buffer used to store block information
	py: parent size in y-axis
	pz: parent size in z-axis
Function Description:
	This function free up the memory.
Function Return:
	N/A
*/
	for (int z = 0; z < pz; z++)
	{
		for (int y = 0; y < py; y++)
		{
			free(cube_buffer[z][y]);
		}
		free(cube_buffer[z]);
	}
	free(cube_buffer);
}

// Reads a cube from the file into a cube buffer
inline void readCubeIntoBuffer(const int &px, const int &py, const int &pz, const int &bx, const int &by, const int &bz, const int &ox, const int &oy, const int &oz, char ***cube_buffer, const int &start)
{
	/*
Function Name: readCubeIntoBuffer
Function Arguments:
	px: parent size in x-axis
	py: parent size in y-axis
	pz: parent size in z-axis
	bx: block size in x-axis
	by: block size in y-axis
	bz: block size in z-axis
	ox: position coordinated of parent block in x-axis
	oy: position coordinated of parent block in y-axis
	oz: position coordinated of parent block in z-axis
	cube_buffer: buffer used for the storage
	start: starting position of input data in memory
Function Description:
	This reads values from std input and assigns into respective location of cube buffer.
Function Return:
	N/A
*/
	fseek(stdin, start, SEEK_SET);
	int byte_offset = (oy * (bx + NEW_LINE_WIDTH)) + ox + ((by * (bx + NEW_LINE_WIDTH)) + NEW_LINE_WIDTH) * oz;
	fseek(stdin, byte_offset, SEEK_CUR);

	for (int z = 0; z < pz; z++)
	{
		for (int y = 0; y < py; y++)
		{
			fgets(cube_buffer[z][y], px + 1, stdin);
			if (y != py - 1)
				fseek(stdin, bx - px + NEW_LINE_WIDTH, SEEK_CUR);
		}
		fseek(stdin, (by - py + 1) * (bx + NEW_LINE_WIDTH) - px + NEW_LINE_WIDTH, SEEK_CUR);
	}
}

// Prints a cube buffer
inline void printCubeBuffer(const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***cube_buffer, std::unordered_map<char, std::string> &tags)
{
	/*
Function Name: printCubeBuffer
Function Arguments:
	px: parent size in x-axis
	py: parent size in y-axis
	pz: parent size in z-axis
	ox: position coordinated of parent block in x-axis
	oy: position coordinated of parent block in y-axis
	oz: position coordinated of parent block in z-axis
	cube_buffer: buffer used for the storage
	tags: map of input characters/tags with their original descriptions
Function Description:
	this prints the values of cube buffer in respective output
Function Return:
	N/A
*/
	for (int z = 0; z < pz; z++)
	{
		for (int y = 0; y < py; y++)
		{
			for (int x = 0; x < px; x++)
			{
				std::cout << x + ox << "," << y + oy << "," << z + oz << "," << 1 << "," << 1 << "," << 1 << "," << tags[cube_buffer[z][y][x]] << std::endl;
			}
		}
	}
}

int main()
{
	// Read and store header information
	int bx, by, bz;
	int px, py, pz;
	char nada;
	std::cin >> bx >> nada >> by >> nada >> bz >> nada >> px >> nada >> py >> nada >> pz >> std::ws;
	std::string line;
	char symbol;
	std::string label;
	std::unordered_map<char, std::string> tags;
	while (true)
	{
		getline(std::cin, line);
		if (line.empty() || line == "\r")
			break;
		std::stringstream line_stream(line);
		line_stream >> symbol >> nada >> label;
		tags[symbol] = label;
	}

	char ***cube_buffer = allocateCubeBuffer(px, py, pz);

	int start = ftell(stdin);

	int ox = 0;
	int oy = 0;
	int oz = 0;

	while (true)
	{
		readCubeIntoBuffer(px, py, pz, bx, by, bz, ox, oy, oz, cube_buffer, start);

		/*
		INSERT COMPRESSION HERE
		*/

		printCubeBuffer(px, py, pz, ox, oy, oz, cube_buffer, tags);

		// Increment offset values
		ox += px;
		if (ox == bx)
		{
			ox = 0;
			oy += py;
		}

		if (oy == by)
		{
			oy = 0;
			oz += pz;
		}

		if (oz == bz)
		{
			break;
		}
	}

	freeCubeBuffer(cube_buffer, py, pz);
	return 0;
}
