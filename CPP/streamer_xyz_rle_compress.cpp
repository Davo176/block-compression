#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include <cstdio>
#include <stdio.h>
#include <mutex>
#include <condition_variable>

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

// run length encoding
inline void xyzRleCompress(const int &px, const int &py, const int &pz, const int &bx, const int &by, const int &bz, const int &ox, const int &oy, const int &oz, char ***cube_buffer, std::unordered_map<char, std::string> &tags)
{
	/*
Function Name: xyzRleCompress
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
	c_buffer: buffer used for the storage
	tags: map of input characters/tags with their original descriptions
Function Description:
	performs 3-Dimensional Run length Encoding
Function Return:
	N/A
*/
	char current;
	int rle_x, rle_y, rle_z;

	int z;
	int xx = 0, yy = 0, zz = 0;

	while (true)
	{
		for (z = 0; z < pz; ++z)
		{
			for (int y = 0; y < py; ++y)
			{
				for (int x = 0; x < px; ++x)
				{
					if (cube_buffer[z + zz][y + yy][x + xx] != '\0')
					{
						current = cube_buffer[z + zz][y + yy][x + xx];
						rle_x = x;
						rle_y = y;
						rle_z = z;
						while ((x < px) && (cube_buffer[z + zz][y + yy][x + xx] == current))
						{
							cube_buffer[z + zz][y + yy][x + xx] = '\0';
							x++;
						}

						y++;
						// check if y compression is possible
						while ((y < py))
						{
							int value = 1;
							for (int i = rle_x; i < x; ++i)
							{
								if (cube_buffer[z + zz][y + yy][i + xx] != current)
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
									cube_buffer[z + zz][y + yy][i + xx] = '\0';
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
									if (cube_buffer[z + zz][j + yy][i + xx] != current)
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
										cube_buffer[z + zz][j + yy][i + xx] = '\0';
									}
								}
								z++;
							}
						}

						std::cout << rle_x + ox + xx << "," << rle_y + oy + yy << "," << rle_z + oz + zz << "," << x - rle_x << "," << y - rle_y << "," << z - rle_z << "," << tags[current] << "\n";
						x = rle_x;
						y = rle_y;
						z = rle_z;
					}
				}
			}
		}
		xx += px;
		if (xx >= bx)
		{
			// break;
			xx = 0;
			yy += py;
		}
		if (yy >= by)
		{
			yy = 0;
			zz += pz;
		}
		if (zz >= bz)
		{
			break;
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

	char ***cube_buffer = allocateCubeBuffer(bx, by, pz);
	int start = ftell(stdin);
	int ox = 0;
	int oy = 0;
	long int oz = 0;
	char input;

	while (true)
	{

		for (int i = 0; i < pz; ++i)
		{
			for (int j = 0; j < by; ++j)
			{
				for (int k = 0; k < bx; ++k)
				{
					std::cin >> input;
					cube_buffer[i][j][k] = input;
				}
			}
		}
		xyzRleCompress(px, py, pz, bx, by, pz, ox, oy, oz, cube_buffer, tags);

		ox = 0;
		oy = 0;
		oz += pz;

		if (std::cin.eof())
		{
			break;
		}
	}

	return 0;
}