import argparse
import sys
import os
import io
import subprocess
import time
import timeit
import tempfile
import typing
import numpy as np
import pandas as pd

def get_header_info_of_file1(file1: io.IOBase, line: str, line_count: int, stderr) -> typing.List[int]:
    try:
        line = file1.readline()
        if line == "":
            print("Error: unexpected end of input on line {}".format(
                line_count),
                file=stderr)
            sys.exit(100)
    except OSError as err:
        print("Error: failed to read line {} ({})".format(
            line_count,
            err.strerror),
            file=stderr)
        sys.exit(200)
    except:
        print("Error: failed to read line {} ({})".format(
            line_count,
            sys.exc_info()[0]),
            file=stderr)
        sys.exit(300)
    return parse_line1_of_file1(line, line_count, stderr)

def parse_line1_of_file1(line: str, line_count: int, stderr) -> typing.List[int]:
    try:
        ints = [int(i) for i in line.split(',')]
    except ValueError as err:
        print("line {}: {}Error: {}".format(
            line_count,
            line,
            err),
            file=stderr)
        sys.exit(1)
    if len(ints) != 6:
        print("line {}: {}Error: expecting line to be six integers of the format:".format(
            line_count,
            line),
            file=stderr)
        print("<x_count>, <y_count>, <z_count>, <x_parent_size>, <y_parent_size>, <z_parent_size>",
            file=stderr)
        sys.exit(1)
    return ints

def read_next_line_of_input(file, line_count, stderr):
    try:
        while True:
            line = file.readline()
            line_count += 1
            if len(line) == 0:
                break
            elif line[0] == '#' or line[0] == '\n' or line[0] == '\r':
                # Ignore comment lines and blank lines
                continue
            else:
                break
    except OSError as err:
        print("Error: failed to read line {} ({})".format(
            line_count,
            err.strerror),
            file=stderr)
        sys.exit(200)
    except:
        print("Error: failed to read line {} ({})".format(
            line_count,
            sys.exc_info()[0]),
            file=stderr)
        sys.exit(300)
    return line, line_count

def file_format_error_and_exit(line: str, line_count: int, stderr):
    print("line {}: {}Error: expecting format '{}'".format(
        line_count,
        line,
        "x, y, z, sx, sy, sz, string"),
        file=stderr)
    sys.exit(1)

def parse_and_check_line_of_file1(x: int, y: int, z:int, line: str, line_count: int, stderr, domains: typing.Dict):
    last_comma = line.rfind(",")
    if last_comma == -1:
        file_format_error_and_exit(line, line_count, stderr)
    domain = line[last_comma+1:].strip("' \n\r")
    domains[domain] = domains.get(domain, 0) + 1
    try:
        ints = [int(i) for i in line[0:last_comma].split(',')]
    except ValueError as err:
        print("line {}: {}Error: {}".format(
            line_count,
            line,
            err),
            file=stderr)
        sys.exit(1)
    if ints[0] != x:
        print("line {}: {}Error: expected x index to be {} in format x,y,z,px,py,pz,string".format(
            line_count,
            line,
            x),
            file=stderr)
        sys.exit(1)
    if ints[1] != y:
        print("line {}: {}Error: expected y index to be {} in format x,y,z,px,py,pz,string".format(
            line_count,
            line,
            y),
            file=stderr)
        sys.exit(1)
    if ints[2] != z:
        print("line {}: {}Error: expected z index to be {} in format x,y,z,px,py,pz,string".format(
            line_count,
            line,
            z),
            file=stderr)
        sys.exit(1)
    if ints[3] != 1 or ints[4] != 1 or ints[5] != 1:
        print("line {}: {}Error: expecting unit sized blocks only".format(
            line_count,
            line),
            file=stderr)
        sys.exit(1)
    return ints[0], ints[1], ints[2], ints[3], ints[4], ints[5], domain

def parse_line(
    line: str,
    line_count: int,
    parent_size_x: int,
    parent_size_y: int,
    parent_size_z: int,
    stderr):
    last_comma = line.rfind(",")
    if last_comma == -1:
        file_format_error_and_exit(line, line_count, stderr)
    domain = line[last_comma+1:].strip("' \n\r")
    try:
        ints = [int(i) for i in line[0:last_comma].split(',')]
    except ValueError as err:
        print("line {}: {}Error: {}".format(
            line_count,
            line,
            err),
            file=stderr)
        sys.exit(1)
    if ints[3] < 0 or ints[4] < 0 or ints[5] < 0:
        print("line {}: {}Error: expecting positive block sizes only".format(
            line_count,
            line),
            file=stderr)
        sys.exit(1)
    if ints[3] > parent_size_x or ints[4] > parent_size_y or ints[5] > parent_size_z:
        print("line {}: {}Error: block is too large".format(
            line_count,
            line),
            file=stderr)
        sys.exit(1)
    return ints[0], ints[1], ints[2], ints[3], ints[4], ints[5], domain

