#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NEW_LINE_WIDTH 1

#if defined(_WIN32)
    #define NEW_LINE_WIDTH 2
#endif

#define LABEL_LENGTH 200
#define ASCII_COUNT 256

int Z_COUNT = 0;

//Allocates a cube buffer
char ***allocateCubeBuffer(int px, int py, int pz)
{
    char ***cube_buffer = (char ***)malloc(pz * sizeof(char **));
    for (int z = 0; z < pz; z++) {
        cube_buffer[z] = (char **)malloc(py * sizeof(char *));
        for (int y = 0; y < py; y++) {
            cube_buffer[z][y] = (char *)malloc(px * sizeof(char));
        }
    }
    return cube_buffer;
}

//Allocates an aray of cube buffers
char ****allocateCubeArray(int px, int py, int pz, int bx, int by, int cube_count) {
	char ****cubes = (char ****) malloc(sizeof(char ***) * cube_count);
	for(int i=0;i<cube_count;i++) {
		cubes[i] = allocateCubeBuffer(px,py,pz);
	}
	return cubes;
}

//Prints a cube buffer
void printCubeBuffers(int px, int py, int pz, int wx, int wy, int wz, char ****cubes, char **map, int cube_count) {
	for(int i=0;i<cube_count;i++) {
		for(int z=0; z<pz; z++) {
			for(int y=0; y<py; y++) {
				for(int x=0; x<px; x++) {
					int ox = (i%wx)*px;
					int oy = (i/(wy))*py;
					int oz = (Z_COUNT * pz);
					printf("%d,%d,%d,%d,%d,%d,%s\n", x+ox, y+oy, z + oz, 1, 1, 1, map[cubes[i][z][y][x]]);
				}
			}
		}
	}
	Z_COUNT++;
}

//Skips n newline chars
void skipNewLine() {
	for(int s=0;s<NEW_LINE_WIDTH;s++) {
		fgetc(stdin);
	}
}

//Fills the array of cube buffers reading from stdin
void readIntoCubeArray(int bx, int by, int bz, int px, int py, int pz, char ****cubes, int cube_count, char **map) {
	int wx = bx/px;
	int wy = by/py;

	for(int z=0;z<pz;z++) {
		for(int y=0;y<by;y++) {
			for(int x=0;x<wx;x++) {
				fgets(cubes[x + (y/py) * wx][z%pz][y%py], px+1, stdin);
			}
			skipNewLine();
		}
		skipNewLine();
	}
}

//Allocates the tag table map
char **allocateMap() {
	char **map = (char **) malloc(sizeof(char *) * ASCII_COUNT);
	for(int i=0;i<ASCII_COUNT;i++) {
		map[i] = (char *) malloc(sizeof(char *) * LABEL_LENGTH);
	}
	return map;
}

int main() {
	//Read and store header information
	int bx, by, bz;
	int px, py, pz;
	fscanf(stdin,"%d,%d,%d,%d,%d,%d\n",&bx,&by,&bz,&px,&py,&pz);
	char line[LABEL_LENGTH+3];
	char symbol;
	char label[LABEL_LENGTH];
	char** map = allocateMap();

	while(1) {
		fgets(line, LABEL_LENGTH+4, stdin);
		if (line[0] == '\0' || line[0] == '\n' || line[1] == '\r') break;
		sscanf(line,"%c, %s\n",&symbol,label);
		strcpy(map[symbol], label);
	}

	int wz = bz/pz;
	int cube_count = (bx/px) * (by/py);
	char ****cubes = allocateCubeArray(px, py, pz, bx, by, cube_count);

	//Main loop
	for(int depth = 0;depth<wz;depth++) {
		readIntoCubeArray(bx,by,bz,px,py,pz,cubes,cube_count,map);

		/*
			IMPLEMENT COMPRESSION HERE
			1 .. n CUBES IN THE CUBE
			ARRAY
		*/

		printCubeBuffers(px,py,pz,wx,wy,wz,cubes,map,cube_count);
	}

	return 0;
}