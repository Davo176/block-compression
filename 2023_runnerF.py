import argparse
import sys
import os
import random
import subprocess
import threading
import timeit
import numpy as np
import msvcrt

def printerr(*args, **kwargs):
    print(*args, **kwargs, file=sys.stderr)

def printlog(*args, **kwargs):
    if ARGS.verbose: print(*args, **kwargs, file=sys.stderr)

def kill_pipeline_and_set_flag(flag):
    cleanup_subprocesses()
    flag[0] = True

def cleanup_subprocesses():
    # Try and clean up subprocesses. We only bother with the test pipeline
    # because the broken pipe on exit will take care of the reference stream.
    if 'processP' in globals(): processP.kill()  # Try and kill the pipeline by taking out the generator
    if 'processT' in globals(): processT.kill()  # Then try and kill the item under test

def close_files():
    if 'stderrT' in globals(): stderrT.close()

def printerr_item_under_test_stderr():
    if os.path.isfile(STDERR_FILE) and os.path.getsize(STDERR_FILE) > 0:
        printerr("{} has stderr as:".format(ARGS.path_to_item_under_test))
        with open(STDERR_FILE, 'r') as file:
            for line in file:
                print(line, end='')

def cleanup_and_exit(exit_code: int):
    cleanup_subprocesses()
    close_files()
    printerr("exit status is {}".format(exit_code))
    printerr_item_under_test_stderr()
    exit(exit_code)

def print_final_stats_and_exit(uncompressed_count, compressed_count):
    cleanup_subprocesses()
    close_files()
    # Print summary info to logs
    if ARGS.verbose:
        printerr_item_under_test_stderr()
    printlog("-----------")
    compression = (uncompressed_count - compressed_count) / uncompressed_count
    printlog("Compression = {:6.2f}%  ({} blocks down from {} blocks)".format(
        compression*100,
        compressed_count,
        uncompressed_count))
    if ARGS.speed:
        speed = uncompressed_count/ARGS.runtime
        printlog("      Speed = {:.0f} blocks per second ({} blocks streamed in {}s)".format(
            speed,
            uncompressed_count,
            ARGS.runtime))
    printlog("-----------")
    printlog("exit status is 0")
    # Print final results to stdout
    print("{}".format(compression))
    if ARGS.speed:
        print("{}".format(speed))
    exit(0)

def file_format_error_and_exit(line: str, line_count: int):
    printerr("line {}: {}Error: expecting format '{}'".format(
        line_count,
        line,
        "x, y, z, sx, sy, sz, string"))
    cleanup_and_exit(1)

def parse_line(
    line: str,
    line_count: int,
    parent_size_x: int,
    parent_size_y: int,
    parent_size_z: int,
    do_checks: bool):
    tag = line[-2]
    try:
        ints = [int(i) for i in line[:-3].split(',')]
    except ValueError as err:
        printerr("line {}: {}Error: {}".format(
            line_count,
            line,
            err))
        cleanup_and_exit(1)
    if do_checks:
        if ints[3] < 0 or ints[4] < 0 or ints[5] < 0:
            printerr("line {}: {}Error: expecting positive block sizes only".format(
                line_count,
                line))
            cleanup_and_exit(1)
        if ints[3] > parent_size_x or ints[4] > parent_size_y or ints[5] > parent_size_z:
            printerr("line {}: {}Error: block is too large".format(
                line_count,
                line))
            cleanup_and_exit(102)
    return ints[0], ints[1], ints[2], ints[3], ints[4], ints[5], tag

parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    epilog=
        "Tests a Python script or executable under the measurement rules of\n"
        "the specification given in Block Model Compression Algorithm\n"
        "(Software Engineering Project 2023, Semester 2) using a procedurally\n"
        "generated block model as input. The item under test is run in its own\n"
        "process with the stream provided to it on standard input.\n"
        "Standard output from this process is then piped back to this process\n"
        "where it is checked for correctness and block model equivalence to the\n"
        "procedurally generated input. This pipeline is run for a specified\n"
        "period of time using -r upon which the compression the item under\n"
        "test achieves will appear on standard output as a floating point number\n"
        "between 0 and 1 followed by a newline. (0 means no compression, 0.25\n"
        "means 25%% compression ... so higher is better.)\n"
        "\n"
        "As soon as any error is encountered this script instead returns with\n"
        "a non-zero exit status and diagnostic information is returned on\n"
        "standard error. Note that if multiple errors are present in the output\n"
        "the diagnostic information is only going the report the first one.\n"
        "\n"
        "Additionally, optional argument -s will also measure the speed of the\n"
        "item under test. Speed is measured as the throughput in blocks per\n"
        "of the overall processing pipeline. Upon successful completion the\n"
        "output and exit status is as before except that the measured speed\n"
        "will appear on standard output following the compression value, as\n"
        "a floating point number followed by a newline. Higher is better.\n"
        "\n"
        "Additionally, -r RUNTIME can be specified to change the overall\n"
        "runtime. A longer runtime will generally result in less fluctuation\n"
        "in the calculation of both compression and speed. If RUNTIME is not\n"
        "specified with -r, it defaults to 300 (five minutes).\n"
        "\n"
        "Additionally, -c CHECKTIME can be specified to change the rate of\n"
        "output checking. The default of 0 will check all the output at the\n"
        "cost of slowing down the overall pipeline and potentially having the\n"
        "checking as the rate limiting stage. By setting CHECKTIME to 60\n"
        "then checking is only performed on a chunks of data every minute,\n"
        "substantially increasingly throughput.\n"
        "\n"
        "Additionally, a -v option can be specified for more verbose output\n"
        "showing what this script is up to."
)
parser.add_argument(
    "path_to_item_under_test",
    metavar="<my_algorithm.py or my_algorithm.exe>",
    help="script or executable to run")