def parse_line_domain_only(line: str, line_count: int, stderr):
    return line[line.rfind(",")+1:].strip("' \n\r")

def run_and_time_reference_implementation() -> float:
    tic = timeit.default_timer()
    completed_process = subprocess.run(command_line_for_reference_implementation)
    toc = timeit.default_timer()
    if completed_process.returncode != 0:
        print("Baseline speed measurement process failed", file=stderr)
        sys.exit(13)
    return toc - tic

def run_and_time_item_under_test() -> float:
    tic = timeit.default_timer()
    completed_process = subprocess.run(command_line_for_item_under_test)
    toc = timeit.default_timer()
    if completed_process.returncode != 0:
        print("Error: {} exited with exit status {}.".format(
            args.path_to_item_under_test,
            completed_process.returncode),
            file=stderr)
        print("Console showing stderr looks like...\n> {}".format(
            " ".join(command_line_for_item_under_test[:-4])),
            file=stderr)
        with open("{}/err.txt".format(tmpdir), "r") as errors:
            print(errors.read(), file=stderr)
        sys.exit(14)
    return toc - tic

parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    epilog=
        "Tests a Python script or executable under the measurement rules of\n"
        "the specification given in Block Model Compression Algorithm\n"
        "(Software Engineering Project 2023, Semester 2) using the provided\n"
        "block model .csv file as input. The item under test is run in its own\n"
        "process with the input .csv file provided to it on standard input.\n"
        "Standard output from this process is then checked for correctness and\n"
        "block model equivalence to the input file. Upon successful completion\n"
        "this script returns with a zero exit status and the compression\n"
        "achieved will appear on standard output as a floating point number\n"
        "between 0 and 1 followed by a newline. (0 means no compression, 0.25\n"
        "means 25%% compression ... so higher is better.)\n"
        "\n"
        "As soon as any error is encountered this script instead returns with\n"
        "a non-zero exit status and diagnostic information is returned on\n"
        "standard error. Note that if multiple errors are present in the output\n"
        "the diagnostic information is only going the report the first one.\n"
        "\n"
        "Additionally, optional argument -s will also measure the speed of the\n"
        "item under test. Speed is measured relative to a reference\n"
        "implementation (a separate process performing an zero compression\n"
        "conversion of the input format to the output format).\n"
        "Upon successful completion the output and exit status is as before\n"
        "except that the measured speed will appear on standard output\n"
        "following the compression value, as a floating point number followed\n"
        "by a newline. (0.2 means five times slower than the direct copy,\n"
        "2.0 means twice as fast ... so higher is better.)\n"
        "\n"
        "Additionally, -r RUNTIME can be specified BEFORE -s to ensure a\n"
        "minimum running time of RUNTIME seconds by doing repeated runs and\n"
        "averaging them for a more accurate measurement of speed.\n"
        "If RUNTIME is not specified with -r, it defaults to 60.\n"
        "\n"
        "Additionally, a -v option can be specified for more verbose output\n"
        "showing what this script is up to."
)
parser.add_argument(
    "path_to_item_under_test",
    metavar="<my_algorithm.py or my_algorithm.exe>",
    help="script or executable to run")
parser.add_argument( 
    "path_to_block_model_input_csv",
    metavar="<input_block_model.csv>",
    help="input block model csv to pass to it on stdin")
parser.add_argument(
    "-r", 
    "--runtime",
    nargs='?',
    type=int,
    const=60,
    default=1000000,
    help="specify a minimum runtime in seconds, used with -s to improve accuracy")
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
args = parser.parse_args()
strnull = open(os.devnull, 'w')
stdout = sys.stdout
stderr = sys.stderr
stdvout = sys.stderr if args.verbose else strnull

if not os.path.isfile(args.path_to_item_under_test):
    print("Script or execuitable not found at '{}'".format(
        args.path_to_item_under_test),
        file=stderr)
    sys.exit(10)
if not (
    args.path_to_item_under_test.endswith(".py") or
    args.path_to_item_under_test.endswith(".exe")):
    print("'{}' must be a Python (.py) or executable (.exe) file".format(
        args.path_to_item_under_test),
        file=stderr)
    sys.exit(11)
if not os.path.isfile(args.path_to_block_model_input_csv):
    print("Input block model file not found at '{}'".format(
        args.path_to_block_model_input_csv),
        file=stderr)
    sys.exit(12)

