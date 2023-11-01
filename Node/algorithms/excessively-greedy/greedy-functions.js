const { extractBlock, createBuckets, optimiseBuckets } = require("./utils");

function run_greedy(map, x_beta, y_beta, z_beta, parent_x, parent_y, parent_z, sorted_options) {
    let results = [];
    let orders = ["xyz", "xzy", "yxz", "yzx", "zxy", "zyx"];
    let doing_xyz_rle_options = [false, true]; //This is for Manishes XYZ RLE implementation
    //it changes from growing once in each direction, to all the way in one, all the way in next, all the way in next.
    for (let doing_xyz_rle_option of doing_xyz_rle_options) {
        //loop over two variations
        for (let options of sorted_options) {
            //then all the ways to iterate over a cube
            for (let order of orders) {
                //then the different orders of growing
                //Left / right = x
                //Top / Bottom = y
                //back / front = z
                let top_left_back = [];
                let top_left_front = [];
                let top_right_back = [];
                let top_right_front = [];
                let bottom_left_back = [];
                let bottom_left_front = [];
                let bottom_right_back = [];
                let bottom_right_front = [];
                let top_left_back_map = extractBlock(map, x_beta, y_beta, z_beta, parent_x, parent_y, parent_z);
                let top_left_front_map = extractBlock(map, x_beta, y_beta, z_beta, parent_x, parent_y, parent_z);
                let top_right_back_map = extractBlock(map, x_beta, y_beta, z_beta, parent_x, parent_y, parent_z);
                let top_right_front_map = extractBlock(map, x_beta, y_beta, z_beta, parent_x, parent_y, parent_z);
                let bottom_left_back_map = extractBlock(map, x_beta, y_beta, z_beta, parent_x, parent_y, parent_z);
                let bottom_left_front_map = extractBlock(map, x_beta, y_beta, z_beta, parent_x, parent_y, parent_z);
                let bottom_right_back_map = extractBlock(map, x_beta, y_beta, z_beta, parent_x, parent_y, parent_z);
                let bottom_right_front_map = extractBlock(map, x_beta, y_beta, z_beta, parent_x, parent_y, parent_z);
                //create copys of the sections of maps we are compressing, so can be written over for algos to work.
                for (let option of options) {
                    //try and grow from that index of parent block
                    //each of these return a string for one block (which produces an array over a parent block)
                    //these run greedy from the corner specified.
                    let tlb = greedy_top_left_back(x_beta, y_beta, z_beta, option[0], option[1], option[2], 0, 0, 0, parent_x, parent_y, parent_z, top_left_back_map, order, doing_xyz_rle_option);
                    let tlf = greedy_top_left_front(x_beta, y_beta, z_beta, option[0], option[1], option[2], 0, 0, 0 + parent_z - 1, parent_x, parent_y, parent_z, top_left_front_map, order, doing_xyz_rle_option);
                    let trb = greedy_top_right_back(x_beta, y_beta, z_beta, option[0], option[1], option[2], 0 + parent_x - 1, 0, 0, parent_x, parent_y, parent_z, top_right_back_map, order, doing_xyz_rle_option);
                    let trf = greedy_top_right_front(x_beta, y_beta, z_beta, option[0], option[1], option[2], 0 + parent_x - 1, 0, 0 + parent_z - 1, parent_x, parent_y, parent_z, top_right_front_map, order, doing_xyz_rle_option);
                    let blb = greedy_bottom_left_back(x_beta, y_beta, z_beta, option[0], option[1], option[2], 0, 0 + parent_y - 1, 0, parent_x, parent_y, parent_z, bottom_left_back_map, order, doing_xyz_rle_option);
                    let blf = greedy_bottom_left_front(x_beta, y_beta, z_beta, option[0], option[1], option[2], 0, 0 + parent_y - 1, 0 + parent_z - 1, parent_x, parent_y, parent_z, bottom_left_front_map, order, doing_xyz_rle_option);
                    let brb = greedy_bottom_right_back(x_beta, y_beta, z_beta, option[0], option[1], option[2], 0 + parent_x - 1, 0 + parent_y - 1, 0, parent_x, parent_y, parent_z, bottom_right_back_map, order, doing_xyz_rle_option);
                    let brf = greedy_bottom_right_front(x_beta, y_beta, z_beta, option[0], option[1], option[2], 0 + parent_x - 1, 0 + parent_y - 1, 0 + parent_z - 1, parent_x, parent_y, parent_z, bottom_right_front_map, order, doing_xyz_rle_option);
                    if (tlb !== -1) {
                        top_left_back.push(tlb);
                    }
                    if (tlf !== -1) {
                        top_left_front.push(tlf);
                    }
                    if (trb !== -1) {
                        top_right_back.push(trb);
                    }
                    if (trf !== -1) {
                        top_right_front.push(trf);
                    }
                    if (blb !== -1) {
                        bottom_left_back.push(blb);
                    }
                    if (blf !== -1) {
                        bottom_left_front.push(blf);
                    }
                    if (brb !== -1) {
                        bottom_right_back.push(brb);
                    }
                    if (brf !== -1) {
                        bottom_right_front.push(brf);
                    }
                }
                //create buckets takes in a list of strings e.g. ["0,0,0,1,1,1,Z","1,1,1,2,2,2,Q"] and converts into veins/buckets

                //so to clarify, you can send optimise buckets to this, a list of list of buckets (produced by the createBuckets function)
                //[["0,0,0,1,1,1,Z","1,1,1,2,2,2,Q"],["0,0,0,1,1,1,Z","1,1,1,2,2,2,Q"],...]
                //that are the outputs of different compression algorithms
                //and itll optimise them for u :)

                let unoptimised_buckets = [top_left_back, top_left_front, top_right_back, top_right_front, bottom_left_back, bottom_right_back, bottom_left_front, bottom_right_front].map(createBuckets);

                results.push(optimiseBuckets(unoptimised_buckets));
            }
        }
    }
    //then return all my buckets!
    return results;
}