parser.add_argument( 
    "path_to_input_generator",
    metavar="<input_generator.py>",
    help="Python script whose output is piped to it")
parser.add_argument(
    "-r", 
    "--runtime",
    nargs='?',
    type=int,
    const=60,
    default=1000000,
    help="specify a minimum runtime in seconds, used with -s to improve accuracy")
parser.add_argument(
    "-c", 
    "--checktime",
    nargs='?',
    type=int,
    const=0,
    default=0,
    help="specify a checking interval in seconds instead of checking all output")
parser.add_argument(
    "-s", 
    "--speed",
    help="output speed in addition to compression",
    action="store_true")
parser.add_argument(
    "-v",
    "--verbose",
    help="print additional output to see details and progress",
    action="store_true")
ARGS = parser.parse_args()

# File where the item under test's stderr is dumped
STDERR_FILE = 'stderr.txt'
# If the item under test hasn't streamed any output after this amount of time, it will be killed
MAX_PIPELINE_YIELD_TIME = 60
# Get Python script to procedurally generate block model data from the command line
GENERATOR_PATH = ARGS.path_to_input_generator
# Randomise generator output
SEED = random.getrandbits(32)
if not os.path.exists(ARGS.path_to_item_under_test):
    printerr("Error: File not found ({}).".format(ARGS.path_to_item_under_test))
    exit(2)
# Tricky: put PYTHONUNBUFFERED on the environment so that python
# subprocesses don't have to manually flush stdout 
os.environ["PYTHONUNBUFFERED"] = "1"
# Build command line for the reference stream (R)
command_lineR = [GENERATOR_PATH, "--seed", str(SEED)]
executableR = sys.executable  # The path to this script's Python interpretter
command_lineR.insert(0, '')   # No idea why this is required, but it stops working when not present!
# Build command line for the test stream (T)
#command_headT = ["cmd.exe", "/c", sys.executable, GENERATOR_PATH, "--seed", str(SEED), "|" ]
if ARGS.path_to_item_under_test.endswith(".py"):
    command_lineT = [ARGS.path_to_item_under_test]
    executableT = sys.executable
    command_lineT.insert(0, '')   # No idea why this is required, but it stops working when not present!
else:
    command_lineT = [ARGS.path_to_item_under_test]
    executableT = None
# Launch the reference stream
processR = subprocess.Popen(
    command_lineR,
    executable=executableR,
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    text=True)
# Launch the test stream, which is a pipeline of two processes
try:
    # First the same as the reference stream
    processP = subprocess.Popen(
        command_lineR,
        executable=executableR,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True)
    # But we now pipe it's stdout to the item under test's stdin
    stderrT = open(STDERR_FILE, 'w')
    processT = subprocess.Popen(
        command_lineT,
        executable=executableT,
        stdin=processP.stdout,
        stdout=subprocess.PIPE,
        stderr=stderrT,
        text=True)
except (OSError, ValueError, subprocess.SubprocessError) as exception:
    printerr("Error: Problems launching program under test:")
    printerr(exception)
    cleanup_and_exit(3)
# Show the header information from the reference stream
lineR = processR.stdout.readline()
ints = [int(i) for i in lineR.split(',')]
bx = ints[0]
by = ints[1]
bz = ints[2]
px = ints[3]
py = ints[4]
pz = ints[5]
printlog("bx={}, by={}, bz={}, px={}, py={}, pz={}".format(bx, by, bz, px, py, pz))
# Read through the tag table of the reference stream
while lineR:
    lineR = processR.stdout.readline()
    if lineR == '\n': break
