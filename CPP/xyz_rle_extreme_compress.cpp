#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <thread>
#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdio.h>
// this implementation contains all 48 different variations of XYZ RLE
#define NEW_LINE_WIDTH 1

#if defined(_WIN32)
#define NEW_LINE_WIDTH 2
#endif

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

// run length encoding
inline void xyRleCompress(const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***cube_buffer, std::unordered_map<char, std::string> &tags)
{

	/*
	Function Name: xyRleCompress
	Function Arguments:
		px: parent size in x-axis
		py: parent size in y-axis
		pz: parent size in z-axis
		ox: position coordinated of parent block in x-axis
		oy: position coordinated of parent block in y-axis
		oz: position coordinated of parent block in z-axis
		c_buffer: buffer used for the storage
		tags: map of input characters/tags with their original descriptions
	Function Description:
		performs 2-Dimensional Run length Encoding
	Function Return:
		N/A
	*/
	char current;
	int rle_x, rle_y, rle_z;
	for (int z = 0; z < pz; ++z)
	{
		for (int y = 0; y < py; ++y)
		{
			for (int x = 0; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;

					while ((x < px) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						x++;
					}
					y++;
					// check if y compression is possible
					while ((y < py))
					{
						int value = 1;
						for (int i = rle_x; i < x; ++i)
						{
							if (cube_buffer[z][y][i] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_x; i < x; ++i)
							{
								cube_buffer[z][y][i] = '\0';
							}
							y++;
						}
					}
					std::cout << rle_x + ox << "," << rle_y + oy << "," << z + oz << "," << x - rle_x << "," << y - rle_y << "," << 1 << "," << tags[current] << "\n";
					if (x < px)
					{
						x--;
					}
					y = rle_y;
				}
			}
		}
	}
}

inline void copyBuffer(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, char ***cube_buffer, char ***dest_buffer)
{
	for (int z = start_x; z < pz; ++z)
	{
		for (int y = start_y; y < py; ++y)
		{
			for (int x = start_z; x < px; ++x)
			{
				dest_buffer[z][y][x] = cube_buffer[z][y][x];
			}
		}
	}
}