//im not explaining all of these. I will do the first two, that should be enough for
//you to understand the rest.

function greedy_top_left_back(x_par_offset, y_par_offset, z_par_offset, x_offset, y_offset, z_offset, x_beta, y_beta, z_beta, parent_x, parent_y, parent_z, map, grow_order = "xyz", doing_xyz_rle = false) {
    //inital_block = reference block
    let initial_block = map[z_offset + z_beta][y_offset + y_beta][x_offset + x_beta];
    if (initial_block === -1) {
        return -1;
    }
    let can_grow_x = true;
    let can_grow_y = true;
    let can_grow_z = true;
    let x_size = 1;
    let y_size = 1;
    let z_size = 1;
    let grow_iter = 0; //tracks which order we should try and grow in, like what we are up to
    //so grow order might look like "xzy" then grow_order[grow_iter] == 'x' so we grow x
    //then grow_iter+=1 (mod3) so then z, then y
    //created this system so didnt have to write hundreds of functions, for each order.
    do {
        //underlying idea is
        //Check in _ direction if all the blocks in that direction are the same as the current block.
        //e.g. if at 0,0,0 and its SEA, check if 1,0,0 is sea, if it is grow.
        // then check if 0,1,0 and 1,1,0 are both SEA, then grow
        // then check 0,0,1 1,0,1, 0,1,1 1,1,1 are all SEA. If one isn't we eliminate z from being allowed to grow (or we hit the edge of a parent block)
        // we keep going, checking x, y and z until all 3 can't grow anymore. That's our block.

        //if up to growing x
        if (grow_order[grow_iter] === "x") {
            grow_iter = (grow_iter + 1) % 3; //increment grow iter
            do {
                //can grow x?
                for (let z = 0; z < z_size; z++) {
                    for (let y = 0; y < y_size; y++) {
                        if (x_size + x_offset + x_beta + 1 > x_beta + parent_x || map[z + z_offset + z_beta][y + y_offset + y_beta][x_size + x_offset + x_beta] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_x); //if we are doing xyz_rle, keep going til we cant any more
        }
        //same as above but for y
        if (grow_order[grow_iter] === "y") {
            grow_iter = (grow_iter + 1) % 3;
            do {
                //can grow y?
                for (let z = 0; z < z_size; z++) {
                    for (let x = 0; x < x_size; x++) {
                        if (y_size + y_offset + y_beta + 1 > y_beta + parent_y || map[z + z_offset + z_beta][y_size + y_offset + y_beta][x + x_offset + x_beta] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_y);
        }
        //then z.
        if (grow_order[grow_iter] === "z") {
            grow_iter = (grow_iter + 1) % 3;
            do {
                //can grow z
                for (let y = 0; y < y_size; y++) {
                    for (let x = 0; x < x_size; x++) {
                        if (z_size + z_offset + z_beta + 1 > z_beta + parent_z || map[z_size + z_offset + z_beta][y + y_offset + y_beta][x + x_offset + x_beta] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_z);
        }
    } while (can_grow_x || can_grow_y || can_grow_z);
    //once we have made a block, we mark all the blocks within it as visited by setting to -1 so they dont get included in another block
    for (let a = 0; a < z_size; a++) {
        for (let b = 0; b < y_size; b++) {
            for (let c = 0; c < x_size; c++) {
                map[a + z_offset + z_beta][b + y_offset + y_beta][c + x_offset + x_beta] = -1;
            }
        }
    }
    //then return that block for further processing
    return `${x_par_offset + x_offset + x_beta},${y_par_offset + y_offset + y_beta},${z_par_offset + z_offset + z_beta},${x_size},${y_size},${z_size},${initial_block}`;
}

//mostly the same, but will point out the different bit
function greedy_top_right_back(x_par_offset, y_par_offset, z_par_offset, x_offset, y_offset, z_offset, x_beta, y_beta, z_beta, parent_x, parent_y, parent_z, map, grow_order = "xyz", doing_xyz_rle = false) {
    //get the right block, not left block! by doing x_beta - x_offset
    let initial_block = map[z_offset + z_beta][y_offset + y_beta][x_beta - x_offset];
    if (initial_block === -1) {
        return -1;
    }
    let can_grow_x = true;
    let can_grow_y = true;
    let can_grow_z = true;
    let x_size = 1;
    let y_size = 1;
    let z_size = 1;
    let grow_iter = 0;
    do {
        if (grow_order[grow_iter] === "x") {
            grow_iter = (grow_iter + 1) % 3;
            do {
                for (let z = 0; z < z_size; z++) {
                    for (let y = 0; y < y_size; y++) {
                        //so here, insted we need to check blocks to the left, not to the right
                        //still checking if in parent block range, and if block to left is same as initial block
                        if (x_beta - x_offset - x_size - 1 < x_beta - parent_x || map[z + z_offset + z_beta][y + y_offset + y_beta][x_beta - x_offset - x_size] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_x);
        }
        if (grow_order[grow_iter] === "y") {
            grow_iter = (grow_iter + 1) % 3;
            do {
                //can grow y?
                for (let z = 0; z < z_size; z++) {
                    for (let x = 0; x < x_size; x++) {
                        if (y_size + y_offset + y_beta + 1 > y_beta + parent_y || map[z + z_offset + z_beta][y_size + y_offset + y_beta][x_beta - x_offset - x] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_y);
        }
        if (grow_order[grow_iter] === "z") {
            grow_iter = (grow_iter + 1) % 3;
            do {
                //can grow z
                for (let y = 0; y < y_size; y++) {
                    for (let x = 0; x < x_size; x++) {
                        if (z_size + z_offset + z_beta + 1 > z_beta + parent_z || map[z_size + z_offset + z_beta][y + y_offset + y_beta][x_beta - x_offset - x] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_z);
        }
    } while (can_grow_x || can_grow_y || can_grow_z);
    //once we have made a block, we mark all the blocks within it as visited by setting to -1 so they dont get included in another block
    for (let a = 0; a < z_size; a++) {
        for (let b = 0; b < y_size; b++) {
            for (let c = 0; c < x_size; c++) {
                map[a + z_offset + z_beta][b + y_offset + y_beta][x_beta - x_offset - c] = -1;
            }
        }
    }
    //then return that block .
    return `${x_par_offset + x_beta - x_offset - x_size + 1},${y_par_offset + y_offset + y_beta},${z_par_offset + z_offset + z_beta},${x_size},${y_size},${z_size},${initial_block}`;
}

//so that same pattern is repeated for next 6 functions.

function greedy_bottom_left_back(x_par_offset, y_par_offset, z_par_offset, x_offset, y_offset, z_offset, x_beta, y_beta, z_beta, parent_x, parent_y, parent_z, map, grow_order = "xyz", doing_xyz_rle = false) {
    let initial_block = map[z_offset + z_beta][y_beta - y_offset][x_offset + x_beta];
    if (initial_block === -1) {
        return -1;
    }
    let can_grow_x = true;
    let can_grow_y = true;
    let can_grow_z = true;
    let x_size = 1;
    let y_size = 1;
    let z_size = 1;
    let grow_iter = 0;
    do {
        if (grow_order[grow_iter] === "x") {
            grow_iter = (grow_iter + 1) % 3;
            do {
                //can grow x?
                for (let z = 0; z < z_size; z++) {
                    for (let y = 0; y < y_size; y++) {
                        if (x_size + x_offset + x_beta + 1 > x_beta + parent_x || map[z + z_offset + z_beta][y_beta - y_offset - y][x_size + x_offset + x_beta] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_x);
        }
        if (grow_order[grow_iter] === "y") {
            do {
                grow_iter = (grow_iter + 1) % 3;
                //can grow y?
                for (let z = 0; z < z_size; z++) {
                    for (let x = 0; x < x_size; x++) {
                        if (y_beta - y_offset - y_size - 1 < y_beta - parent_y || map[z + z_offset + z_beta][y_beta - y_offset - y_size][x + x_offset + x_beta] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_y);
        }
        if (grow_order[grow_iter] === "z") {
            grow_iter = (grow_iter + 1) % 3;
            do {
                //can grow z
                for (let y = 0; y < y_size; y++) {
                    for (let x = 0; x < x_size; x++) {
                        if (z_size + z_offset + z_beta + 1 > z_beta + parent_z || map[z_size + z_offset + z_beta][y_beta - y - y_offset][x + x_offset + x_beta] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_z);
        }
    } while (can_grow_x || can_grow_y || can_grow_z);
    //once we have made a block, we mark all the blocks within it as visited by setting to -1 so they dont get included in another block
    for (let a = 0; a < z_size; a++) {
        for (let b = 0; b < y_size; b++) {
            for (let c = 0; c < x_size; c++) {
                map[a + z_offset + z_beta][y_beta - y_offset - b][c + x_offset + x_beta] = -1;
            }
        }
    }
    //then print that block out.
    return `${x_par_offset + x_offset + x_beta},${y_par_offset + y_beta - y_offset - y_size + 1},${z_par_offset + z_offset + z_beta},${x_size},${y_size},${z_size},${initial_block}`;
}

function greedy_top_left_front(x_par_offset, y_par_offset, z_par_offset, x_offset, y_offset, z_offset, x_beta, y_beta, z_beta, parent_x, parent_y, parent_z, map, grow_order = "xyz", doing_xyz_rle = false) {
    let initial_block = map[z_beta - z_offset][y_offset + y_beta][x_offset + x_beta];
    if (initial_block === -1) {
        return -1;
    }
    let can_grow_x = true;
    let can_grow_y = true;
    let can_grow_z = true;
    let x_size = 1;
    let y_size = 1;
    let z_size = 1;
    let grow_iter = 0;
    do {
        //can grow x?
        if (grow_order[grow_iter] === "x") {
            grow_iter = (grow_iter + 1) % 3;
            do {
                for (let z = 0; z < z_size; z++) {
                    for (let y = 0; y < y_size; y++) {
                        if (x_size + x_offset + x_beta + 1 > x_beta + parent_x || map[z_beta - z_offset - z][y + y_offset + y_beta][x_size + x_offset + x_beta] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_x);
        }
        if (grow_order[grow_iter] === "y") {
            grow_iter = (grow_iter + 1) % 3;
            do {
                //can grow y?
                for (let z = 0; z < z_size; z++) {
                    for (let x = 0; x < x_size; x++) {
                        if (y_size + y_offset + y_beta + 1 > y_beta + parent_y || map[z_beta - z_offset - z][y_size + y_offset + y_beta][x + x_offset + x_beta] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_y);
        }
        if (grow_order[grow_iter] === "z") {
            grow_iter = (grow_iter + 1) % 3;
            do {
                //can grow z
                for (let y = 0; y < y_size; y++) {
                    for (let x = 0; x < x_size; x++) {
                        if (z_beta - z_offset - z_size - 1 < z_beta - parent_z || map[z_beta - z_offset - z_size][y + y_offset + y_beta][x + x_offset + x_beta] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_z);
        }
    } while (can_grow_x || can_grow_y || can_grow_z);
    //once we have made a block, we mark all the blocks within it as visited by setting to -1 so they dont get included in another block
    for (let a = 0; a < z_size; a++) {
        for (let b = 0; b < y_size; b++) {
            for (let c = 0; c < x_size; c++) {
                map[z_beta - z_offset - a][b + y_offset + y_beta][c + x_offset + x_beta] = -1;
            }
        }
    }
    //then print that block out.
    return `${x_par_offset + x_offset + x_beta},${y_par_offset + y_offset + y_beta},${z_par_offset + z_beta - z_offset - z_size + 1},${x_size},${y_size},${z_size},${initial_block}`;
}

function greedy_bottom_right_back(x_par_offset, y_par_offset, z_par_offset, x_offset, y_offset, z_offset, x_beta, y_beta, z_beta, parent_x, parent_y, parent_z, map, grow_order = "xyz", doing_xyz_rle = false) {
    let initial_block = map[z_offset + z_beta][y_beta - y_offset][x_beta - x_offset];
    if (initial_block === -1) {
        return -1;
    }
    let can_grow_x = true;
    let can_grow_y = true;
    let can_grow_z = true;
    let x_size = 1;
    let y_size = 1;
    let z_size = 1;
    let grow_iter = 0;
    do {
        //can grow x?
        if (grow_order[grow_iter] === "x") {
            grow_iter = (grow_iter + 1) % 3;
            do {
                for (let z = 0; z < z_size; z++) {
                    for (let y = 0; y < y_size; y++) {
                        if (x_beta - x_offset - x_size - 1 < x_beta - parent_x || map[z + z_offset + z_beta][y_beta - y_offset - y][x_beta - x_offset - x_size] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_x);
        }
        if (grow_order[grow_iter] === "y") {
            grow_iter = (grow_iter + 1) % 3;
            do {
                //can grow y?
                for (let z = 0; z < z_size; z++) {
                    for (let x = 0; x < x_size; x++) {
                        if (y_beta - y_offset - y_size - 1 < y_beta - parent_y || map[z + z_offset + z_beta][y_beta - y_offset - y_size][x_beta - x_offset - x] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_y);
        }
        if (grow_order[grow_iter] === "z") {
            grow_iter = (grow_iter + 1) % 3;
            do {
                //can grow z
                for (let y = 0; y < y_size; y++) {
                    for (let x = 0; x < x_size; x++) {
                        if (z_size + z_offset + z_beta + 1 > z_beta + parent_z || map[z_size + z_offset + z_beta][y_beta - y_offset - y][x_beta - x_offset - x] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_z);
        }
    } while (can_grow_x || can_grow_y || can_grow_z);
    //once we have made a block, we mark all the blocks within it as visited by setting to -1 so they dont get included in another block
    for (let a = 0; a < z_size; a++) {
        for (let b = 0; b < y_size; b++) {
            for (let c = 0; c < x_size; c++) {
                map[a + z_offset + z_beta][y_beta - y_offset - b][x_beta - x_offset - c] = -1;
            }
        }
    }
    //then print that block out.
    return `${x_par_offset + x_beta - x_offset - x_size + 1},${y_par_offset + y_beta - y_offset - y_size + 1},${z_par_offset + z_offset + z_beta},${x_size},${y_size},${z_size},${initial_block}`;
}

function greedy_top_right_front(x_par_offset, y_par_offset, z_par_offset, x_offset, y_offset, z_offset, x_beta, y_beta, z_beta, parent_x, parent_y, parent_z, map, grow_order = "xyz", doing_xyz_rle = false) {
    let initial_block = map[z_beta - z_offset][y_offset + y_beta][x_beta - x_offset];
    if (initial_block === -1) {
        return -1;
    }
    let can_grow_x = true;
    let can_grow_y = true;
    let can_grow_z = true;
    let x_size = 1;
    let y_size = 1;
    let z_size = 1;
    let grow_iter = 0;
    do {
        if (grow_order[grow_iter] === "x") {
            grow_iter = (grow_iter + 1) % 3;
            do {
                //can grow x?
                for (let z = 0; z < z_size; z++) {
                    for (let y = 0; y < y_size; y++) {
                        if (x_beta - x_offset - x_size - 1 < x_beta - parent_x || map[z_beta - z_offset - z][y + y_offset + y_beta][x_beta - x_offset - x_size] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_x);
        }
        if (grow_order[grow_iter] === "y") {
            grow_iter = (grow_iter + 1) % 3;
            do {
                //can grow y?
                for (let z = 0; z < z_size; z++) {
                    for (let x = 0; x < x_size; x++) {
                        if (y_size + y_offset + y_beta + 1 > y_beta + parent_y || map[z_beta - z_offset - z][y_size + y_offset + y_beta][x_beta - x_offset - x] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_y);
        }
        if (grow_order[grow_iter] === "z") {
            grow_iter = (grow_iter + 1) % 3;
            do {
                //can grow z
                for (let y = 0; y < y_size; y++) {
                    for (let x = 0; x < x_size; x++) {
                        if (z_beta - z_offset - z_size - 1 < z_beta - parent_z || map[z_beta - z_offset - z_size][y + y_offset + y_beta][x_beta - x_offset - x] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_z);
        }
    } while (can_grow_x || can_grow_y || can_grow_z);
    //once we have made a block, we mark all the blocks within it as visited by setting to -1 so they dont get included in another block
    for (let a = 0; a < z_size; a++) {
        for (let b = 0; b < y_size; b++) {
            for (let c = 0; c < x_size; c++) {
                map[z_beta - z_offset - a][b + y_offset + y_beta][x_beta - x_offset - c] = -1;
            }
        }
    }
    //then print that block out.
    return `${x_par_offset + x_beta - x_offset - x_size + 1},${y_par_offset + y_offset + y_beta},${z_par_offset + z_beta - z_offset - z_size + 1},${x_size},${y_size},${z_size},${initial_block}`;
}

function greedy_bottom_left_front(x_par_offset, y_par_offset, z_par_offset, x_offset, y_offset, z_offset, x_beta, y_beta, z_beta, parent_x, parent_y, parent_z, map, grow_order = "xyz", doing_xyz_rle = false) {
    let initial_block = map[z_beta - z_offset][y_beta - y_offset][x_offset + x_beta];
    if (initial_block === -1) {
        return -1;
    }
    let can_grow_x = true;
    let can_grow_y = true;
    let can_grow_z = true;
    let x_size = 1;
    let y_size = 1;
    let z_size = 1;
    let grow_iter = 0;
    do {
        if (grow_order[grow_iter] === "x") {
            grow_iter = (grow_iter + 1) % 3;
            do {
                //can grow x?
                for (let z = 0; z < z_size; z++) {
                    for (let y = 0; y < y_size; y++) {
                        if (x_size + x_offset + x_beta + 1 > x_beta + parent_x || map[z_beta - z_offset - z][y_beta - y_offset - y][x_size + x_offset + x_beta] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_x);
        }
        if (grow_order[grow_iter] === "y") {
            grow_iter = (grow_iter + 1) % 3;
            //can grow y?
            do {
                for (let z = 0; z < z_size; z++) {
                    for (let x = 0; x < x_size; x++) {
                        if (y_beta - y_offset - y_size - 1 < y_beta - parent_y || map[z_beta - z_offset - z][y_beta - y_offset - y_size][x + x_offset + x_beta] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_y);
        }
        if (grow_order[grow_iter] === "z") {
            grow_iter = (grow_iter + 1) % 3;
            //can grow z
            do {
                for (let y = 0; y < y_size; y++) {
                    for (let x = 0; x < x_size; x++) {
                        if (z_beta - z_offset - z_size - 1 < z_beta - parent_z || map[z_beta - z_offset - z_size][y_beta - y_offset - y][x + x_offset + x_beta] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_z);
        }
    } while (can_grow_x || can_grow_y || can_grow_z);
    //once we have made a block, we mark all the blocks within it as visited by setting to -1 so they dont get included in another block
    for (let a = 0; a < z_size; a++) {
        for (let b = 0; b < y_size; b++) {
            for (let c = 0; c < x_size; c++) {
                map[z_beta - z_offset - a][y_beta - y_offset - b][c + x_offset + x_beta] = -1;
            }
        }
    }
    //then print that block out.
    return `${x_par_offset + x_offset + x_beta},${y_par_offset + y_beta - y_offset - y_size + 1},${z_par_offset + z_beta - z_offset - z_size + 1},${x_size},${y_size},${z_size},${initial_block}`;
}

function greedy_bottom_right_front(x_par_offset, y_par_offset, z_par_offset, x_offset, y_offset, z_offset, x_beta, y_beta, z_beta, parent_x, parent_y, parent_z, map, grow_order = "xyz", doing_xyz_rle = false) {
    let initial_block = map[z_beta - z_offset][y_beta - y_offset][x_beta - x_offset];
    if (initial_block === -1) {
        return -1;
    }
    let can_grow_x = true;
    let can_grow_y = true;
    let can_grow_z = true;
    let x_size = 1;
    let y_size = 1;
    let z_size = 1;
    let grow_iter = 0;
    do {
        if (grow_order[grow_iter] === "x") {
            grow_iter = (grow_iter + 1) % 3;
            do {
                //can grow x?
                for (let z = 0; z < z_size; z++) {
                    for (let y = 0; y < y_size; y++) {
                        if (x_beta - x_offset - x_size - 1 < x_beta - parent_x || map[z_beta - z_offset - z][y_beta - y_offset - y][x_beta - x_offset - x_size] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_x);
        }
        if (grow_order[grow_iter] === "y") {
            grow_iter = (grow_iter + 1) % 3;
            //can grow y?
            do {
                for (let z = 0; z < z_size; z++) {
                    for (let x = 0; x < x_size; x++) {
                        if (y_beta - y_offset - y_size - 1 < y_beta - parent_y || map[z_beta - z_offset - z][y_beta - y_offset - y_size][x_beta - x_offset - x] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_y);
        }
        if (grow_order[grow_iter] === "z") {
            grow_iter = (grow_iter + 1) % 3;
            do {
                //can grow z
                for (let y = 0; y < y_size; y++) {
                    for (let x = 0; x < x_size; x++) {
                        if (z_beta - z_offset - z_size - 1 < z_beta - parent_z || map[z_beta - z_offset - z_size][y_beta - y_offset - y][x_beta - x_offset - x] != initial_block) {
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
            } while (doing_xyz_rle && can_grow_z);
        }
    } while (can_grow_x || can_grow_y || can_grow_z);
    //once we have made a block, we mark all the blocks within it as visited by setting to -1 so they dont get included in another block
    for (let a = 0; a < z_size; a++) {
        for (let b = 0; b < y_size; b++) {
            for (let c = 0; c < x_size; c++) {
                map[z_beta - z_offset - a][y_beta - y_offset - b][x_beta - x_offset - c] = -1;
            }
        }
    }
    //then print that block out.
    return `${x_par_offset + x_beta - x_offset - x_size + 1},${y_par_offset + y_beta - y_offset - y_size + 1},${z_par_offset + z_beta - z_offset - z_size + 1},${x_size},${y_size},${z_size},${initial_block}`;
}

//We removed reverse greedy from this due to time constraints, but
//code remains for easy readding
function reverse_greedy(x_par_offset, y_par_offset, z_par_offset, x_off, y_off, z_off, x_par, y_par, z_par, x_size, y_size, z_size, map) {
    let return_blocks = [];
    //loop over all the spots where the block could fit in the parent block
    for (let z = 0; z <= z_par - z_size; z++) {
        for (let y = 0; y <= y_par - y_size; y++) {
            for (let x = 0; x <= x_par - x_size; x++) {
                //get the top left block
                let initial_block = map[z_off + z][y_off + y][x_off + x];
                //if we've already visited, no need to check again
                if (initial_block === -1) {
                    continue;
                }
                let fits = true;
                //loop over the size of the block we are trying to fit
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
                    return_blocks.push(`${x_par_offset + x},${y_par_offset + y},${z_par_offset + z},${x_size},${y_size},${z_size},${initial_block}`);
                }
            }
        }
    }
    if (return_blocks.length === 0) {
        return -1;
    }
    return return_blocks;
}

module.exports = { run_greedy };
