#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#define NEW_LINE_WIDTH 1
#define LABEL_LENGTH 200
#define ASCII_COUNT 256

int THREAD_COUNT = 4;
int Z_COUNT = 0;

struct thread_args {
	int cube_count;
	int options_length;
	char **map;
	char ****cubes;
	int **options;
	int px;
	int py;
	int pz; 
	int wx;
	int wy;
	int wz;
	int thread;
};

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

//Skips n newline chars
void skipNewLine() {
	for(int s=0;s<NEW_LINE_WIDTH;s++) {
		fgetc(stdin);
	}
}

//Fills the array of cube buffers reading from stdin
void readIntoCubeArray(int bx, int by, int bz, int px, int py, int pz, int wx, int wy, int wz, char ****cubes, int cube_count, char **map) {
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

//Create the options
int **getOptions(int px, int py, int pz) {
	int **options = (int **) malloc(sizeof(int *) * (px) * (py) * (pz));
	int i=0;
	for(int z=0;z<pz;z++) {
		for(int y=0;y<py;y++) {
			for(int x=0;x<px;x++) {
				options[i] = (int *) malloc(sizeof(int) * 3);
				options[i][0] = x;
				options[i][1] = y;
				options[i][2] = z;
				i++;
			}
		}
	}
	return options;
}

//Lambda used for sorting the options
int compareOptions(const void *pa, const void *pb) {
	const int *a = *(const int **)pa;
	const int *b = *(const int **)pb;

	int a_sum = 0;
	for (int i = 0; i < 3; i++ ) {
		a_sum += a[i];
	}
	int b_sum = 0;
	for (int i = 0; i < 3; i++ ) {
		b_sum += b[i];
	}
	if (a_sum < b_sum){
		return -1;
	}
	if (a_sum > b_sum){
		return 1;
	}
	for (int j=0;j<3;j++){
		if (a[j] < b[j]){
			return 1;
		}
		if (a[j] > b[j]){
			return -1;
		}
	}
	
	return 0;
}

void greedy(int x_off,int y_off,int z_off,int x_par,int y_par,int z_par,int x_beta,int y_beta,int z_beta, char ***cube, char **map) {
    //loop over all the spots where the block could fit in the parent block
    int initial_block = cube[z_beta][y_beta][x_beta];
    if (initial_block == -1){
        return;
    }
    bool can_grow_x = true;
    bool can_grow_y = true;
    bool can_grow_z = true;
    int x_size = 1;
    int y_size = 1;
    int z_size = 1;
    do {
        can_grow_x = true;
        for (int z=0;z<z_size;z++){
            for (int y=0;y<y_size;y++){
                if (x_size+x_beta+1>x_par || cube[z+z_beta][y+y_beta][x_size+x_beta]!=initial_block ){
                    can_grow_x=false;
                    break;
                }
            }
            if (!can_grow_x){
                break;
            }
        }
        if (can_grow_x){
            //then grow x
            x_size++;
        }

        can_grow_y = true;
        for (int z=0;z<z_size;z++){
            for (int x=0;x<x_size;x++){
                if (y_size+y_beta+1>y_par || cube[z+z_beta][y_size+y_beta][x+x_beta]!=initial_block ){
                    can_grow_y=false;
                    break;
                }
            }
            if (!can_grow_y){
                break;
            }
        }
        if (can_grow_y){
            //then grow y
            y_size++;
        }

        can_grow_z = true;
        for (int y=0;y<y_size;y++){
            for (int x=0;x<x_size;x++){
                if (z_size+z_beta+1>z_par || cube[z_size+z_beta][y+y_beta][x+x_beta]!=initial_block){
                    can_grow_z=false;
                    break;
                }
                if (!can_grow_z){
                    break;
                }
            }
        }
        if (can_grow_z){
            //then grow z
            z_size++;
        }
    } while (can_grow_x || can_grow_y || can_grow_z);
    for (int a =0;a<z_size;a++){
        for (int b =0;b<y_size;b++){
            for (int c=0;c<x_size;c++){
                cube[a+z_beta][b+y_beta][c+x_beta]=-1;
            }
        }
    }
    printf("%d,%d,%d,%d,%d,%d,%s\n", x_off+x_beta, y_off+y_beta, z_off+z_beta, x_size, y_size, z_size, map[initial_block]);
}

void* threadJob(void *argsp) {
	struct thread_args *args = (struct thread_args *) argsp;

	int oz = (Z_COUNT * args->pz);

	for(int i=args->thread;i<args->cube_count;i=i+THREAD_COUNT) {
		int ox = (i%args->wx)*args->px;
		int oy = (i/(args->wx))*args->py;
		
		for (int j=0;j<args->options_length;j++){
			// printf("%i,%i,%i\n",args->options[j][0],args->options[j][1],args->options[j][2]);
			//see if we can fit that block anywhere in the parent block
	        greedy(ox,oy,oz,args->px,args->py,args->pz,args->options[j][0],args->options[j][1],args->options[j][2],args->cubes[i],args->map);
	    }
	}

	return NULL;
}
int main() {
	//Read and store header information
	int bx, by, bz;
	int px, py, pz;
	fscanf(stdin,"%d,%d,%d,%d,%d,%d\n",&bx,&by,&bz,&px,&py,&pz);
	char line[LABEL_LENGTH+3];
	char symbol;
	char label[LABEL_LENGTH];
	char **map = allocateMap();

	while(1) {
		fgets(line, LABEL_LENGTH+4, stdin);
		if (line[0] == '\0' || line[0] == '\n' || line[1] == '\r') break;
		sscanf(line,"%c, %s\n",&symbol,label);
		strcpy(map[symbol], label);
	}

	int wz = bz/pz;
	int wx = bx/px;
	int wy = by/py;
	int cube_count = (bx/px) * (by/py);
	int options_length = px*py*pz;
	char ****cubes = allocateCubeArray(px, py, pz, bx, by, cube_count);

	//Greedy stuff
	int **options = getOptions(px,py,pz);
	qsort((void*)options,options_length, sizeof(int *), compareOptions);

	//Thread stuff
	struct thread_args **arg_structs = (struct thread_args **)malloc(sizeof(struct thread_args)*THREAD_COUNT);
	pthread_t thread_ids[THREAD_COUNT];

	for(int i=0;i<THREAD_COUNT;i++) {
		arg_structs[i] = (struct thread_args *)malloc(sizeof(struct thread_args));
		arg_structs[i]->cube_count = cube_count;
		arg_structs[i]->options_length = options_length;
		arg_structs[i]->map = map; 
		arg_structs[i]->cubes = cubes;
		arg_structs[i]->options = options;
		arg_structs[i]->px = px;
		arg_structs[i]->py = py;
		arg_structs[i]->pz = pz;
		arg_structs[i]->wx = wx;
		arg_structs[i]->wy = wy;
		arg_structs[i]->wz = wz;
		arg_structs[i]->thread = i;
	}

	//Main loop
	for(int depth = 0;depth<wz;depth++) {
		readIntoCubeArray(bx,by,bz,px,py,pz,wx,wy,wz,cubes,cube_count,map);

		for(int i=0;i<THREAD_COUNT;i++) {
			pthread_create(&thread_ids[i], NULL, threadJob,(void *)arg_structs[i]);
		}

		for(int i=0;i<THREAD_COUNT;i++) {
			pthread_join(thread_ids[i], NULL);
		}

		Z_COUNT++;
	}

	return 0;
}
