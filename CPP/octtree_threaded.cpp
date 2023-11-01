#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <cstdio>
#include <stdio.h>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

#define NEW_LINE_WIDTH 1

#if defined(_WIN32)
#define NEW_LINE_WIDTH 2
#endif

std::mutex mtx;
std::condition_variable cv;

std::queue<char ***> inputQueue;

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

// ----------------------------------------------------
//               Oct tree Compression
// ----------------------------------------------------

// Block representation of the data chunk
struct Block
{
    int x, y, z;
    int size_x, size_y, size_z;
    char value;
    std::string label;
};

// Node structure of the oct tree
struct Node
{
    Block block_value;
    bool isLeaf;
    Node *children[8];

    Node()
    {
        block_value.value = '\0';
        isLeaf = false;
        for (int i = 0; i < 8; ++i)
        {
            children[i] = nullptr;
        }
    }
};

// Build oct tree
Node *buildOctree(const int &px, const int &py, const int &pz, char ***cube_buffer,
                  const int &ox, const int &oy, const int &oz)
{
    /*
    Function Name: buildOctree
    Function Arguments:
        px : parent X count
        py : parent Y count
        pz : parent Z count
        cube_buffer : input buffer of px X py X pz size
        ox: start position of oct tree in x quadrant
        oy: start position of oct tree in y quadrant
        oz: start position of oct tree in z quadrant
    Function Description:
        builds the oct tree from the input data/cubebuffer
    Function Return:
        node: return the node of created oct tree
    */

    Node *node = new Node();

    // termination of recursion
    if (px <= 1 && py <= 1 && pz <= 1)
    {
        if (cube_buffer[oz][oy][ox])
            node->block_value.value = cube_buffer[oz][oy][ox];
        node->isLeaf = true;
        return node;
    }

    // oct tree creation process
    int hx = px > 1 ? px / 2 : px;
    int hy = py > 1 ? py / 2 : py;
    int hz = pz > 1 ? pz / 2 : pz;

    node->isLeaf = true;
    char base_value = cube_buffer[oz][oy][ox];

    for (int z = oz; z < oz + pz; ++z)
    {
        for (int y = oy; y < oy + py; ++y)
        {
            for (int x = ox; x < ox + px; ++x)
            {
                if (cube_buffer[z][y][x] != base_value)
                {
                    node->isLeaf = false;
                    break;
                }
            }
            if (!node->isLeaf)
            {
                break;
            }
        }
        if (!node->isLeaf)
        {
            break;
        }
    }

    if (!node->isLeaf)
    {
        node->children[0] = buildOctree(hx, hy, hz, cube_buffer, ox, oy, oz);
        node->children[1] = buildOctree(px - hx, hy, hz, cube_buffer, ox + hx, oy, oz);
        node->children[2] = buildOctree(hx, py - hy, hz, cube_buffer, ox, oy + hy, oz);
        node->children[3] = buildOctree(px - hx, py - hy, hz, cube_buffer, ox + hx, oy + hy, oz);
        if (pz > 1)
        {
            node->children[4] = buildOctree(hx, hy, pz - hz, cube_buffer, ox, oy, oz + hz);
            node->children[5] = buildOctree(px - hx, hy, pz - hz, cube_buffer, ox + hx, oy, oz + hz);
            node->children[6] = buildOctree(hx, py - hy, pz - hz, cube_buffer, ox, oy + hy, oz + hz);
            node->children[7] = buildOctree(px - hx, py - hy, pz - hz, cube_buffer, ox + hx, oy + hy, oz + hz);
        }
    }
    else
    {
        node->block_value.value = cube_buffer[oz][oy][ox];
    }

    return node;
}

void printCompressedBlock(const Block &block)
{
    /*
    Function Name: printCompressedBlock
    Function Arguments:
        block : block of data of Block structure(single data chunk)
    Function Description:
        prints the data into desired structure
    Function Return:
        N/A
    */
    std::cout << block.x << "," << block.y << "," << block.z << ","
              << block.size_x << "," << block.size_y << "," << block.size_z << ","
              << block.label << std::endl;
}

void encodeAndPrintBlocks(Node *node, const int &ox, const int &oy, const int &oz,
                          const int &size_x, const int &size_y, const int &size_z, const int &px, const int &py, const int &pz, std::unordered_map<char, std::string> &tags)
{
    /*
    Function Name: encodeAndPrintBlocks
    Function Arguments:
        node : node of oct tree
        ox: oct tree root position in x direction
        oy: oct tree root position in y direction
        oz: oct tree root position in z direction
        size_x: parent X count
        size_y: parent Y count
        size_z: parent Z count
        tags: map of tags that represent the input elements

    Function Description:
        traverses over the created oct tree to print the data in desired format
    Function Return:
        N/A
    */
    if (node->isLeaf)
    {
        Block block;
        block.x = ox;
        block.y = oy;
        block.z = oz;
        block.size_x = size_x;
        block.size_y = size_y;
        block.size_z = size_z == 0 ? size_z + 1 : size_z;
        block.label = tags[node->block_value.value];
        printCompressedBlock(block);
    }
    else
    {
        int hx = size_x > 1 ? size_x / 2 : size_x;
        int hy = size_y > 1 ? size_y / 2 : size_y;
        int hz = size_z > 1 ? size_z / 2 : size_z;

        encodeAndPrintBlocks(node->children[0], ox, oy, oz, hx, hy, hz, px, py, pz, tags);
        encodeAndPrintBlocks(node->children[1], ox + hx, oy, oz, size_x - hx, hy, hz, px, py, pz, tags);
        encodeAndPrintBlocks(node->children[2], ox, oy + hy, oz, hx, size_y - hy, hz, px, py, pz, tags);
        encodeAndPrintBlocks(node->children[3], ox + hx, oy + hy, oz, size_x - hx, size_y - hy, hz, px, py, pz, tags);
        if (size_z > 1)
        {
            encodeAndPrintBlocks(node->children[4], ox, oy, oz + hz, hx, hy, size_z - hz, px, py, pz, tags);
            encodeAndPrintBlocks(node->children[5], ox + hx, oy, oz + hz, size_x - hx, hy, size_z - hz, px, py, pz, tags);
            encodeAndPrintBlocks(node->children[6], ox, oy + hy, oz + hz, hx, size_y - hy, size_z - hz, px, py, pz, tags);
            encodeAndPrintBlocks(node->children[7], ox + hx, oy + hy, oz + hz, size_x - hx, size_y - hy, size_z - hz, px, py, pz, tags);
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

    // input thread that takes input into the buffer and sends it to oct tree
    auto inputThread = [&]()
    {
        readCubeIntoBuffer(px, py, pz, bx, by, bz, ox, oy, oz, cube_buffer, start);
        std::unique_lock<std::mutex> lock(mtx);
        inputQueue.push(cube_buffer);
        cv.notify_all();
    };

    // compress thread that takes input from the queue, creates oct tree, compresses data and displays it
    auto compressThread = [&]()
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, []
                { return !inputQueue.empty(); });

        // Simulate compression (replace with your actual compression logic)
        char ***cube_buffer = inputQueue.front();
        inputQueue.pop();
        Node *root = buildOctree(px, py, pz, cube_buffer, 0, 0, 0);
        encodeAndPrintBlocks(root, ox, oy, oz, px, py, pz, px, py, pz, tags);

        lock.unlock();
        cv.notify_all();
    };

    while (true)
    {
        std::thread inputThreadObj(inputThread);
        std::thread compressThreadObj(compressThread);

        inputThreadObj.join();
        compressThreadObj.join();
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