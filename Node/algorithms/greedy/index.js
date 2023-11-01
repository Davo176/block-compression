const fs = require("fs");

let map = [[]];
const tag_table = {};
let x_count;
let y_count;
let z_count;
let parent_x;
let parent_y;
let parent_z;

//This algorithm was not set up to read from a stream
//so read from stdin (file pointer 0)
//and once done, run the callback (file is in data)
fs.readFile(0, "utf8", (err, data) => {
    if (err) {
        console.log(err);
    }
    //get an array with each line
    const file = data.split("\n");
    //pop the first line off, split it on the commas, and store the values
    [x_count, y_count, z_count, parent_x, parent_y, parent_z] = file
        .shift()
        .split(",")
        .map((e) => parseInt(e));

    //Track if we have passed the tag table
    let passed_tag_table = false;
    //track what index we are up to in each dimension
    let x_index = -1;
    let y_index = -1;
    let z_index = 0;

    //loop over file
    for (let line of file) {
        line = line.trim(); //remove new line characters
        //if our line is null, and we havent passed the tag table
        if (!line && !passed_tag_table) {
            passed_tag_table = true; //now we have passed tag table
            continue; //go to next line
        }
        //if we are passed the tag table
        if (passed_tag_table) {
            //if line is empty
            if (!line) {
                //up to next z layer
                z_index += 1;
                map.push([]);
                y_index = -1;
                continue;
            } else {
                //otherwise at next y layer
                y_index += 1;
                map[z_index].push([]);
            }
            //then loop over all chars in the line, and add corresponding
            //material to our map, based interpreting from tag table
            x_index = -1;
            for (let char of line) {
                x_index += 1;
                map[z_index][y_index].push(tag_table[char]);
            }
            //if we arent passed the tag table
        } else {
            //parse the line, and add to tag table
            tag_table[line.split(", ")[0]] = line.split(", ")[1];
        }
    }
    //Add all index's of a cube into an array
    //so we can sort them in fancy ways and loop over it later
    let options = [];
    for (let z = 0; z < parent_z; z++) {
        for (let y = 0; y < parent_y; y++) {
            for (let x = 0; x < parent_x; x++) {
                options.push([x, y, z]);
            }
        }
    }
    //Sort them based off this arbitrary algorithm I decided (this can be optimised)
    options.sort((a, b) => {
        let a_sum = 0;
        for (let i = 0; i < a.length; i++) {
            a_sum += a[i];
            if (a[i] === 0) {
                a_sum -= 0.6;
            }
        }
        let b_sum = 0;
        for (let i = 0; i < b.length; i++) {
            b_sum += b[i];
            if (b[i] === 0) {
                b_sum -= 0.6;
            }
        }
        if (a_sum < b_sum) {
            return -1;
        }
        if (a_sum > b_sum) {
            return 1;
        }
        for (let j = 0; j < a.length; j++) {
            if (a[j] < b[j]) {
                return 1;
            }
            if (a[j] > b[j]) {
                return -1;
            }
        }
        return 0;
    });
    //Loop over all the parent blocks
    for (let z_beta = 0; z_beta < z_count; z_beta += parent_z) {
        for (let y_beta = 0; y_beta < y_count; y_beta += parent_y) {
            for (let x_beta = 0; x_beta < x_count; x_beta += parent_x) {
                //and then all the index's of that parent block
                for (let option of options) {
                    //try and grow from that index of parent block
                    compressGreedily(
                        option[0] + x_beta, //x coord of top left back block
                        option[1] + y_beta, //y coord of top left back block
                        option[2] + z_beta, //z coord of top left back block
                        x_beta, //x coord of top left back corner of parent block
                        y_beta, //y coord of top left back corner of parent block
                        z_beta //z coord of top left back corner of parent block
                    );
                }
            }
        }
    }
});

function compressGreedily(x_alpha, y_alpha, z_alpha, x_beta, y_beta, z_beta) {
    //define our reference block
    let reference_block = map[z_alpha][y_alpha][x_alpha];
    if (reference_block === -1) {
        return -1;
    }
    let can_grow_x = true;
    let can_grow_y = true;
    let can_grow_z = true;
    let x_size = 1;
    let y_size = 1;
    let z_size = 1;
    do {
        //underlying idea is
        //Check in _ direction if all the blocks in that direction are the same as the current block.
        //e.g. if at 0,0,0 and its SEA, check if 1,0,0 is sea, if it is grow.
        // then check if 0,1,0 and 1,1,0 are both SEA, then grow
        // then check 0,0,1 1,0,1, 0,1,1 1,1,1 are all SEA. If one isn't we eliminate z from being allowed to grow (or we hit the edge of a parent block)
        // we keep going, checking x, y and z until all 3 can't grow anymore. That's our block.

        //can grow x?
        can_grow_x = true;
        for (let z = 0; z < z_size; z++) {
            for (let y = 0; y < y_size; y++) {
                //check if the we are over parent block size or the next block is not the reference block
                if (x_size + x_alpha + 1 > x_beta + parent_x || map[z + z_alpha][y + y_alpha][x_size + x_alpha] != reference_block) {
                    can_grow_x = false;
                    break;
                }
            }
            if (!can_grow_x) {
                break;
            }
        }
        if (can_grow_x) {
            //then grow x
            x_size++;
        }
        //can grow y?
        can_grow_y = true;
        for (let z = 0; z < z_size; z++) {
            for (let x = 0; x < x_size; x++) {
                //check if the we are over parent block size or the next block is not the reference block
                if (y_size + y_alpha + 1 > y_beta + parent_y || map[z + z_alpha][y_size + y_alpha][x + x_alpha] != reference_block) {
                    can_grow_y = false;
                    break;
                }
            }
            if (!can_grow_y) {
                break;
            }
        }
        if (can_grow_y) {
            //then grow y
            y_size++;
        }
        //can grow z
        can_grow_z = true;
        for (let y = 0; y < y_size; y++) {
            for (let x = 0; x < x_size; x++) {
                //check if the we are over parent block size or the next block is not the reference block
                if (z_size + z_alpha + 1 > z_beta + parent_z || map[z_size + z_alpha][y + y_alpha][x + x_alpha] != reference_block) {
                    can_grow_z = false;
                    break;
                }
                if (!can_grow_z) {
                    break;
                }
            }
        }
        if (can_grow_z) {
            //then grow z
            z_size++;
        }
    } while (can_grow_x || can_grow_y || can_grow_z);
    //once we have made a block, we mark all the blocks within it as visited by setting to -1 so they dont get included in another block
    for (let a = 0; a < z_size; a++) {
        for (let b = 0; b < y_size; b++) {
            for (let c = 0; c < x_size; c++) {
                map[a + z_alpha][b + y_alpha][c + x_alpha] = -1;
            }
        }
    }
    //then print that block out.
    console.log(`${x_alpha},${y_alpha},${z_alpha},${x_size},${y_size},${z_size},${reference_block}`);
}