// run length encoding
// start x check y check z
inline int xyzRleCompress_000(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(start_x, start_y, start_z, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = start_x; z < pz; ++z)
	{
		for (int y = start_y; y < py; ++y)
		{
			for (int x = start_z; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					x++;
					while ((x < px) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						x++;
					}

					y++;
					// check if y compression is possible
					while ((y < py))
					{
						int value = 1;
						for (int i = rle_x; i < x; ++i)
						{
							if (cube_buffer[z][y][i] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_x; i < x; ++i)
							{
								cube_buffer[z][y][i] = '\0';
							}
							y++;
						}
					}

					z++;
					// check if z compression is possible
					while ((z < pz))
					{
						int value = 1;
						for (int j = rle_y; j < y; ++j)
						{
							for (int i = rle_x; i < x; ++i)
							{
								if (cube_buffer[z][j][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j < y; ++j)
							{
								for (int i = rle_x; i < x; ++i)
								{
									cube_buffer[z][j][i] = '\0';
								}
							}
							z++;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << rle_y + oy << "," << rle_z + oz << "," << x - rle_x << "," << y - rle_y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start x check y check z
inline int xyzRleCompress_x00(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = 0; z < pz; ++z)
	{
		for (int y = 0; y < py; ++y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					x--;
					while ((x >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						x--;
					}

					y++;
					// check if y compression is possible
					while ((y < py))
					{
						int value = 1;
						for (int i = rle_x; i > x; --i)
						{
							if (cube_buffer[z][y][i] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_x; i > x; --i)
							{
								cube_buffer[z][y][i] = '\0';
							}
							y++;
						}
					}

					z++;
					// check if z compression is possible
					while ((z < pz))
					{
						int value = 1;
						for (int j = rle_y; j < y; ++j)
						{
							for (int i = rle_x; i > x; --i)
							{
								if (cube_buffer[z][j][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j < y; ++j)
							{
								for (int i = rle_x; i > x; --i)
								{
									cube_buffer[z][j][i] = '\0';
								}
							}
							z++;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << rle_y + oy << "," << rle_z + oz << "," << rle_x - x << "," << y - rle_y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start x check y check z
inline int xyzRleCompress_0y0(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = 0; z < pz; ++z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = 0; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					x++;
					while ((x < px) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						x++;
					}

					y--;
					// check if y compression is possible
					while ((y >= 0))
					{
						int value = 1;
						for (int i = rle_x; i < x; ++i)
						{
							if (cube_buffer[z][y][i] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_x; i < x; ++i)
							{
								cube_buffer[z][y][i] = '\0';
							}
							y--;
						}
					}

					z++;
					// check if z compression is possible
					while ((z < pz))
					{
						int value = 1;
						for (int j = rle_y; j > y; --j)
						{
							for (int i = rle_x; i < x; ++i)
							{
								if (cube_buffer[z][j][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j > y; --j)
							{
								for (int i = rle_x; i < x; ++i)
								{
									cube_buffer[z][j][i] = '\0';
								}
							}
							z++;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << y + 1 + oy << "," << rle_z + oz << "," << x - rle_x << "," << rle_y - y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start x check y check z
inline int xyzRleCompress_xy0(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = 0; z < pz; ++z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					x--;
					while ((x >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						x--;
					}

					y--;
					// check if y compression is possible
					while ((y >= 0))
					{
						int value = 1;
						for (int i = rle_x; i > x; --i)
						{
							if (cube_buffer[z][y][i] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_x; i > x; --i)
							{
								cube_buffer[z][y][i] = '\0';
							}
							y--;
						}
					}

					z++;
					// check if z compression is possible
					while ((z < pz))
					{
						int value = 1;
						for (int j = rle_y; j > y; --j)
						{
							for (int i = rle_x; i > x; --i)
							{
								if (cube_buffer[z][j][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j > y; --j)
							{
								for (int i = rle_x; i > x; --i)
								{
									cube_buffer[z][j][i] = '\0';
								}
							}
							z++;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << y + 1 + oy << "," << rle_z + oz << "," << rle_x - x << "," << rle_y - y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start x check y check z
inline int xyzRleCompress_00z(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = 0; y < py; ++y)
		{
			for (int x = 0; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					x++;
					while ((x < px) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						x++;
					}

					y++;
					// check if y compression is possible
					while ((y < py))
					{
						int value = 1;
						for (int i = rle_x; i < x; ++i)
						{
							if (cube_buffer[z][y][i] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_x; i < x; ++i)
							{
								cube_buffer[z][y][i] = '\0';
							}
							y++;
						}
					}

					z--;
					// check if z compression is possible
					while ((z >= 0))
					{
						int value = 1;
						for (int j = rle_y; j < y; ++j)
						{
							for (int i = rle_x; i < x; ++i)
							{
								if (cube_buffer[z][j][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j < y; ++j)
							{
								for (int i = rle_x; i < x; ++i)
								{
									cube_buffer[z][j][i] = '\0';
								}
							}
							z--;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << rle_y + oy << "," << z + 1 + oz << "," << x - rle_x << "," << y - rle_y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start x check y check z
inline int xyzRleCompress_x0z(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = 0; y < py; ++y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					x--;
					while ((x >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						x--;
					}

					y++;
					// check if y compression is possible
					while ((y < py))
					{
						int value = 1;
						for (int i = rle_x; i > x; --i)
						{
							if (cube_buffer[z][y][i] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_x; i > x; --i)
							{
								cube_buffer[z][y][i] = '\0';
							}
							y++;
						}
					}

					z--;
					// check if z compression is possible
					while ((z >= 0))
					{
						int value = 1;
						for (int j = rle_y; j < y; ++j)
						{
							for (int i = rle_x; i > x; --i)
							{
								if (cube_buffer[z][j][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j < y; ++j)
							{
								for (int i = rle_x; i > x; --i)
								{
									cube_buffer[z][j][i] = '\0';
								}
							}
							z--;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << rle_y + oy << "," << z + 1 + oz << "," << rle_x - x << "," << y - rle_y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start x check y check z
inline int xyzRleCompress_0yz(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = 0; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					x++;
					while ((x < px) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						x++;
					}

					y--;
					// check if y compression is possible
					while ((y >= 0))
					{
						int value = 1;
						for (int i = rle_x; i < x; ++i)
						{
							if (cube_buffer[z][y][i] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_x; i < x; ++i)
							{
								cube_buffer[z][y][i] = '\0';
							}
							y--;
						}
					}

					z--;
					// check if z compression is possible
					while ((z >= 0))
					{
						int value = 1;
						for (int j = rle_y; j > y; --j)
						{
							for (int i = rle_x; i < x; ++i)
							{
								if (cube_buffer[z][j][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j > y; --j)
							{
								for (int i = rle_x; i < x; ++i)
								{
									cube_buffer[z][j][i] = '\0';
								}
							}
							z--;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << y + 1 + oy << "," << z + 1 + oz << "," << x - rle_x << "," << rle_y - y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start x check y check z
inline int xyzRleCompress_xyz(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					x--;
					while ((x >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						x--;
					}

					y--;
					// check if y compression is possible
					while ((y >= 0))
					{
						int value = 1;
						for (int i = rle_x; i > x; --i)
						{
							if (cube_buffer[z][y][i] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_x; i > x; --i)
							{
								cube_buffer[z][y][i] = '\0';
							}
							y--;
						}
					}

					z--;
					// check if z compression is possible
					while ((z >= 0))
					{
						int value = 1;
						for (int j = rle_y; j > y; --j)
						{
							for (int i = rle_x; i > x; --i)
							{
								if (cube_buffer[z][j][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j > y; --j)
							{
								for (int i = rle_x; i > x; --i)
								{
									cube_buffer[z][j][i] = '\0';
								}
							}
							z--;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << y + 1 + oy << "," << z + 1 + oz << "," << rle_x - x << "," << rle_y - y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start x check z check y
inline int xzyRleCompress_000(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(start_x, start_y, start_z, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = start_x; z < pz; ++z)
	{
		for (int y = start_y; y < py; ++y)
		{
			for (int x = start_z; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					x++;
					while ((x < px) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						x++;
					}

					z++;
					// check if y compression is possible
					while ((z < pz))
					{
						int value = 1;
						for (int i = rle_x; i < x; ++i)
						{
							if (cube_buffer[z][y][i] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_x; i < x; ++i)
							{
								cube_buffer[z][y][i] = '\0';
							}
							z++;
						}
					}

					y++;
					// check if z compression is possible
					while ((y < py))
					{
						int value = 1;
						for (int j = rle_z; j < z; ++j)
						{
							for (int i = rle_x; i < x; ++i)
							{
								if (cube_buffer[j][y][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j < z; ++j)
							{
								for (int i = rle_x; i < x; ++i)
								{
									cube_buffer[j][y][i] = '\0';
								}
							}
							y++;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << rle_y + oy << "," << rle_z + oz << "," << x - rle_x << "," << y - rle_y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start x check y check z
inline int xzyRleCompress_0y0(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = 0; z < pz; ++z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = 0; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					x++;
					while ((x < px) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						x++;
					}

					z++;
					// check if y compression is possible
					while ((z < pz))
					{
						int value = 1;
						for (int i = rle_x; i < x; ++i)
						{
							if (cube_buffer[z][y][i] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_x; i < x; ++i)
							{
								cube_buffer[z][y][i] = '\0';
							}
							z++;
						}
					}

					y--;
					// check if z compression is possible
					while ((y >= 0))
					{
						int value = 1;
						for (int j = rle_z; j < z; ++j)
						{
							for (int i = rle_x; i < x; ++i)
							{
								if (cube_buffer[j][y][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j < z; ++j)
							{
								for (int i = rle_x; i < x; ++i)
								{
									cube_buffer[j][y][i] = '\0';
								}
							}
							y--;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << y + 1 + oy << "," << rle_z + oz << "," << x - rle_x << "," << rle_y - y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start x check y check z
inline int xzyRleCompress_00z(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(start_x, start_y, start_z, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = start_y; y < py; ++y)
		{
			for (int x = start_z; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					x++;
					while ((x < px) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						x++;
					}

					z--;
					// check if y compression is possible
					while ((z >= 0))
					{
						int value = 1;
						for (int i = rle_x; i < x; ++i)
						{
							if (cube_buffer[z][y][i] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_x; i < x; ++i)
							{
								cube_buffer[z][y][i] = '\0';
							}
							z--;
						}
					}

					y++;
					// check if z compression is possible
					while ((y < py))
					{
						int value = 1;
						for (int j = rle_z; j > z; --j)
						{
							for (int i = rle_x; i < x; ++i)
							{
								if (cube_buffer[j][y][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j > z; --j)
							{
								for (int i = rle_x; i < x; ++i)
								{
									cube_buffer[j][y][i] = '\0';
								}
							}
							y++;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << rle_y + oy << "," << z + 1 + oz << "," << x - rle_x << "," << y - rle_y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start x check y check z
inline int xzyRleCompress_0yz(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(start_x, start_y, start_z, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = start_z; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					x++;
					while ((x < px) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						x++;
					}

					z--;
					// check if y compression is possible
					while ((z >= 0))
					{
						int value = 1;
						for (int i = rle_x; i < x; ++i)
						{
							if (cube_buffer[z][y][i] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_x; i < x; ++i)
							{
								cube_buffer[z][y][i] = '\0';
							}
							z--;
						}
					}

					y--;
					// check if z compression is possible
					while ((y >= 0))
					{
						int value = 1;
						for (int j = rle_z; j > z; --j)
						{
							for (int i = rle_x; i < x; ++i)
							{
								if (cube_buffer[j][y][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j > z; --j)
							{
								for (int i = rle_x; i < x; ++i)
								{
									cube_buffer[j][y][i] = '\0';
								}
							}
							y--;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << y + 1 + oy << "," << z + 1 + oz << "," << x - rle_x << "," << rle_y - y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start x check y check z
inline int xzyRleCompress_x00(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(start_x, start_y, start_z, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = start_z; z < pz; ++z)
	{
		for (int y = start_y; y < py; ++y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					x--;
					while ((x >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						x--;
					}

					z++;
					// check if y compression is possible
					while ((z < pz))
					{
						int value = 1;
						for (int i = rle_x; i > x; --i)
						{
							if (cube_buffer[z][y][i] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_x; i > x; --i)
							{
								cube_buffer[z][y][i] = '\0';
							}
							z++;
						}
					}

					y++;
					// check if z compression is possible
					while ((y < py))
					{
						int value = 1;
						for (int j = rle_z; j < z; ++j)
						{
							for (int i = rle_x; i > x; --i)
							{
								if (cube_buffer[j][y][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j < z; ++j)
							{
								for (int i = rle_x; i > x; --i)
								{
									cube_buffer[j][y][i] = '\0';
								}
							}
							y++;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << rle_y + oy << "," << rle_z + oz << "," << rle_x - x << "," << y - rle_y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start x check y check z
inline int xzyRleCompress_xy0(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(start_x, start_y, start_z, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = start_z; z < pz; ++z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					x--;
					while ((x >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						x--;
					}

					z++;
					// check if y compression is possible
					while ((z < pz))
					{
						int value = 1;
						for (int i = rle_x; i > x; --i)
						{
							if (cube_buffer[z][y][i] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_x; i > x; --i)
							{
								cube_buffer[z][y][i] = '\0';
							}
							z++;
						}
					}

					y--;
					// check if z compression is possible
					while ((y >= 0))
					{
						int value = 1;
						for (int j = rle_z; j < z; ++j)
						{
							for (int i = rle_x; i > x; --i)
							{
								if (cube_buffer[j][y][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j < z; ++j)
							{
								for (int i = rle_x; i > x; --i)
								{
									cube_buffer[j][y][i] = '\0';
								}
							}
							y--;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << y + 1 + oy << "," << rle_z + oz << "," << rle_x - x << "," << rle_y - y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start x check y check z
inline int xzyRleCompress_x0z(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(start_x, start_y, start_z, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = start_y; y < py; ++y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					x--;
					while ((x >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						x--;
					}

					z--;
					// check if y compression is possible
					while ((z >= 0))
					{
						int value = 1;
						for (int i = rle_x; i > x; --i)
						{
							if (cube_buffer[z][y][i] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_x; i > x; --i)
							{
								cube_buffer[z][y][i] = '\0';
							}
							z--;
						}
					}

					y++;
					// check if z compression is possible
					while ((y < py))
					{
						int value = 1;
						for (int j = rle_z; j > z; --j)
						{
							for (int i = rle_x; i > x; --i)
							{
								if (cube_buffer[j][y][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j > z; --j)
							{
								for (int i = rle_x; i > x; --i)
								{
									cube_buffer[j][y][i] = '\0';
								}
							}
							y++;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << rle_y + oy << "," << z + 1 + oz << "," << rle_x - x << "," << y - rle_y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start x check y check z
inline int xzyRleCompress_xyz(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(start_x, start_y, start_z, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					x--;
					while ((x >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						x--;
					}

					z--;
					// check if y compression is possible
					while ((z >= 0))
					{
						int value = 1;
						for (int i = rle_x; i > x; --i)
						{
							if (cube_buffer[z][y][i] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_x; i > x; --i)
							{
								cube_buffer[z][y][i] = '\0';
							}
							z--;
						}
					}

					y--;
					// check if z compression is possible
					while ((y >= 0))
					{
						int value = 1;
						for (int j = rle_z; j > z; --j)
						{
							for (int i = rle_x; i > x; --i)
							{
								if (cube_buffer[j][y][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j > z; --j)
							{
								for (int i = rle_x; i > x; --i)
								{
									cube_buffer[j][y][i] = '\0';
								}
							}
							y--;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << y + 1 + oy << "," << z + 1 + oz << "," << rle_x - x << "," << rle_y - y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start y check x check z
inline int yxzRleCompress_000(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(start_x, start_y, start_z, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = start_x; z < pz; ++z)
	{
		for (int y = start_y; y < py; ++y)
		{
			for (int x = start_z; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					y++;

					while ((y < py) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						y++;
					}

					x++;
					// check if x compression is possible
					while ((x < px))
					{
						int value = 1;
						for (int i = rle_y; i < y; ++i)
						{
							if (cube_buffer[z][i][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_y; i < y; ++i)
							{
								cube_buffer[z][i][x] = '\0';
							}
							x++;
						}
					}

					z++;
					// check if z compression is possible
					while ((z < pz))
					{
						int value = 1;
						for (int j = rle_y; j < y; ++j)
						{
							for (int i = rle_x; i < x; ++i)
							{
								if (cube_buffer[z][j][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j < y; ++j)
							{
								for (int i = rle_x; i < x; ++i)
								{
									cube_buffer[z][j][i] = '\0';
								}
							}
							z++;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << rle_y + oy << "," << rle_z + oz << "," << x - rle_x << "," << y - rle_y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					if (x < px)
					{
						x--;
					}
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start y check x check z
inline int yxzRleCompress_0y0(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(start_x, start_y, start_z, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = start_x; z < pz; ++z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = start_z; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					y--;

					while ((y >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						y--;
					}

					x++;
					// check if x compression is possible
					while ((x < px))
					{
						int value = 1;
						for (int i = rle_y; i > y; --i)
						{
							if (cube_buffer[z][i][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_y; i > y; --i)
							{
								cube_buffer[z][i][x] = '\0';
							}
							x++;
						}
					}

					z++;
					// check if z compression is possible
					while ((z < pz))
					{
						int value = 1;
						for (int j = rle_y; j > y; --j)
						{
							for (int i = rle_x; i < x; ++i)
							{
								if (cube_buffer[z][j][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j > y; --j)
							{
								for (int i = rle_x; i < x; ++i)
								{
									cube_buffer[z][j][i] = '\0';
								}
							}
							z++;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << y + 1 + oy << "," << rle_z + oz << "," << x - rle_x << "," << rle_y - y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					if (x < px)
					{
						x--;
					}
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start y check x check z
inline int yxzRleCompress_00z(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(start_x, start_y, start_z, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = start_y; y < py; ++y)
		{
			for (int x = start_x; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					y++;

					while ((y < py) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						y++;
					}

					x++;
					// check if x compression is possible
					while ((x < px))
					{
						int value = 1;
						for (int i = rle_y; i < y; ++i)
						{
							if (cube_buffer[z][i][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_y; i < y; ++i)
							{
								cube_buffer[z][i][x] = '\0';
							}
							x++;
						}
					}

					z--;
					// check if z compression is possible
					while ((z >= 0))
					{
						int value = 1;
						for (int j = rle_y; j < y; ++j)
						{
							for (int i = rle_x; i < x; ++i)
							{
								if (cube_buffer[z][j][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j < y; ++j)
							{
								for (int i = rle_x; i < x; ++i)
								{
									cube_buffer[z][j][i] = '\0';
								}
							}
							z--;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << rle_y + oy << "," << z + 1 + oz << "," << x - rle_x << "," << y - rle_y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					if (x < px)
					{
						x--;
					}
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start y check x check z
inline int yxzRleCompress_0yz(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(start_x, start_y, start_z, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = start_z; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;

					y--;

					while ((y >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						y--;
					}

					x++;
					// check if x compression is possible
					while ((x < px))
					{
						int value = 1;
						for (int i = rle_y; i > y; --i)
						{
							if (cube_buffer[z][i][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_y; i > y; --i)
							{
								cube_buffer[z][i][x] = '\0';
							}
							x++;
						}
					}

					z--;
					// check if z compression is possible
					while ((z >= 0))
					{
						int value = 1;
						for (int j = rle_y; j > y; --j)
						{
							for (int i = rle_x; i < x; ++i)
							{
								if (cube_buffer[z][j][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j > y; --j)
							{
								for (int i = rle_x; i < x; ++i)
								{
									cube_buffer[z][j][i] = '\0';
								}
							}
							z--;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << y + 1 + oy << "," << z + 1 + oz << "," << x - rle_x << "," << rle_y - y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					if (x < px)
					{
						x--;
					}
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

inline int yxzRleCompress_x00(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(start_x, start_y, start_z, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = 0; z < pz; ++z)
	{
		for (int y = 0; y < py; ++y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					y++;

					while ((y < py) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						y++;
					}

					x--;
					// check if x compression is possible
					while ((x >= 0))
					{
						int value = 1;
						for (int i = rle_y; i < y; ++i)
						{
							if (cube_buffer[z][i][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_y; i < y; ++i)
							{
								cube_buffer[z][i][x] = '\0';
							}
							x--;
						}
					}

					z++;
					// check if z compression is possible
					while ((z < pz))
					{
						int value = 1;
						for (int j = rle_y; j < y; ++j)
						{
							for (int i = rle_x; i > x; --i)
							{
								if (cube_buffer[z][j][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j < y; ++j)
							{
								for (int i = rle_x; i > x; --i)
								{
									cube_buffer[z][j][i] = '\0';
								}
							}
							z++;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << rle_y + oy << "," << rle_z + oz << "," << rle_x - x << "," << y - rle_y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start y check x check z
inline int yxzRleCompress_xy0(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(start_x, start_y, start_z, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = 0; z < pz; ++z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					y--;

					while ((y >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						y--;
					}

					x--;
					// check if x compression is possible
					while ((x >= 0))
					{
						int value = 1;
						for (int i = rle_y; i > y; --i)
						{
							if (cube_buffer[z][i][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_y; i > y; --i)
							{
								cube_buffer[z][i][x] = '\0';
							}
							x--;
						}
					}

					z++;
					// check if z compression is possible
					while ((z < pz))
					{
						int value = 1;
						for (int j = rle_y; j > y; --j)
						{
							for (int i = rle_x; i > x; --i)
							{
								if (cube_buffer[z][j][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j > y; --j)
							{
								for (int i = rle_x; i > x; --i)
								{
									cube_buffer[z][j][i] = '\0';
								}
							}
							z++;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << y + 1 + oy << "," << rle_z + oz << "," << rle_x - x << "," << rle_y - y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start y check x check z
inline int yxzRleCompress_x0z(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(start_x, start_y, start_z, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = start_y; y < py; ++y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					y++;

					while ((y < py) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						y++;
					}

					x--;
					// check if x compression is possible
					while ((x >= 0))
					{
						int value = 1;
						for (int i = rle_y; i < y; ++i)
						{
							if (cube_buffer[z][i][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_y; i < y; ++i)
							{
								cube_buffer[z][i][x] = '\0';
							}
							x--;
						}
					}

					z--;
					// check if z compression is possible
					while ((z >= 0))
					{
						int value = 1;
						for (int j = rle_y; j < y; ++j)
						{
							for (int i = rle_x; i > x; --i)
							{
								if (cube_buffer[z][j][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j < y; ++j)
							{
								for (int i = rle_x; i > x; --i)
								{
									cube_buffer[z][j][i] = '\0';
								}
							}
							z--;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << rle_y + oy << "," << z + 1 + oz << "," << rle_x - x << "," << y - rle_y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start y check x check z
inline int yxzRleCompress_xyz(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(start_x, start_y, start_z, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;

					y--;

					while ((y >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						y--;
					}

					x--;
					// check if x compression is possible
					while ((x >= 0))
					{
						int value = 1;
						for (int i = rle_y; i > y; --i)
						{
							if (cube_buffer[z][i][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_y; i > y; --i)
							{
								cube_buffer[z][i][x] = '\0';
							}
							x--;
						}
					}

					z--;
					// check if z compression is possible
					while ((z >= 0))
					{
						int value = 1;
						for (int j = rle_y; j > y; --j)
						{
							for (int i = rle_x; i > x; --i)
							{
								if (cube_buffer[z][j][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j > y; --j)
							{
								for (int i = rle_x; i > x; --i)
								{
									cube_buffer[z][j][i] = '\0';
								}
							}
							z--;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << y + 1 + oy << "," << z + 1 + oz << "," << rle_x - x << "," << rle_y - y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start y check x check z
inline int yzxRleCompress(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(start_x, start_y, start_z, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = start_x; z < pz; ++z)
	{
		for (int y = start_y; y < py; ++y)
		{
			for (int x = start_z; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					y++;

					while ((y < py) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						y++;
					}

					z++;
					// check if x compression is possible
					while ((z < pz))
					{
						int value = 1;
						for (int i = rle_y; i < y; ++i)
						{
							if (cube_buffer[z][i][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_y; i < y; ++i)
							{
								cube_buffer[z][i][x] = '\0';
							}
							z++;
						}
					}

					x++;
					// check if z compression is possible
					while ((x < px))
					{
						int value = 1;
						for (int j = rle_y; j < y; ++j)
						{
							for (int i = rle_z; i < z; ++i)
							{
								if (cube_buffer[i][j][x] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j < y; ++j)
							{
								for (int i = rle_z; i < z; ++i)
								{
									cube_buffer[i][j][x] = '\0';
								}
							}
							x++;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << rle_y + oy << "," << rle_z + oz << "," << x - rle_x << "," << y - rle_y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					if (x < px)
					{
						x--;
					}
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}
// run length encoding
// start y check x check z
inline int yzxRleCompress_0y0(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = 0; z < pz; ++z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = 0; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					y--;

					while ((y >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						y--;
					}

					z++;
					// check if x compression is possible
					while ((z < pz))
					{
						int value = 1;
						for (int i = rle_y; i > y; --i)
						{
							if (cube_buffer[z][i][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_y; i > y; --i)
							{
								cube_buffer[z][i][x] = '\0';
							}
							z++;
						}
					}

					x++;
					// check if z compression is possible
					while ((x < px))
					{
						int value = 1;
						for (int j = rle_y; j > y; --j)
						{
							for (int i = rle_z; i < z; ++i)
							{
								if (cube_buffer[i][j][x] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j > y; --j)
							{
								for (int i = rle_z; i < z; ++i)
								{
									cube_buffer[i][j][x] = '\0';
								}
							}
							x++;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << y + 1 + oy << "," << rle_z + oz << "," << x - rle_x << "," << rle_y - y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start y check x check z
inline int yzxRleCompress_00z(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = 0; y < py; ++y)
		{
			for (int x = 0; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					y++;

					while ((y < py) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						y++;
					}

					z--;
					// check if x compression is possible
					while ((z >= 0))
					{
						int value = 1;
						for (int i = rle_y; i < y; ++i)
						{
							if (cube_buffer[z][i][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_y; i < y; ++i)
							{
								cube_buffer[z][i][x] = '\0';
							}
							z--;
						}
					}

					x++;
					// check if z compression is possible
					while ((x < px))
					{
						int value = 1;
						for (int j = rle_y; j < y; ++j)
						{
							for (int i = rle_z; i > z; --i)
							{
								if (cube_buffer[i][j][x] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j < y; ++j)
							{
								for (int i = rle_z; i > z; --i)
								{
									cube_buffer[i][j][x] = '\0';
								}
							}
							x++;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << rle_y + oy << "," << z + 1 + oz << "," << x - rle_x << "," << y - rle_y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start y check x check z
inline int yzxRleCompress_0yz(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = 0; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					y--;

					while ((y >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						y--;
					}

					z--;
					// check if x compression is possible
					while ((z >= 0))
					{
						int value = 1;
						for (int i = rle_y; i > y; --i)
						{
							if (cube_buffer[z][i][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_y; i > y; --i)
							{
								cube_buffer[z][i][x] = '\0';
							}
							z--;
						}
					}

					x++;
					// check if z compression is possible
					while ((x < px))
					{
						int value = 1;
						for (int j = rle_y; j > y; --j)
						{
							for (int i = rle_z; i > z; --i)
							{
								if (cube_buffer[i][j][x] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j > y; --j)
							{
								for (int i = rle_z; i > z; --i)
								{
									cube_buffer[i][j][x] = '\0';
								}
							}
							x++;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << y + 1 + oy << "," << z + 1 + oz << "," << x - rle_x << "," << rle_y - y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start y check x check z
inline int yzxRleCompress_x00(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = 0; z < pz; ++z)
	{
		for (int y = 0; y < py; ++y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					y++;

					while ((y < py) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						y++;
					}

					z++;
					// check if x compression is possible
					while ((z < pz))
					{
						int value = 1;
						for (int i = rle_y; i < y; ++i)
						{
							if (cube_buffer[z][i][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_y; i < y; ++i)
							{
								cube_buffer[z][i][x] = '\0';
							}
							z++;
						}
					}

					x--;
					// check if z compression is possible
					while ((x >= 0))
					{
						int value = 1;
						for (int j = rle_y; j < y; ++j)
						{
							for (int i = rle_z; i < z; ++i)
							{
								if (cube_buffer[i][j][x] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j < y; ++j)
							{
								for (int i = rle_z; i < z; ++i)
								{
									cube_buffer[i][j][x] = '\0';
								}
							}
							x--;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << rle_y + oy << "," << rle_z + oz << "," << rle_x - x << "," << y - rle_y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start y check x check z
inline int yzxRleCompress_x0z(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = 0; y < py; ++y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					y++;

					while ((y < py) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						y++;
					}

					z--;
					// check if x compression is possible
					while ((z >= 0))
					{
						int value = 1;
						for (int i = rle_y; i < y; ++i)
						{
							if (cube_buffer[z][i][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_y; i < y; ++i)
							{
								cube_buffer[z][i][x] = '\0';
							}
							z--;
						}
					}

					x--;
					// check if z compression is possible
					while ((x >= 0))
					{
						int value = 1;
						for (int j = rle_y; j < y; ++j)
						{
							for (int i = rle_z; i > z; --i)
							{
								if (cube_buffer[i][j][x] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j < y; ++j)
							{
								for (int i = rle_z; i > z; --i)
								{
									cube_buffer[i][j][x] = '\0';
								}
							}
							x--;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << rle_y + oy << "," << z + 1 + oz << "," << rle_x - x << "," << y - rle_y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start y check x check z
inline int yzxRleCompress_xy0(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = 0; z < pz; ++z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					y--;

					while ((y >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						y--;
					}

					z++;
					// check if x compression is possible
					while ((z < pz))
					{
						int value = 1;
						for (int i = rle_y; i > y; --i)
						{
							if (cube_buffer[z][i][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_y; i > y; --i)
							{
								cube_buffer[z][i][x] = '\0';
							}
							z++;
						}
					}

					x--;
					// check if z compression is possible
					while ((x >= 0))
					{
						int value = 1;
						for (int j = rle_y; j > y; --j)
						{
							for (int i = rle_z; i < z; ++i)
							{
								if (cube_buffer[i][j][x] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j > y; --j)
							{
								for (int i = rle_z; i < z; ++i)
								{
									cube_buffer[i][j][x] = '\0';
								}
							}
							x--;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << y + 1 + oy << "," << rle_z + oz << "," << rle_x - x << "," << rle_y - y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// start y check x check z
inline int yzxRleCompress_xyz(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					y--;

					while ((y >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						y--;
					}

					z--;
					// check if x compression is possible
					while ((z >= 0))
					{
						int value = 1;
						for (int i = rle_y; i > y; --i)
						{
							if (cube_buffer[z][i][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_y; i > y; --i)
							{
								cube_buffer[z][i][x] = '\0';
							}
							z--;
						}
					}

					x--;
					// check if z compression is possible
					while ((x >= 0))
					{
						int value = 1;
						for (int j = rle_y; j > y; --j)
						{
							for (int i = rle_z; i > z; --i)
							{
								if (cube_buffer[i][j][x] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_y; j > y; --j)
							{
								for (int i = rle_z; i > z; --i)
								{
									cube_buffer[i][j][x] = '\0';
								}
							}
							x--;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << y + 1 + oy << "," << z + 1 + oz << "," << rle_x - x << "," << rle_y - y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// z then y then x
inline int zyxRleCompress(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(start_x, start_y, start_z, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = start_x; z < pz; ++z)
	{
		for (int y = start_y; y < py; ++y)
		{
			for (int x = start_z; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					z++;
					while ((z < pz) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						z++;
					}

					y++;
					// check if y compression is possible
					while ((y < py))
					{
						int value = 1;
						for (int i = rle_z; i < z; ++i)
						{
							if (cube_buffer[i][y][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_z; i < z; ++i)
							{
								cube_buffer[i][y][x] = '\0';
							}
							y++;
						}
					}

					x++;
					// check if z compression is possible
					while ((x < px))
					{
						int value = 1;
						for (int j = rle_z; j < z; ++j)
						{
							for (int i = rle_y; i < y; ++i)
							{
								if (cube_buffer[j][i][x] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j < z; ++j)
							{
								for (int i = rle_y; i < y; ++i)
								{
									cube_buffer[j][i][x] = '\0';
								}
							}
							x++;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << rle_y + oy << "," << rle_z + oz << "," << x - rle_x << "," << y - rle_y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}
// run length encoding
// z then y then x
inline int zyxRleCompress_0y0(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = 0; z < pz; ++z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = 0; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					z++;
					while ((z < pz) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						z++;
					}

					y--;
					// check if y compression is possible
					while ((y >= 0))
					{
						int value = 1;
						for (int i = rle_z; i < z; ++i)
						{
							if (cube_buffer[i][y][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_z; i < z; ++i)
							{
								cube_buffer[i][y][x] = '\0';
							}
							y--;
						}
					}

					x++;
					// check if z compression is possible
					while ((x < px))
					{
						int value = 1;
						for (int j = rle_z; j < z; ++j)
						{
							for (int i = rle_y; i > y; --i)
							{
								if (cube_buffer[j][i][x] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j < z; ++j)
							{
								for (int i = rle_y; i > y; --i)
								{
									cube_buffer[j][i][x] = '\0';
								}
							}
							x++;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << y + 1 + oy << "," << rle_z + oz << "," << x - rle_x << "," << rle_y - y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// z then y then x
inline int zyxRleCompress_00z(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = 0; y < py; ++y)
		{
			for (int x = 0; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					z--;
					while ((z >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						z--;
					}

					y++;
					// check if y compression is possible
					while ((y < py))
					{
						int value = 1;
						for (int i = rle_z; i > z; --i)
						{
							if (cube_buffer[i][y][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_z; i > z; --i)
							{
								cube_buffer[i][y][x] = '\0';
							}
							y++;
						}
					}

					x++;
					// check if z compression is possible
					while ((x < px))
					{
						int value = 1;
						for (int j = rle_z; j > z; --j)
						{
							for (int i = rle_y; i < y; ++i)
							{
								if (cube_buffer[j][i][x] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j > z; --j)
							{
								for (int i = rle_y; i < y; ++i)
								{
									cube_buffer[j][i][x] = '\0';
								}
							}
							x++;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << rle_y + oy << "," << z + 1 + oz << "," << x - rle_x << "," << y - rle_y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// z then y then x
inline int zyxRleCompress_0yz(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = 0; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					z--;
					while ((z >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						z--;
					}

					y--;
					// check if y compression is possible
					while ((y >= 0))
					{
						int value = 1;
						for (int i = rle_z; i > z; --i)
						{
							if (cube_buffer[i][y][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_z; i > z; --i)
							{
								cube_buffer[i][y][x] = '\0';
							}
							y--;
						}
					}

					x++;
					// check if z compression is possible
					while ((x < px))
					{
						int value = 1;
						for (int j = rle_z; j > z; --j)
						{
							for (int i = rle_y; i > y; --i)
							{
								if (cube_buffer[j][i][x] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j > z; --j)
							{
								for (int i = rle_y; i > y; --i)
								{
									cube_buffer[j][i][x] = '\0';
								}
							}
							x++;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << y + 1 + oy << "," << z + 1 + oz << "," << x - rle_x << "," << rle_y - y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// z then y then x
inline int zyxRleCompress_x00(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = 0; z < pz; ++z)
	{
		for (int y = 0; y < py; ++y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					z++;
					while ((z < pz) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						z++;
					}

					y++;
					// check if y compression is possible
					while ((y < py))
					{
						int value = 1;
						for (int i = rle_z; i < z; ++i)
						{
							if (cube_buffer[i][y][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_z; i < z; ++i)
							{
								cube_buffer[i][y][x] = '\0';
							}
							y++;
						}
					}

					x--;
					// check if z compression is possible
					while ((x >= 0))
					{
						int value = 1;
						for (int j = rle_z; j < z; ++j)
						{
							for (int i = rle_y; i < y; ++i)
							{
								if (cube_buffer[j][i][x] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j < z; ++j)
							{
								for (int i = rle_y; i < y; ++i)
								{
									cube_buffer[j][i][x] = '\0';
								}
							}
							x--;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << rle_y + oy << "," << rle_z + oz << "," << rle_x - x << "," << y - rle_y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// z then y then x
inline int zyxRleCompress_xy0(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = 0; z < pz; ++z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					z++;
					while ((z < pz) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						z++;
					}

					y--;
					// check if y compression is possible
					while ((y >= 0))
					{
						int value = 1;
						for (int i = rle_z; i < z; ++i)
						{
							if (cube_buffer[i][y][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_z; i < z; ++i)
							{
								cube_buffer[i][y][x] = '\0';
							}
							y--;
						}
					}

					x--;
					// check if z compression is possible
					while ((x >= 0))
					{
						int value = 1;
						for (int j = rle_z; j < z; ++j)
						{
							for (int i = rle_y; i > y; --i)
							{
								if (cube_buffer[j][i][x] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j < z; ++j)
							{
								for (int i = rle_y; i > y; --i)
								{
									cube_buffer[j][i][x] = '\0';
								}
							}
							x--;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << y + 1 + oy << "," << rle_z + oz << "," << rle_x - x << "," << rle_y - y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// z then y then x
inline int zyxRleCompress_x0z(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = 0; y < py; ++y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					z--;
					while ((z >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						z--;
					}

					y++;
					// check if y compression is possible
					while ((y < py))
					{
						int value = 1;
						for (int i = rle_z; i > z; --i)
						{
							if (cube_buffer[i][y][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_z; i > z; --i)
							{
								cube_buffer[i][y][x] = '\0';
							}
							y++;
						}
					}

					x--;
					// check if z compression is possible
					while ((x >= 0))
					{
						int value = 1;
						for (int j = rle_z; j > z; --j)
						{
							for (int i = rle_y; i < y; ++i)
							{
								if (cube_buffer[j][i][x] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j > z; --j)
							{
								for (int i = rle_y; i < y; ++i)
								{
									cube_buffer[j][i][x] = '\0';
								}
							}
							x--;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << rle_y + oy << "," << z + 1 + oz << "," << rle_x - x << "," << y - rle_y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// z then y then x
inline int zyxRleCompress_xyz(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					z--;
					while ((z >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						z--;
					}

					y--;
					// check if y compression is possible
					while ((y >= 0))
					{
						int value = 1;
						for (int i = rle_z; i > z; --i)
						{
							if (cube_buffer[i][y][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_z; i > z; --i)
							{
								cube_buffer[i][y][x] = '\0';
							}
							y--;
						}
					}

					x--;
					// check if z compression is possible
					while ((x >= 0))
					{
						int value = 1;
						for (int j = rle_z; j > z; --j)
						{
							for (int i = rle_y; i > y; --i)
							{
								if (cube_buffer[j][i][x] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j > z; --j)
							{
								for (int i = rle_y; i > y; --i)
								{
									cube_buffer[j][i][x] = '\0';
								}
							}
							x--;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << y + 1 + oy << "," << z + 1 + oz << "," << rle_x - x << "," << rle_y - y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// z then y then x
inline int zxyRleCompress(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(start_x, start_y, start_z, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = start_x; z < pz; ++z)
	{
		for (int y = start_y; y < py; ++y)
		{
			for (int x = start_z; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					z++;
					while ((z < pz) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						z++;
					}

					x++;
					// check if y compression is possible
					while ((x < px))
					{
						int value = 1;
						for (int i = rle_z; i < z; ++i)
						{
							if (cube_buffer[i][y][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_z; i < z; ++i)
							{
								cube_buffer[i][y][x] = '\0';
							}
							x++;
						}
					}

					y++;
					// check if z compression is possible
					while ((y < py))
					{
						int value = 1;
						for (int j = rle_z; j < z; ++j)
						{
							for (int i = rle_x; i < x; ++i)
							{
								if (cube_buffer[j][y][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j < z; ++j)
							{
								for (int i = rle_x; i < x; ++i)
								{
									cube_buffer[j][y][i] = '\0';
								}
							}
							y++;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << rle_y + oy << "," << rle_z + oz << "," << x - rle_x << "," << y - rle_y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// z then y then x
inline int zxyRleCompress_0y0(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = 0; z < pz; ++z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = 0; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					z++;
					while ((z < pz) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						z++;
					}

					x++;
					// check if y compression is possible
					while ((x < px))
					{
						int value = 1;
						for (int i = rle_z; i < z; ++i)
						{
							if (cube_buffer[i][y][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_z; i < z; ++i)
							{
								cube_buffer[i][y][x] = '\0';
							}
							x++;
						}
					}

					y--;
					// check if z compression is possible
					while ((y >= 0))
					{
						int value = 1;
						for (int j = rle_z; j < z; ++j)
						{
							for (int i = rle_x; i < x; ++i)
							{
								if (cube_buffer[j][y][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j < z; ++j)
							{
								for (int i = rle_x; i < x; ++i)
								{
									cube_buffer[j][y][i] = '\0';
								}
							}
							y--;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << y + 1 + oy << "," << rle_z + oz << "," << x - rle_x << "," << rle_y - y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// z then y then x
inline int zxyRleCompress_00z(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = 0; y < py; ++y)
		{
			for (int x = 0; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					z--;
					while ((z >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						z--;
					}

					x++;
					// check if y compression is possible
					while ((x < px))
					{
						int value = 1;
						for (int i = rle_z; i > z; --i)
						{
							if (cube_buffer[i][y][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_z; i > z; --i)
							{
								cube_buffer[i][y][x] = '\0';
							}
							x++;
						}
					}

					y++;
					// check if z compression is possible
					while ((y < py))
					{
						int value = 1;
						for (int j = rle_z; j > z; --j)
						{
							for (int i = rle_x; i < x; ++i)
							{
								if (cube_buffer[j][y][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j > z; --j)
							{
								for (int i = rle_x; i < x; ++i)
								{
									cube_buffer[j][y][i] = '\0';
								}
							}
							y++;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << rle_y + oy << "," << z + 1 + oz << "," << x - rle_x << "," << y - rle_y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// z then y then x
inline int zxyRleCompress_0yz(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = 0; x < px; ++x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					z--;
					while ((z >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						z--;
					}

					x++;
					// check if y compression is possible
					while ((x < px))
					{
						int value = 1;
						for (int i = rle_z; i > z; --i)
						{
							if (cube_buffer[i][y][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_z; i > z; --i)
							{
								cube_buffer[i][y][x] = '\0';
							}
							x++;
						}
					}

					y--;
					// check if z compression is possible
					while ((y >= 0))
					{
						int value = 1;
						for (int j = rle_z; j > z; --j)
						{
							for (int i = rle_x; i < x; ++i)
							{
								if (cube_buffer[j][y][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j > z; --j)
							{
								for (int i = rle_x; i < x; ++i)
								{
									cube_buffer[j][y][i] = '\0';
								}
							}
							y--;
						}
					}
					if (print == 1)
						std::cout << rle_x + ox << "," << y + 1 + oy << "," << z + 1 + oz << "," << x - rle_x << "," << rle_y - y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// z then y then x
inline int zxyRleCompress_x00(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = 0; z < pz; ++z)
	{
		for (int y = 0; y < py; ++y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					z++;
					while ((z < pz) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						z++;
					}

					x--;
					// check if y compression is possible
					while ((x >= 0))
					{
						int value = 1;
						for (int i = rle_z; i < z; ++i)
						{
							if (cube_buffer[i][y][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_z; i < z; ++i)
							{
								cube_buffer[i][y][x] = '\0';
							}
							x--;
						}
					}

					y++;
					// check if z compression is possible
					while ((y < py))
					{
						int value = 1;
						for (int j = rle_z; j < z; ++j)
						{
							for (int i = rle_x; i > x; --i)
							{
								if (cube_buffer[j][y][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j < z; ++j)
							{
								for (int i = rle_x; i > x; --i)
								{
									cube_buffer[j][y][i] = '\0';
								}
							}
							y++;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << rle_y + oy << "," << rle_z + oz << "," << rle_x - x << "," << y - rle_y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// z then y then x
inline int zxyRleCompress_xy0(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = 0; z < pz; ++z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					z++;
					while ((z < pz) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						z++;
					}

					x--;
					// check if y compression is possible
					while ((x >= 0))
					{
						int value = 1;
						for (int i = rle_z; i < z; ++i)
						{
							if (cube_buffer[i][y][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_z; i < z; ++i)
							{
								cube_buffer[i][y][x] = '\0';
							}
							x--;
						}
					}

					y--;
					// check if z compression is possible
					while ((y >= 0))
					{
						int value = 1;
						for (int j = rle_z; j < z; ++j)
						{
							for (int i = rle_x; i > x; --i)
							{
								if (cube_buffer[j][y][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j < z; ++j)
							{
								for (int i = rle_x; i > x; --i)
								{
									cube_buffer[j][y][i] = '\0';
								}
							}
							y--;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << y + 1 + oy << "," << rle_z + oz << "," << rle_x - x << "," << rle_y - y << "," << z - rle_z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// z then y then x
inline int zxyRleCompress_x0z(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = 0; y < py; ++y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					z--;
					while ((z >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						z--;
					}

					x--;
					// check if y compression is possible
					while ((x >= 0))
					{
						int value = 1;
						for (int i = rle_z; i > z; --i)
						{
							if (cube_buffer[i][y][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_z; i > z; --i)
							{
								cube_buffer[i][y][x] = '\0';
							}
							x--;
						}
					}

					y++;
					// check if z compression is possible
					while ((y < py))
					{
						int value = 1;
						for (int j = rle_z; j > z; --j)
						{
							for (int i = rle_x; i > x; --i)
							{
								if (cube_buffer[j][y][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j > z; --j)
							{
								for (int i = rle_x; i > x; --i)
								{
									cube_buffer[j][y][i] = '\0';
								}
							}
							y++;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << rle_y + oy << "," << z + 1 + oz << "," << rle_x - x << "," << y - rle_y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
}

// run length encoding
// z then y then x
inline int zxyRleCompress_xyz(const int &start_x, const int &start_y, const int &start_z, const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***c_buffer, std::unordered_map<char, std::string> &tags, int print = 0)
{
	char ***cube_buffer = allocateCubeBuffer(px, py, pz);
	copyBuffer(0, 0, 0, px, py, pz, c_buffer, cube_buffer);

	char current;
	int rle_x, rle_y, rle_z;
	int block_count = 0;
	for (int z = pz - 1; z >= 0; --z)
	{
		for (int y = py - 1; y >= 0; --y)
		{
			for (int x = px - 1; x >= 0; --x)
			{
				if (cube_buffer[z][y][x] != '\0')
				{
					current = cube_buffer[z][y][x];
					rle_x = x;
					rle_y = y;
					rle_z = z;
					z--;
					while ((z >= 0) && (cube_buffer[z][y][x] == current))
					{
						cube_buffer[z][y][x] = '\0';
						z--;
					}

					x--;
					// check if y compression is possible
					while ((x >= 0))
					{
						int value = 1;
						for (int i = rle_z; i > z; --i)
						{
							if (cube_buffer[i][y][x] != current)
							{
								value = 0;
							}
						}

						if (value != 1)
						{
							break;
						}
						else
						{
							for (int i = rle_z; i > z; --i)
							{
								cube_buffer[i][y][x] = '\0';
							}
							x--;
						}
					}

					y--;
					// check if z compression is possible
					while ((y >= 0))
					{
						int value = 1;
						for (int j = rle_z; j > z; --j)
						{
							for (int i = rle_x; i > x; --i)
							{
								if (cube_buffer[j][y][i] != current)
								{
									value = 0;
								}
							}
						}
						if (value != 1)
						{
							break;
						}
						else
						{
							for (int j = rle_z; j > z; --j)
							{
								for (int i = rle_x; i > x; --i)
								{
									cube_buffer[j][y][i] = '\0';
								}
							}
							y--;
						}
					}
					if (print == 1)
						std::cout << x + 1 + ox << "," << y + 1 + oy << "," << z + 1 + oz << "," << rle_x - x << "," << rle_y - y << "," << rle_z - z << "," << tags[current] << "\n";
					block_count++;
					x = rle_x;
					y = rle_y;
					z = rle_z;
				}
			}
		}
	}
	return block_count;
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
	int small_idx;

	while (true)
	{
		std::vector<int> block_count;
		readCubeIntoBuffer(px, py, pz, bx, by, bz, ox, oy, oz, cube_buffer, start);
		// These have 48 differnt combination of starting points and compression order of 3 dimensions of 3-Dimesional RLE

		block_count = {
			xyzRleCompress_000(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			xzyRleCompress_000(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			yxzRleCompress_000(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			yzxRleCompress(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			zxyRleCompress(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			zyxRleCompress(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),

			xyzRleCompress_0yz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			xyzRleCompress_0y0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			xyzRleCompress_00z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			xyzRleCompress_x00(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			xyzRleCompress_xy0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			xyzRleCompress_x0z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			xyzRleCompress_xyz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),

			xzyRleCompress_0y0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			xzyRleCompress_00z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			xzyRleCompress_0yz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			xzyRleCompress_x00(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			xzyRleCompress_xy0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			xzyRleCompress_x0z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			xzyRleCompress_xyz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),

			yxzRleCompress_0y0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			yxzRleCompress_00z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			yxzRleCompress_0yz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			yxzRleCompress_0y0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			yxzRleCompress_00z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			yxzRleCompress_0yz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			yxzRleCompress_x00(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			yxzRleCompress_xy0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			yxzRleCompress_x0z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			yxzRleCompress_xyz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),

			yzxRleCompress_0y0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			yzxRleCompress_00z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			yzxRleCompress_0yz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			yzxRleCompress_x00(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			yzxRleCompress_xy0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			yzxRleCompress_x0z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			yzxRleCompress_xyz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),

			zxyRleCompress_0y0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			zxyRleCompress_00z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			zxyRleCompress_0yz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			zxyRleCompress_x00(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			zxyRleCompress_xy0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			zxyRleCompress_x0z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			zxyRleCompress_xyz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),

			zyxRleCompress_0y0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			zyxRleCompress_00z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			zyxRleCompress_0yz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			zyxRleCompress_x00(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			zyxRleCompress_xy0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			zyxRleCompress_x0z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags),
			zyxRleCompress_xyz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags)

		};

		auto minElement = std::min_element(block_count.begin(), block_count.end());
		// for(auto ele :block_count)
		// 	std::cout<<ele<<" ";
		small_idx = std::distance(block_count.begin(), minElement);
		// std::cout<<small_idx;
		// std::cout<<small_idx<<"sf";

		switch (small_idx)
		{
		case 0:
			xyzRleCompress_000(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 1:
			xzyRleCompress_000(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 2:
			yxzRleCompress_000(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 3:
			yzxRleCompress(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 4:
			zxyRleCompress(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 5:
			zyxRleCompress(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 6:
			xyzRleCompress_0yz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 7:
			xyzRleCompress_0y0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 8:
			xyzRleCompress_00z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 9:
			xyzRleCompress_x00(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 10:
			xyzRleCompress_xy0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 11:
			xyzRleCompress_x0z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 12:
			xyzRleCompress_xyz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 13:
			xzyRleCompress_0y0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 14:
			xzyRleCompress_00z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 15:
			xzyRleCompress_0yz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 16:
			xzyRleCompress_x00(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 17:
			xzyRleCompress_xy0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 18:
			xzyRleCompress_x0z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 19:
			xzyRleCompress_xyz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 20:
			yxzRleCompress_0y0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 21:
			yxzRleCompress_00z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 22:
			xzyRleCompress_x0z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 23:
			yxzRleCompress_0yz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;
		case 24:
			yxzRleCompress_0y0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 25:
			yxzRleCompress_00z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 26:
			yxzRleCompress_0yz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 27:
			yxzRleCompress_x00(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 28:
			yxzRleCompress_xy0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 29:
			yxzRleCompress_x0z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 30:
			yxzRleCompress_xyz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 31:
			yzxRleCompress_0y0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 32:
			yzxRleCompress_00z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 33:
			yzxRleCompress_0yz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 34:
			yzxRleCompress_x00(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 35:
			yzxRleCompress_xy0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 36:
			yzxRleCompress_x0z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 37:
			yzxRleCompress_xyz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 38:
			zxyRleCompress_0y0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 39:
			zxyRleCompress_00z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 40:
			zxyRleCompress_0yz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 41:
			zxyRleCompress_x00(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 42:
			zxyRleCompress_xy0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 43:
			zxyRleCompress_x0z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 44:
			zxyRleCompress_xyz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 45:
			zyxRleCompress_0y0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 46:
			zyxRleCompress_00z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 47:
			zyxRleCompress_0yz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 48:
			zyxRleCompress_x00(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 49:
			zyxRleCompress_xy0(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 50:
			zyxRleCompress_x0z(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		case 51:
			zyxRleCompress_xyz(0, 0, 0, px, py, pz, ox, oy, oz, cube_buffer, tags, 1);
			break;

		default:
			break;
		}

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