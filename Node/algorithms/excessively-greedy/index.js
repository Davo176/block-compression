const { run_greedy } = require("./greedy-functions");
const readline = require("readline");
const { optimiseBuckets, getOptions } = require("./utils");

const tag_table = {};
let x_count;
let y_count;
let z_count;
let parent_x;
let parent_y;
let parent_z;

//parsing from a stream in js was not something
//man was meant to ever accomplish.

//need to set up a readline interface..
const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout,
});

//define global variables i need to access within
//my callback function that runs on each new line.
let cube = [];
let done_first_line = false;
let passed_tag_table = false;
let x_index = -1;
let y_index = -1;
let z_index = 0;
let total_zs = 0;
let greedy_config;

//JS reading from a stream is different. I need to give a callback on each new line
//rather than other languages where you just request next line.
//how they do this under the hood.. god knows
rl.on("line", (line) => {
    //if we havent handled the first line
    if (!done_first_line) {
        //parse it, and store variables
        [x_count, y_count, z_count, parent_x, parent_y, parent_z] = line.split(",").map((e) => parseInt(e));
        done_first_line = true; //weve done it now

        //configure some stuff for greedy algorithms
        greedy_config = {
            options: getOptions(parent_x, parent_y, parent_z),
        };

        //any other algorithm configurations can be put in here
        //anything that needs to happen once

        //configure the cube (this is x_count by y_count by parent_z sized.)
        //just need one cube, can overwrite it after its compressed
        //to save memory.
        for (let z = 0; z < parent_z; z++) {
            let yi = [];
            for (let y = 0; y < y_count; y++) {
                let xi = [];
                for (let x = 0; x < x_count; x++) {
                    xi.push(-1);
                }
                yi.push(xi);
            }
            cube.push(yi);
        }
    }
    //remove new line chars
    line = line.trim();
    //if our line is null, and we havent passed the tag table
    if (!line && !passed_tag_table) {
        passed_tag_table = true; //now we have passed tag table
        return; //break out of callback
    }
    //if we are passed the tag table
    if (passed_tag_table) {
        //if line is empty
        if (!line) {
            //up to next z layer
            z_index += 1;
            y_index = -1;
            total_zs += 1;
            //if the z_index we are up to is the same as parent z
            //we have finished the cube
            if (z_index === parent_z) {
                //reset z
                z_index = 0;
                //compress our cube
                compressParentCube(cube, parent_x, parent_y, parent_z, 0, total_zs - parent_z, greedy_config);
            }
            return;
        } else {
            //otherwise at next y layer
            y_index += 1;
        }
        x_index = -1;
        //then loop over all chars in the line, and add corresponding
        //material to our map, based interpreting from tag table
        for (let char of line) {
            x_index += 1;
            cube[z_index][y_index][x_index] = tag_table[char];
        }
        //if we arent passed the tag table
    } else {
        //parse the line, and add to tag table
        tag_table[line.split(", ")[0]] = line.split(", ")[1];
    }
});

function compressParentCube(cube, parent_x, parent_y, parent_z, z_beta, z_offset, greedy_config) {
    for (let y_beta = 0; y_beta < y_count; y_beta += parent_y) {
        for (let x_beta = 0; x_beta < x_count; x_beta += parent_x) {
            //Run all configurations of our Greedy
            let greedy_results = run_greedy(cube, x_beta, y_beta, z_beta, parent_x, parent_y, parent_z, greedy_config.options);

            //Any other algorithms you would want to run would go here.
            //then concat the buckets returned

            //note that greedy_results here is already an array of buckets
            let buckets = optimiseBuckets(greedy_results);
            //print out the best of everything for that parent chunk.
            console.log(
                buckets
                    .map((b) => {
                        //because the algorithms think they are doing the first chunk
                        //artificially add back on the actual chunk we are doing.
                        let blocks = b.blocks;
                        blocks = blocks.map((b) => {
                            b[2] = b[2] + parseInt(z_offset);
                            return b;
                        });
                        return blocks.join("\n");
                    })
                    .join("\n")
            );
        }
    }
}