# Set up measurement variables
line_countR = 0
block_countR = 0
line_countT = 0
compressed_block_countT = 0
decompressed_block_countT = 0
chunk_count = 0
# Initialise the present z range of interest, ie. the present chunk
bz_start = 0
bz_end = bz_start + pz
checking_this_chunk = True       # Always check the first chunk
chunk_buffer = np.empty((pz,by,bx), dtype=np.uint8)
# Setup a timer to kill off the test stream pipline if it just blocks on read
watchdog_flag = [False]
watchdog = threading.Timer(MAX_PIPELINE_YIELD_TIME, kill_pipeline_and_set_flag, args=[watchdog_flag])
watchdog.start()
# Setup a timer so that we can stop after the designated testing time
overall_tic = timeit.default_timer()
check_tic = overall_tic + ARGS.checktime
# Stream the test stream counting blocks as we go
lineT = processT.stdout.readline()
if watchdog_flag[0] == True:
    printerr("Error: {} failed to produce any output after {}s.".format(
        ARGS.path_to_item_under_test, MAX_PIPELINE_YIELD_TIME))
    printerr("Error: Maybe it is trying to read in the entire input stream.")
    printerr("Error: The stream is infinite so this is infeasible.")
    printerr("Error: You must read standard input incrementally.")
    cleanup_and_exit(99)
watchdog.cancel()
while True:
    # Read from the test stream until the first block in the next chunk is seen
    chunk_count += 1
    compressed_block_count_this_chunkT = 0
    decompressed_block_count_this_chunkT = 0
    printlog("Processing chunk {} ({})".format(
        chunk_count,
        "stream and check" if checking_this_chunk else "streaming only"))
    if checking_this_chunk:
        chunk_buffer[...] = 63  # ASCII question mark code
    while True:
        if lineT == "":
            printerr("Error: unexpected end of input after line {}".format(line_countT))
            cleanup_and_exit(100)
        xT, yT, zT, xsT, ysT, zsT, char = parse_line(lineT, line_countT, px, py, pz, checking_this_chunk)
        if zT >= bz_end:
            # This block is from the next chunk, so don't process it yet
            # and assume we have come to the end of the current chunk.
            break
        else:
            line_countT += 1
            compressed_block_count_this_chunkT += 1
            decompressed_block_count_this_chunkT += xsT*ysT*zsT
            if checking_this_chunk:
                if zT < bz_start:
                    # This block is from an earlier chunk and we mandate chunkwise
                    # processing => format error.
                    printerr("{}: {}Error: block z value lies in an earlier chunk.".format(
                        line_countT,
                        lineT))
                    cleanup_and_exit(1)
                elif xT // px != (xT + xsT - 1) // px:
                    printerr("{}: {}Error: block crosses a parent block boundary in the x direction.".format(
                        line_countT,
                        lineT))
                    cleanup_and_exit(1)
                elif yT // py != (yT + ysT - 1) // py:
                    printerr("{}: {}Error: block crosses a parent block boundary in the y direction.".format(
                        line_countT,
                        lineT))
                    cleanup_and_exit(1)
                elif zT // pz != (zT + zsT - 1) // pz:
                    print("{}: {}Error: block crosses a parent block boundary in the z direction.".format(
                        line_countT,
                        lineT))
                    cleanup_and_exit(1)
                # Explode the just-read block into the chunk_buffer
                chunk_buffer[zT-bz_start:zT-bz_start+zsT, yT:yT+ysT, xT:xT+xsT] = ord(char)
            # We're ready for the next line
            lineT = processT.stdout.readline()
            # Are we out of time yet?
            if line_countT % 1024 == 0 and timeit.default_timer() - overall_tic > ARGS.runtime:
                print_final_stats_and_exit(block_countR, compressed_block_countT)
    # We've finished reading the current chunk off the test stream.
    # Now do the same for the reference stream and check for correspondences if required
    for z in range(pz):
        for y in range(by):
            lineR = processR.stdout.readline()
            line_countR += 1
            block_countR += len(lineR) - 1
            if checking_this_chunk:
                if chunk_buffer[z,y].tobytes().decode("ascii") != lineR[:-1]:
                    printerr("Error: Block correspondence failed on slice {} column {}.".format(bz_start+z, y))
                    printerr("...correct line is:")
                    printerr(lineR[:-1])
                    printerr("...decompressed line is:")
                    printerr(chunk_buffer[z,y].tobytes().decode("ascii"))
                    cleanup_and_exit(1)
        lineR = processR.stdout.readline()
        line_countR += 1
        if lineR != '\n':
            printerr("Error: expected blank line at line {} of reference stream.".format(line_countR))
            printerr("Instead found '{}'".format(lineR))
    # Check decompression tally for this chunk
    if decompressed_block_count_this_chunkT != bx * by * pz:
        printerr("Codec error: stream expanded to {} blocks in chunk {} whereas {} is expected.".format(
            decompressed_block_count_this_chunkT,
            chunk_count,
            bx * by * pz))
        cleanup_and_exit(101)
    # Bring test stream block count tallies up-to-date with the reference stream
    compressed_block_countT += compressed_block_count_this_chunkT
    decompressed_block_countT += decompressed_block_count_this_chunkT
    # Log some stats
    printlog("... done. {} vs {}".format(block_countR, decompressed_block_countT))
    # Advance to the next chunk
    bz_start += pz
    bz_end += pz
    # Is it time to check the next chunk?
    if timeit.default_timer() > check_tic:
        checking_this_chunk = True
        check_tic += ARGS.checktime
    else:
        checking_this_chunk = False
