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


char*** allocateCubeBuffer(int px, int py, int pz)
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
    for (int z = 0; z < pz; z++) {
        cube_buffer[z] = (char **)malloc(py * sizeof(char *)); // For each z level allocate y_count y levels
        for (int y = 0; y < py; y++) {
            cube_buffer[z][y] = (char *)malloc(px * sizeof(char)); // For each y level allocate x_count x levels
        }
    }
    return cube_buffer;
}

//Free a cube buffer
void freeCubeBuffer(char*** cube_buffer,int py, int pz) {
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
    for (int z = 0; z < pz; z++) {
        for (int y = 0; y < py; y++) {
            free(cube_buffer[z][y]);
        }
        free(cube_buffer[z]);
    }
    free(cube_buffer);
}

//Reads a cube from the file into a cube buffer
inline void readCubeIntoBuffer(const int& px, const int& py, const int& pz, const int& bx, const int& by, const int& bz, const int& ox, const int& oy, const int& oz, char*** cube_buffer, const int& start) {
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
	int byte_offset = (oy * (bx+NEW_LINE_WIDTH)) + ox + ((by*(bx+NEW_LINE_WIDTH))+NEW_LINE_WIDTH) * oz;
	fseek(stdin,byte_offset, SEEK_CUR);

	for(int z=0; z<pz; z++) {
		for(int y=0; y<py; y++) {
			fgets(cube_buffer[z][y], px+1, stdin);
			if(y != py-1) fseek(stdin, bx-px+NEW_LINE_WIDTH, SEEK_CUR);
		}
		fseek(stdin, (by-py+1)*(bx+NEW_LINE_WIDTH)-px+NEW_LINE_WIDTH, SEEK_CUR);
	}
}

//Prints a cube buffer
inline void printCubeBuffer(const int& px, const int& py, const int& pz, const int& ox, const int& oy, const int& oz, char*** cube_buffer, std::unordered_map<char, std::string>& tags) {
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
    for(int z=0; z<pz; z++) {
		for(int y=0; y<py; y++) {
			for(int x=0; x<px; x++) {
				std::cout << x + ox << "," << y + oy << "," << z + oz << "," << 1 << "," << 1 << "," << 1 << "," << tags[cube_buffer[z][y][x]] << std::endl;
			}
		}
	}
}


struct Node {
    char value;
    bool isLeaf;
    Node* children[8];

    Node() {
        value = '\0';
        isLeaf = false;
        for (int i = 0; i < 8; ++i) {
            children[i] = nullptr;
        }
    }
};

// Build 3D quadtree
Node* buildQuadtree(const int& px, const int& py, const int& pz, char*** cube_buffer,
                    const int& ox, const int& oy, const int& oz) {
    Node* node = new Node();

    if (px == 1 && py == 1 && pz == 1) {
        node->value = cube_buffer[oz][oy][ox];
        node->isLeaf = true;
        return node;
    }

    int hx = px / 2;
    int hy = py / 2;
    int hz = pz / 2;

    node->isLeaf = true;
    char baseValue = cube_buffer[oz][oy][ox];
    for (int z = oz; z < oz + pz; ++z) {
        for (int y = oy; y < oy + py; ++y) {
            for (int x = ox; x < ox + px; ++x) {
                if (cube_buffer[z][y][x] != baseValue) {
                    node->isLeaf = false;
                    break;
                }
            }
            if (!node->isLeaf) {
                break;
            }
        }
        if (!node->isLeaf) {
            break;
        }
    }

    if (!node->isLeaf) {
        node->children[0] = buildQuadtree(hx, hy, hz, cube_buffer, ox, oy, oz);
        node->children[1] = buildQuadtree(px - hx, hy, hz, cube_buffer, ox + hx, oy, oz);
        node->children[2] = buildQuadtree(hx, py - hy, hz, cube_buffer, ox, oy + hy, oz);
        node->children[3] = buildQuadtree(px - hx, py - hy, hz, cube_buffer, ox + hx, oy + hy, oz);
        node->children[4] = buildQuadtree(hx, hy, pz - hz, cube_buffer, ox, oy, oz + hz);
        node->children[5] = buildQuadtree(px - hx, hy, pz - hz, cube_buffer, ox + hx, oy, oz + hz);
        node->children[6] = buildQuadtree(hx, py - hy, pz - hz, cube_buffer, ox, oy + hy, oz + hz);
        node->children[7] = buildQuadtree(px - hx, py - hy, pz - hz, cube_buffer, ox + hx, oy + hy, oz + hz);
    }

    return node;
}