with tempfile.TemporaryDirectory() as tmpdir:
    print("Using temporary directory '{}'".format(tmpdir),file=stdvout)
    command_line_for_reference_implementation = [
        "cmd.exe",
        "/c",
        r"2023_block_coalescing_reference_implementation.exe",
        "<", args.path_to_block_model_input_csv,
        ">", "{}/ref.csv".format(tmpdir),
        "2>", "{}/err.txt".format(tmpdir),
        ]
    command_head = ["cmd.exe", "/c"]
    if args.path_to_item_under_test.endswith(".py"):
        command_body = [sys.executable, args.path_to_item_under_test]
    else:
        command_body = [args.path_to_item_under_test]
    command_tail = [
        "<", args.path_to_block_model_input_csv,
        ">", "{}/out.csv".format(tmpdir),
        "2>", "{}/err.txt".format(tmpdir)]
    command_line_for_item_under_test = command_head + command_body + command_tail

    print("Running reference implementation...", file=stdvout)
    ref_time = run_and_time_reference_implementation()
    print("...done in {:.3f}s".format(ref_time), file=stdvout)
    print("Running the item under test...", file=stdvout)
    test_time = run_and_time_item_under_test()
    print("...done in {:.3f}s".format(test_time), file=stdvout)

    print("Analysing the output of the item under test...", file=stdvout)
    tic = timeit.default_timer()
    with open(args.path_to_block_model_input_csv, "r") as file1:
        line_f1 = ""
        line_count_f1 = 1
        ints = get_header_info_of_file1(file1, line_f1, line_count_f1, stderr)
    x_count = ints[0]
    y_count = ints[1]
    z_count = ints[2]
    block_count_f1 = x_count * y_count * z_count
    parent_size_x = ints[3]
    parent_size_y = ints[4]
    parent_size_z = ints[5]
    print("...looking for {} blocks arranged in {} chunk{} measuring {}x{}x{}".format(
        x_count * y_count * z_count,
        z_count // parent_size_z,
        "s, each" if z_count // parent_size_z > 0 else "",
        x_count,
        y_count,
        parent_size_z),
        file=stdvout)

    if block_count_f1 < 50000000:
        # We run a slow checker that can produce precise output error diagnostics
        with open("{}/ref.csv".format(tmpdir), "r") as file1, \
            open("{}/out.csv".format(tmpdir), "r") as file2:
            line_f1 = ""
            line_count_f1 = 1
            # We have sufficient info to start read in the first chunk of file2.
            # A chunk is all the blocks in a parent_size_z run of consecutive slices in z.
            line_count_f2 = 0
            line_f2, line_count_f2 = read_next_line_of_input(file2, line_count_f2, stderr)
            if line_f2 == "":
                print("Error: unexpected end of input on line {}".format(
                    line_count_f2),
                    file=stderr)
                sys.exit(100)
            line_count_f2_last_chunk_end = 0
            block_count_f2 = 0
            chunk_array_size = x_count * y_count * parent_size_z
            for z in range(0, z_count, parent_size_z):
                idx = 0
                print("...reading chunk {} of {}".format(
                    z // parent_size_z + 1, z_count // parent_size_z),
                    file=stdvout)
                dtype = [('x', int), ('y', int), ('z', int), ('l', int), ('d', np.dtype('a16'))]
                chunk_array = np.empty(chunk_array_size, dtype=dtype)
                while True:
                    x2, y2, z2, xs2, ys2, zs2, domain = parse_line(
                        line_f2,
                        line_count_f2,
                        parent_size_x,
                        parent_size_y,
                        parent_size_z,
                        stderr)
                    if z2 < z:
                        # This block is from an earlier chunk and we mandate chunkwise
                        # processing => format error.
                        print("{}: {}Error: block z value lies in an earlier chunk, expected before line {}.".format(
                            line_count_f2,
                            line_f2,
                            line_count_f2_last_chunk_end - 1),
                            file=stderr)
                        sys.exit(1)
                    elif x2 // parent_size_x != (x2 + xs2 - 1) // parent_size_x:
                        print("{}: {}Error: block crosses a parent block boundary in the x direction.".format(
                            line_count_f2,
                            line_f2),
                            file=stderr)
                        sys.exit(1)
                    elif y2 // parent_size_y != (y2 + ys2 - 1) // parent_size_y:
                        print("{}: {}Error: block crosses a parent block boundary in the y direction.".format(
                            line_count_f2,
                            line_f2),
                            file=stderr)
                        sys.exit(1)
                    elif z2 // parent_size_z != (z2 + zs2 - 1) // parent_size_z:
                        print("{}: {}Error: block crosses a parent block boundary in the z direction.".format(
                            line_count_f2,
                            line_f2),
                            file=stderr)
                        sys.exit(1)
                    elif z2 > z + parent_size_z - 1:
                        # This block is from the next chunk, so don't process it yet
                        # and assume we have come to the end of the current chunk.
                        line_count_f2_last_chunk_end = line_count_f2
                        break
                    # Explode the block into sub-blocks and add each to the chunk list
                    block_count_f2 += 1
                    for sz in range(zs2):
                        for sy in range(ys2):
                            for sx in range(xs2):
                                if idx == chunk_array_size:
                                    idx += 1
                                    break
                                chunk_array[idx][0] = x2+sx
                                chunk_array[idx][1] = y2+sy
                                chunk_array[idx][2] = z2+sz
                                chunk_array[idx][3] = line_count_f2
                                chunk_array[idx][4] = domain
                                idx += 1
                    # Now grab the next line and continue the loop
                    line_f2, line_count_f2 = read_next_line_of_input(file2, line_count_f2, stderr)
                    if line_f2 == "":
                        # We have reached the end of the file and now should process
                        # the last chunk.
                        break
                if idx < chunk_array_size:
                    print("Error: {} specifies too few blocks for the chunk ending at line {}.".format(
                        file2.name,
                        line_count_f2),
                        file=stderr)
                    sys.exit(1)
                elif idx > chunk_array_size:
                    print("Error: {} specifies too many blocks for the chunk at line {}.".format(
                        file2.name,
                        line_count_f2),
                        file=stderr)
                    sys.exit(1)
                # Sort the data by x, y and z in that order:
                print("...sorting chunk {} of {}".format(
                    z // parent_size_z + 1, z_count // parent_size_z),
                    file=stdvout)
                chunk_array.sort(order=['z','y','x'])
                # Now compare it line by line with file1 over the slice range z:z+zsf to check domain correctness
                print("...checking chunk {} of {}".format(
                    z // parent_size_z + 1, z_count // parent_size_z),
                    file=stdvout)
                for e in chunk_array[:]:
                    line_f1, line_count_f1 = read_next_line_of_input(
                        file1,
                        line_count_f1,
                        stderr)
                    if line_f1 == "":
                        print("Error: {} specifies more blocks than {} from line {} onwards.".format(
                            file2.name,
                            file1.name,
                            e[3]),
                            file=stderr)
                        sys.exit(1)
                    domain1 = parse_line_domain_only(
                        line_f1,
                        line_count_f1,
                        stderr)
                    if e[4] != domain1.encode('ascii'):
                        print("Error: '{}' block on line {} should be '{}'.".format(
                            e[4],
                            e[3],
                            domain1),
                            file=stderr)
                        sys.exit(1)
    else:
        print("Error: checking of files > 50000000 blocks is NYI.", file=stderr)
        sys.exit(1)
    toc = timeit.default_timer()
    print("...done in {:.3f}s".format(toc-tic), file=stdvout)
    print("...equivalence = 100%", file=stdvout)
 
    if args.speed:
        if block_count_f1 < 1000000:
            print("Warming up IO ...", file=stdvout)
            warmup_time = run_and_time_reference_implementation()
            print("...done in {:.3f}s".format(warmup_time), file=stdvout)
        reference_times = []
        item_under_test_times = []
        item_under_test_times.append(test_time)
        start = timeit.default_timer()
        while True:
            print("Measuring reference implementation speed...", file=stdvout)
            reference_times.append(run_and_time_reference_implementation())
            print("...done in {:.3f}s".format(reference_times[-1]), file=stdvout)
            if args.runtime == 1000000:
                # This is the command line value when -r is NOT specified
                break
            elif timeit.default_timer() > start + args.runtime:
                # We've burnt through our speed testing time
                break
            print("Running the item under test...", file=stdvout)
            print('..."{}"'.format(" ".join(command_line_for_item_under_test[0:-4])), file=stdvout)
            item_under_test_times.append(run_and_time_item_under_test())
            print("...done in {:.3f}s".format(item_under_test_times[-1]), file=stdvout)

# The temporary directory should be gone by the time we get here.
print("-----------", file=stdvout)
compression = (block_count_f1 - block_count_f2) / block_count_f1
print("Compression = {:6.2f}%  ({} blocks down from {} blocks)".format(
    compression*100,
    block_count_f2,
    block_count_f1), file=stdvout)
if args.speed:
    average_std2std_time = sum(reference_times) / len(reference_times)
    average_item_under_test_time = sum(item_under_test_times) / len(item_under_test_times)
    speed = average_std2std_time / average_item_under_test_time
    print("      Speed = {:6.2f}%  ({:.0f} blocks per second, reference exe is {:.0f}, runs = {})".format(
        speed*100,
        block_count_f1/average_item_under_test_time,
        block_count_f1/average_std2std_time,
        len(reference_times)),
        file=stdvout)
print("-----------", file=stdvout)
print("exit status is 0", file=stdvout)
print("stdout is:", file=stdvout)

# Print final results to stdout
print("{}".format(compression), file=stdout)
if args.speed:
    print("{}".format(speed), file=stdout)
