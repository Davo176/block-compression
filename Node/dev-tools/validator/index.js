const fs = require("fs");

let map = [[]];
const tag_table = {};
const reverse_tag_table = {};
let x_count;
let y_count;
let z_count;
let parent_x;
let parent_y;
let parent_z;

let errors = [];
//read in input file, add to 3d array for reference later
//look at greedy alg for comments on reading input in.
fs.readFile(process.argv[2], "utf8", (err, data) => {
    if (err) {
        console.log(err);
    }
    const file = data.split("\n");

    [x_count, y_count, z_count, parent_x, parent_y, parent_z] = file
        .shift()
        .split(",")
        .map((e) => parseInt(e));

    let reached_data = false;
    let x_index = -1;
    let y_index = -1;
    let z_index = 0;
    for (let line of file) {
        line = line.trim();
        if (!line && !reached_data) {
            reached_data = true;
            continue;
        }
        if (reached_data) {
            if (!line) {
                z_index += 1;
                map.push([]);
                y_index = -1;
                continue;
            } else {
                y_index += 1;
                map[z_index].push([]);
            }
            x_index = -1;
            for (let char of line) {
                x_index += 1;
                map[z_index][y_index].push([]);
                map[z_index][y_index][x_index] = tag_table[char];
            }
        } else {
            tag_table[line.split(", ")[0]] = line.split(", ")[1];
            reverse_tag_table[line.split(", ")[1]] = line.split(", ")[0];
        }
    }
    //then read in output file
    fs.readFile(process.argv[3], "utf8", (err, data) => {
        //track line number, and how many blocks processed
        //num lines is how many blocks you compressed to
        //running num blocks is total blocks originally.
        let line_no = 0;
        let running_number_of_blocks = 0;
        if (err) {
            console.log(err);
        }
        const file1 = data.split("\n");
        let curr_z_block = 0; //track what z chunk im up to
        while (file1.length > 0) {
            line = file1[0];
            if (!line) {
                break;
            }
            line_no++;
            line = line.trim();
            const [x_coord, y_coord, z_coord, x_size, y_size, z_size, label] = file1
                .shift()
                .trim()
                .split(",")
                .map((e) => (parseInt(e) >= 0 ? parseInt(e) : e));
            running_number_of_blocks += x_size * y_size * z_size;
            //if youve outputted a block from a previous chunk, add an error.
            if (Math.floor(z_coord / parent_z) < curr_z_block) {
                errors.push(`Line ${line_no} is outputted too late, you have already started on another parent block thickness!`);
            }
            //track current z chunk yp to
            curr_z_block = Math.floor(z_coord / parent_z);
            //for your block on this line
            for (let z = 0; z < z_size; z++) {
                for (let y = 0; y < y_size; y++) {
                    for (let x = 0; x < x_size; x++) {
                        //check the points match
                        if (map[z_coord + z][y_coord + y][x_coord + x] !== label) {
                            //points get marked as -1 when visited, so if it's -1 then youve revisited it,
                            //this gives a more detailed error.
                            if (map[z_coord + z][y_coord + y][x_coord + x] === -1) {
                                errors.push(`Line ${line_no} revisits a node at x=${x_coord + x},y=${y_coord + y},z=${z_coord + z}`);
                                errors.push(`(Move errored line to top for quick way to find conflicting line)`);
                            } else {
                                errors.push(`Label Does not match on line ${line_no} expected ${map[z_coord + z][y_coord + y][x_coord + x]} got ${label}`);
                            }
                        }
                        //use modulus maths to make sure you havent gone over a parent block dimension
                        if (x_coord % parent_x >= (x_coord + x) % parent_x && x !== 0) {
                            errors.push(`Line ${line_no} has gone over the bounds of a parent block in the x direction`);
                        }
                        if (y_coord % parent_y >= (y_coord + y) % parent_y && y !== 0) {
                            errors.push(`Line ${line_no} has gone over the bounds of a parent block in the y direction`);
                        }
                        if (z_coord % parent_z >= (z_coord + z) % parent_z && z !== 0) {
                            errors.push(`Line ${line_no} has gone over the bounds of a parent block in the z direction`);
                        }
                        map[z_coord + z][y_coord + y][x_coord + x] = -1; //let us know we have been here
                    }
                }
            }
        }
        let num_1_unit_blocks = x_count * y_count * z_count;
        //if you didnt print out the right number of blocks
        if (running_number_of_blocks !== num_1_unit_blocks) {
            //add error
            errors.push(`Incorrect number of blocks outputted. Expected ${num_1_unit_blocks} got ${running_number_of_blocks}`);
        }
        //otherwise, if no errors, you did it!
        if (errors.length === 0) {
            //print out compression rate.
            console.log("Success");
            console.log(`Compressed ${num_1_unit_blocks} blocks into ${line_no} blocks`);
            console.log(`Compression rate = ${((num_1_unit_blocks - line_no) / num_1_unit_blocks) * 100}`);
        } else {
            //otherwise, print out all your errors
            errors.map((err) => {
                console.log(err);
            });
        }
    });
});
