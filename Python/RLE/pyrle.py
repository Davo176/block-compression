import sys
import threading

# Create a mutex
print_mutex = threading.Lock()

# Function Name: run_length_encode
# Function Arguments:
#     substring: The substring to encode
#     x: The starting x coordinate
#     y: The starting y coordinate
#     z: The starting z coordinate
#     tag_table: The dictionary of tags
# Function Description:
#     Given a substring runs RLE and prints the output
# Function Return:
#     Name: None
def run_length_encode(substring, x,y,z, tag_table):
    count = 1
    prev_char = substring[0]
    for char in substring[1:]:
        if char == prev_char:
            count += 1
        else:
            label = tag_table[prev_char]
            with print_mutex:  # Acquire the mutex before printing
                print(f"{x},{y},{z},{count},1,1,{label}")
            x += count
            count = 1
            prev_char = char
    label = tag_table[prev_char]
    with print_mutex:  # Acquire the mutex before printing
        print(f"{x},{y},{z},{count},1,1,{label}")

# Function Name: process_lines
# Function Arguments:
#     lines: A list of all the input lines
#     no_lines: The total number of lines
#     start_line: The line to start encoding from
#     iterator: How many lines to skip
#     tag_table: The dictionary of tags
# Function Description:
#     Given all the input file lines calls run length encoding for each substring.
# Function Return:
#     Name: None
def process_lines(lines, no_lines, start_line, iterator, tag_table):
    z = 0
    y = start_line
    for current_line in range(start_line, no_lines, iterator):
        line = lines[y].strip()
        x = 0
        if line == '':
            z += 1
            y = 0
            continue
        for i in range(0, len(line), px):
            substring = line[i:i + px]
            run_length_encode(substring, x, y, z, tag_table)
            x += px
        y += iterator

if __name__ == '__main__':
    # Read the first line containing dimensions
    bx, by, bz, px, py, pz = map(int, input().strip().split(','))

    # Read the tag table
    tag_table = {}
    while True:
        line = input().strip()
        if not line:
            break
        tag, label = line.split(', ')
        tag_table[tag] = label

    lines = list(sys.stdin)
    no_lines = len(lines)

    process_lines(lines=lines,no_lines=no_lines,start_line=0,iterator=1,tag_table=tag_table)
