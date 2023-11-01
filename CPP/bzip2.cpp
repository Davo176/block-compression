#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <bzlib.h>
#include <cstdio>

#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <list>
#include <string>
#include <queue>

#include <stdio.h>

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

//============================================================================================
//                  BZip 2 implementation
//============================================================================================

struct rotate
{
    std::string rotated;
    int idx;
};

// Burrows-Wheeler Transform
std::string burrowsWheelerTransform(const std::string &flattened_string)
{
    /*
Function Name: burrowsWheelerTransform
Function Arguments:
flattened_string: string input after flattening from 3 Dimensions to 1 Dimension
Function Description:
This function performs burrowsWheelerTransform and returns the transformed string
Function Return:
string: BWT string
*/
    // The processs is:
    // create vector of rotations from the flattened string
    // sort it
    // iterate through the sorted rotations
    // bwt operation over

    std::vector<rotate> rotations;
    for (int i = 0; i < flattened_string.length(); ++i)
    {
        rotate rot;
        rot.rotated = flattened_string.substr(i) + flattened_string.substr(0, i);
        rot.idx = i;
        rotations.push_back(rot);
    }

    std::sort(rotations.begin(), rotations.end(), [](const rotate &a, const rotate &b)
              { return a.rotated < b.rotated; });

    std::string bwt;
    for (const rotate &rot : rotations)
    {
        if (rot.idx == 0)
        {
            bwt = rot.rotated;
        }
    }

    return bwt;
}

// Run-Length Encoding
std::string runLengthEncode(const std::string &bwt)
{
    /*
Function Name: runLengthEncode
Function Arguments:
bwt: string input after burrowsWheelerTransform
Function Description:
This function performs Run Length Encoding and returns the encoded string
Function Return:
string: RLE string
*/
    std::string encoded;
    for (int i = 0; i < bwt.length(); ++i)
    {
        char current = bwt[i];
        int count = 1;
        while (i + 1 < bwt.length() && bwt[i + 1] == current)
        {
            ++count;
            ++i;
        }
        encoded += current;
        encoded += std::to_string(count);
    }
    return encoded;
}

// Move-to-Front Transform
std::string moveToFrontTransform(const std::string &input)
{
    /*
Function Name: moveToFrontTransform
Function Arguments:
input: string input
Function Description:
This function performs move to front transform and returns the transformed string
Function Return:
string: MTFT string
*/
    std::list<char> mtfList;
    for (char c = 0; c < 256; ++c)
    {
        mtfList.push_back(c);
    }
    std::string result;
    for (char c : input)
    {
        auto it = std::find(mtfList.begin(), mtfList.end(), c);
        if (it != mtfList.end())
        {
            int position = std::distance(mtfList.begin(), it);
            result += char(position);
            mtfList.erase(it);
            mtfList.push_front(c);
        }
    }

    return result;
}

// Huffman Coding
struct HuffmanNode
{
    char data;
    int frequency;
    HuffmanNode *left;
    HuffmanNode *right;

    HuffmanNode(char data, int frequency) : data(data), frequency(frequency), left(nullptr), right(nullptr) {}
};

struct CompareNodes
{
    bool operator()(HuffmanNode *left, HuffmanNode *right)
    {
        return left->frequency > right->frequency;
    }
};

HuffmanNode *buildHuffmanTree(const std::string &input)
{
    /*
Function Name: buildHuffmanTree
Function Arguments:
input: string input
Function Description:
This function creates the Huffman tree to perform Huffman Coding for the compression
Function Return:
Huffman Node
*/
    // Calculate the frequency of each character in the input
    std::map<char, int> frequency_map;
    for (char c : input)
    {
        frequency_map[c]++;
    }

    // Create a priority queue of Huffman nodes
    std::priority_queue<HuffmanNode *, std::vector<HuffmanNode *>, CompareNodes> pq;

    // Create a leaf node for each character and add it to the priority queue
    for (const auto &pair : frequency_map)
    {
        pq.push(new HuffmanNode(pair.first, pair.second));
    }

    // Build the Huffman tree
    while (pq.size() > 1)
    {
        HuffmanNode *left = pq.top();
        pq.pop();
        HuffmanNode *right = pq.top();
        pq.pop();

        HuffmanNode *newNode = new HuffmanNode('\0', left->frequency + right->frequency);
        newNode->left = left;
        newNode->right = right;
        pq.push(newNode);
    }

    // The remaining node in the priority queue is the root of the Huffman tree
    return pq.top();
}