// Encode 3D quadtree to a compressed format
void encodeQuadtree(Node* node) {
    if (node->isLeaf) {
        std::cout << 'L' << node->value;
    } else {
        std::cout << 'I';
        for (int i = 0; i < 8; ++i) {
            encodeQuadtree(node->children[i]);
        }
    }
}

struct Block {
    int x, y, z;
    int sizeX, sizeY, sizeZ;
    std::string label;
};

void printCompressedBlock(const Block& block) {
    std::cout << block.x << "," << block.y << "," << block.z << ","
              << block.sizeX << "," << block.sizeY << "," << block.sizeZ << ","
              << block.label << std::endl;
}

void encodeAndPrintBlocks(Node* node, const int& ox, const int& oy, const int& oz,
                          const int& sizeX, const int& sizeY, const int& sizeZ, std::unordered_map<char, std::string>& tags) {
    if (node->isLeaf) {
        Block block;
        block.x = ox;
        block.y = oy;
        block.z = oz;
        block.sizeX = sizeX;
        block.sizeY = sizeY;
        block.sizeZ = sizeZ;
        block.label = tags[node->value];
        printCompressedBlock(block);
    } else {
        int hx = sizeX / 2;
        int hy = sizeY / 2;
        int hz = sizeZ / 2;
        encodeAndPrintBlocks(node->children[0], ox, oy, oz, hx, hy, hz, tags);
        encodeAndPrintBlocks(node->children[1], ox + hx, oy, oz, sizeX - hx, hy, hz, tags);
        encodeAndPrintBlocks(node->children[2], ox, oy + hy, oz, hx, sizeY - hy, hz, tags);
        encodeAndPrintBlocks(node->children[3], ox + hx, oy + hy, oz, sizeX - hx, sizeY - hy, hz, tags);
        encodeAndPrintBlocks(node->children[4], ox, oy, oz + hz, hx, hy, sizeZ - hz, tags);
        encodeAndPrintBlocks(node->children[5], ox + hx, oy, oz + hz, sizeX - hx, hy, sizeZ - hz, tags);
        encodeAndPrintBlocks(node->children[6], ox, oy + hy, oz + hz, hx, sizeY - hy, sizeZ - hz, tags);
        encodeAndPrintBlocks(node->children[7], ox + hx, oy + hy, oz + hz, sizeX - hx, sizeY - hy, sizeZ - hz, tags);
    }
}

int main() {
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

	char*** cube_buffer = allocateCubeBuffer(px,py,pz);

	int start = ftell(stdin);

	int ox = 0;
	int oy = 0;
	int oz = 0;

	while(true) {
		readCubeIntoBuffer(px,py,pz,bx,by,bz,ox,oy,oz,cube_buffer,start);

		Node* root = buildQuadtree(px, py, pz, cube_buffer, 0, 0, 0);

        
        encodeAndPrintBlocks(root, ox, oy, oz, px, py, pz, tags);

			
		//printCubeBuffer(px,py,pz,ox,oy,oz,cube_buffer,tags);

		//Increment offset values
		ox += px;
		if(ox == bx) {
			ox = 0;
			oy += py;
		}

		if(oy == by) {
			oy=0;
			oz += pz;
		}

		if(oz == bz) {
			break;
		}
	}

	freeCubeBuffer(cube_buffer,py,pz);
	return 0;
}