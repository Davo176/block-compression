const fs = require("fs");

let map = [[]];
const tag_table = {};
let x_count;
let y_count;
let z_count;
let parent_x;
let parent_y;
let parent_z;

//Parsing the input is identical to greedy.
//Read the comments there if anything isnt clear.
fs.readFile(0, "utf8", (err, data) => {
    if (err) {
        console.log(err);
    }
    const file = data.split("\n");

    [x_count, y_count, z_count, parent_x, parent_y, parent_z] = file
        .shift()
        .split(",")
        .map((e) => parseInt(e));

    let passed_tag_table = false;
    let x_index = -1;
    let y_index = -1;
    let z_index = 0;
    //parses input file
    for (let line of file) {
        line = line.trim();
        if (!line && !passed_tag_table) {
            passed_tag_table = true;
            continue;
        }
        if (passed_tag_table) {
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
                map[z_index][y_index].push(tag_table[char]);
            }
        } else {
            tag_table[line.split(", ")[0]] = line.split(", ")[1];
        }
    }
    //Add all cuboid sizes into an array
    //these are all the possible shapes that could be compressed to
    // / exported
    let options = [];
    for (let z = 1; z <= parent_z; z++) {
        for (let y = 1; y <= parent_y; y++) {
            for (let x = 1; x <= parent_x; x++) {
                options.push([x, y, z]);
            }
        }
    }
    //Sort them based of volume, largest to smallest
    options.sort((a, b) => {
        let a_mult = 1;
        for (let i = 0; i < a.length; i++) {
            a_mult *= a[i];
        }
        let b_mult = 1;
        for (let i = 0; i < b.length; i++) {
            b_mult *= b[i];
        }
        if (a_mult < b_mult) {
            return 1;
        }
        if (a_mult > b_mult) {
            return -1;
        }
        for (let j = 0; j < a.length; j++) {
            if (a[j] > b[j]) {
                return 1;
            }
            if (a[j] < b[j]) {
                return -1;
            }
        }
        return 0;
    });
    //Loop over all the parent blocks
    for (let z_beta = 0; z_beta < z_count; z_beta += parent_z) {
        for (let y_beta = 0; y_beta < y_count; y_beta += parent_y) {
            for (let x_beta = 0; x_beta < x_count; x_beta += parent_x) {
                //and then all the block sizes
                for (let option of options) {
                    //see if we can fit that block anywhere in the parent block
                    compressReverseGreedily(x_beta, y_beta, z_beta, parent_x, parent_y, parent_z, option[0], option[1], option[2]);
                }
            }
        }
    }
});

function compressReverseGreedily(x_off, y_off, z_off, x_par, y_par, z_par, x_size, y_size, z_size) {
    //loop over all the spots where the mask could fit in the parent block
    for (let z = 0; z <= z_par - z_size; z++) {
        for (let y = 0; y <= y_par - y_size; y++) {
            for (let x = 0; x <= x_par - x_size; x++) {
                //get the top left block of the mask
                let initial_block = map[z_off + z][y_off + y][x_off + x];
                //if we've already visited, no need to check again
                if (initial_block === -1) {
                    continue;
                }
                let fits = true;
                //loop over the everything in the mask we are trying to fit
                for (let z_alpha = z; z_alpha < z + z_size; z_alpha++) {
                    for (let y_alpha = y; y_alpha < y + y_size; y_alpha++) {
                        for (let x_alpha = x; x_alpha < x + x_size; x_alpha++) {
                            //if any arent equal to the first, the block doesnt fit there
                            if (map[z_off + z_alpha][y_off + y_alpha][x_off + x_alpha] !== initial_block) {
                                fits = false;
                                break;
                            }
                        }
                        if (!fits) {
                            break;
                        }
                    }
                    if (!fits) {
                        break;
                    }
                }
                //if the block fits, mark each smaller block as visited
                if (fits) {
                    for (let a = 0; a < z_size; a++) {
                        for (let b = 0; b < y_size; b++) {
                            for (let c = 0; c < x_size; c++) {
                                map[a + z_off + z][b + y_off + y][c + x_off + x] = -1;
                            }
                        }
                    }
                    //then print it
                    console.log(`${x_off + x},${y_off + y},${z_off + z},${x_size},${y_size},${z_size},${initial_block}`);
                }
            }
        }
    }
}
