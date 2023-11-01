#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <cstdio>
#include <stdio.h>

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
void readCubeIntoBuffer(int px, int py, int pz, int bx, int by, int bz, int ox, int oy, int oz, char ***cube_buffer, int start)
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
	int byte_offset = (oy * (bx + 1)) + ox + ((by * (bx + 1)) + 1) * oz;
	fseek(stdin, byte_offset, SEEK_CUR);

	for (int z = 0; z < pz; z++)
	{
		for (int y = 0; y < py; y++)
		{
			fgets(cube_buffer[z][y], px + 1, stdin);
			if (y != py - 1)
				fseek(stdin, bx - px + 1, SEEK_CUR);
		}
		fseek(stdin, (by - py + 1) * (bx + 1) + (1 - px), SEEK_CUR);
	}
}

// Compression and printing done here - currently does no compression
bool compressAndPrint(int px, int py, int pz, int bx, int by, int bz, int &ox, int &oy, int &oz, char ***cube_buffer, std::unordered_map<char, std::string> &tags)
{
	/*
Function Name: compressAndPrint
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
	tags: map of input characters/tags with their original descriptions
Function Description:
	compress and prints the values of cube buffer in respective output
Function Return:
	boolean of whether compression is successfull or not
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
		return false;
	}

	return true;
}

int main()
{

	// Read and store header information
	int bx, by, bz;
	int px, py, pz;
	char nada;
	std::cin >> bx >> nada >> by >> nada >> bz >> nada >> px >> nada >> py >> nada >> pz >> std::ws;

	// uncomment next 2 lines to run diff testing to check if input is parsed correctly
	// std::cout << bx << " " << by << " " << bz << " " << px << " " << py << " " << pz;
	// return 0;

	std::string line;
	char symbol;
	std::string label;
	std::unordered_map<char, std::string> tags;
	while (true)
	{
		getline(std::cin, line);
		// need \"r" line or it doesn't work properly
		if (line.empty() || line == "\r")
		{
			break;
		}
		std::stringstream line_stream(line);
		line_stream >> symbol >> nada >> label;
		tags[symbol] = label;
	}

	// uncomment next 4 lines to text tag table
	// for(auto tag : tags) {
	// 	std::cout << tag.first << " " << tag.second << std::endl;
	// }
	// return 0;

	char ***cube_buffer = allocateCubeBuffer(px, py, pz);

	int start = ftell(stdin);

	int ox = 0;
	int oy = 0;
	int oz = 0;

	while (true)
	{
		// get the input to buffer
		readCubeIntoBuffer(px, py, pz, bx, by, bz, ox, oy, oz, cube_buffer, start);

		// compress the input of the buffer
		if (!compressAndPrint(px, py, pz, bx, by, bz, ox, oy, oz, cube_buffer, tags))
			break;
	}
	// Free up cube buffer
	freeCubeBuffer(cube_buffer, py, pz);
	return 0;
}