void buildHuffmanCodesHelper(HuffmanNode *root, const std::string &code, std::map<char, std::string> &huffman_codes)
{
    /*
Function Name: buildHuffmanCodesHelper
Function Arguments:
    root: Huffman Root node of the tree
    code: string of huffman codes
    huffman_code: map of huffman codes for the coded string
Function Description:
    This function Helps in building the codes for the huffman coding which are used in huffmanCoding Compression.
Function Return:
    N/A
*/
    if (root == nullptr)
    {
        return;
    }

    if (root->data != '\0')
    {
        huffman_codes[root->data] = code;
    }

    buildHuffmanCodesHelper(root->left, code + '0', huffman_codes);
    buildHuffmanCodesHelper(root->right, code + '1', huffman_codes);
}

std::map<char, std::string> buildHuffmanCodes(HuffmanNode *root)
{
    /*
Function Name: buildHuffmanCodes
Function Arguments:
    root: Huffman Root node of the tree
Function Description:
    This function builds the codes for the huffman coding which are used in huffmanCoding Compression.
Function Return:
    Map of huffmanCodes
*/
    std::map<char, std::string> huffman_codes;
    buildHuffmanCodesHelper(root, "", huffman_codes);
    return huffman_codes;
}

std::string huffmanEncode(const std::string &input, const std::map<char, std::string> &huffman_codes)
{
    /*
Function Name: huffmanEncode
Function Arguments:
   input: input string of the character for HuffmanCoding
   huffman_codes:
Function Description:
   This function builds the codes for the huffman coding which are used in huffmanCoding Compression.
Function Return:
   Map of huffmanCodes
*/
    std::string encoded_string;
    for (char c : input)
    {
        encoded_string += huffman_codes.at(c);
    }
    return encoded_string;
}

inline void bzipCompressBuffer(const int &px, const int &py, const int &pz, const int &ox, const int &oy, const int &oz, char ***cube_buffer, std::unordered_map<char, std::string> &tags)
{

    /*
Function Name: bzipCompressBuffer
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
   This function applies the BZIP2 compression of the input buffer of px,py,pz size.
Function Return:
   N/A
*/
    // steps of BZip2 compression algorithm

    // Flatten the 3D buffer into a single string
    std::string flattened;
    for (int z = 0; z < pz; ++z)
    {
        for (int y = 0; y < py; ++y)
        {
            for (int x = 0; x < px; ++x)
            {
                flattened += cube_buffer[z][y][x];
            }
        }
    }

    // Step 1 perform BWT
    std::string bwt_result = burrowsWheelerTransform(flattened);
    // step2 perform rle on bwt of the input string
    std::string rle_result = runLengthEncode(bwt_result);
    // step3 perform MFT(move to front transform) of rle string
    std::string mtf_result = moveToFrontTransform(rle_result);

    // step4 perform huffman encoding
    HuffmanNode *huffman_tree = buildHuffmanTree(mtf_result);
    std::map<char, std::string> huffman_codes = buildHuffmanCodes(huffman_tree);

    std::string huffman_result = huffmanEncode(mtf_result, huffman_codes);

    std::cout << huffman_result;
}

int main()
{
    // Read and store header information
    int bx, by, bz;
    int px, py, pz;
    char nada;
    // get the input size of the block and parent size of blocks.
    std::cin >> bx >> nada >> by >> nada >> bz >> nada >> px >> nada >> py >> nada >> pz >> std::ws;

    std::string line;
    char symbol;
    std::string label;
    std::unordered_map<char, std::string> tags;
    // Get the map of the input characters
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
#pragma omp parallel for
    while (true)
    {
        // get the input to buffer
        readCubeIntoBuffer(px, py, pz, bx, by, bz, ox, oy, oz, cube_buffer, start);
        // compress the input of the buffer
        bzipCompressBuffer(px, py, pz, ox, oy, oz, cube_buffer, tags);

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

    // Free up cube buffer
    freeCubeBuffer(cube_buffer, py, pz);
    return 0;
}