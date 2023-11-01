//takes in a large chunk, starting coords, and a size
//and returns a smaller 3d array zero indexed of size
//identical to that section of the parent block
function extractBlock(map, offsetX, offsetY, offsetZ, sizeX, sizeY, sizeZ) {
    const result = [];
    for (let z = offsetZ; z < offsetZ + sizeZ; z++) {
        const yArray = [];
        for (let y = offsetY; y < offsetY + sizeY; y++) {
            const xArray = [];
            for (let x = offsetX; x < offsetX + sizeX; x++) {
                xArray.push(map[z][y][x]);
            }
            yArray.push(xArray);
        }
        result.push(yArray);
    }
    return result;
}
//takes in two cuboids
//in the output format
//and returns true if they are touching
//orthogonally (not diagonally)
function calcTouching(element, output) {
    //get the min and max of both in x,y,z
    let e_min_x = element[0];
    let e_max_x = element[0] + element[3] - 1;
    let e_min_y = element[1];
    let e_max_y = element[1] + element[4] - 1;
    let e_min_z = element[2];
    let e_max_z = element[2] + element[5] - 1;
    let o_min_x = output[0];
    let o_max_x = output[0] + output[3] - 1;
    let o_min_y = output[1];
    let o_max_y = output[1] + output[4] - 1;
    let o_min_z = output[2];
    let o_max_z = output[2] + output[5] - 1;
    //work out if they are in range in each direction
    let in_range_x = o_max_x >= e_min_x && o_min_x <= e_max_x; //someone check my logic?
    let in_range_y = o_max_y >= e_min_y && o_min_y <= e_max_y; //someone check my logic?
    let in_range_z = o_max_z >= e_min_z && o_min_z <= e_max_z; //someone check my logic?
    //work out if their edges are aligned
    let touching_left = e_min_x - 1 === o_max_x;
    let touching_right = e_max_x + 1 === o_min_x;
    let touching_up = e_min_y - 1 === o_max_y;
    let touching_down = e_max_y + 1 === o_min_y;
    let touching_back = e_min_z - 1 === o_max_z;
    let touching_front = e_max_z + 1 === o_min_z;
    //now return true if we are in range, and touching on a correct face.
    return (touching_left && in_range_y && in_range_z) || (touching_right && in_range_y && in_range_z) || (touching_up && in_range_x && in_range_z) || (touching_down && in_range_x && in_range_z) || (touching_back && in_range_x && in_range_y) || (touching_front && in_range_x && in_range_y);
}

//takes in cuboids in the output format
//returns buckets.
function createBuckets(outputs) {
    //parse the outputs back out from a string to an array so I can work with them
    outputs = outputs.map((o) => o.split(",").map((n) => (parseInt(n) >= 0 ? parseInt(n) : n)));
    //initialise buckets
    let buckets = [];
    //for each cuboid in output
    for (let output of outputs) {
        //initially not in any buckets
        let belongs_to_bucket = false;
        for (let bucket of buckets) {
            //if they arent the same material, we can skip it!
            if (bucket.type !== output[6]) {
                continue;
            }
            //for each thing in the bucket
            for (let element of bucket.blocks) {
                //if your touching it, then you belong to that bucket!
                if (calcTouching(element, output)) {
                    //track it goes into the bucket!
                    belongs_to_bucket = true;
                    //add it in
                    bucket.blocks.push(output);
                    //each bucket is uniquely ided by the topmost leftmost backmost blocks coords.
                    //check if need to adjust bucket key
                    let bx = bucket.top_left_back[0];
                    let by = bucket.top_left_back[1];
                    let bz = bucket.top_left_back[2];
                    let ox = output[0];
                    let oy = output[1];
                    let oz = output[2];
                    if (ox < bx) {
                        bucket.top_left_back = [output[0], output[1], output[2]];
                        bucket.key = [output[0], output[1], output[2]].join(",");
                        break;
                    }
                    if (ox === bx) {
                        if (oy < by) {
                            bucket.top_left_back = [output[0], output[1], output[2]];
                            bucket.key = [output[0], output[1], output[2]].join(",");
                            break;
                        }
                        if (oy === by) {
                            if (oz < bz) {
                                bucket.top_left_back = [output[0], output[1], output[2]];
                                bucket.key = [output[0], output[1], output[2]].join(",");
                                break;
                            }
                        }
                    }
                    break;
                }
            }
            if (belongs_to_bucket) {
                break;
            }
        }
        //if not in any buckets, create a new one!
        if (!belongs_to_bucket) {
            buckets.push({
                key: [output[0], output[1], output[2]].join(","),
                top_left_back: [output[0], output[1], output[2]],
                type: output[6],
                blocks: [output],
            });
        }
    }
    //we might have two buckets that are touching
    //but we added the blocks to the list in the wrong order
    //we can merge some buckets
    let gone_full_loop_without_merging = false;
    do {
        gone_full_loop_without_merging = true;
        //loop over pairs of buckets
        for (let bucket1 of buckets) {
            for (let bucket2 of buckets) {
                //but not the same bucket
                if (bucket1.key === bucket2.key) {
                    continue;
                }
                //if not same type, cant merge
                if (bucket1.type !== bucket2.type) {
                    continue;
                }
                //loop over each of the blocks in each bucket
                for (block1 of bucket1.blocks) {
                    for (block2 of bucket2.blocks) {
                        //if they are touching
                        if (calcTouching(block1, block2)) {
                            //merge them!
                            //Remove bucket2 from the buckets array as we are merging it into buckets 1
                            for (let bucketIndex = 0; bucketIndex < buckets.length; bucketIndex += 1) {
                                if (buckets[bucketIndex].key == bucket2.key) {
                                    buckets.splice(bucketIndex, 1);
                                    break;
                                }
                            }
                            //weve merged now, might need to merge again!
                            gone_full_loop_without_merging = false;
                            //concat all the blocks
                            bucket1.blocks = bucket1.blocks.concat(bucket2.blocks);
                            let b1x = bucket1.top_left_back[0];
                            let b1y = bucket1.top_left_back[1];
                            let b1z = bucket1.top_left_back[2];
                            let b2x = bucket2.top_left_back[0];
                            let b2y = bucket2.top_left_back[1];
                            let b2z = bucket2.top_left_back[2];
                            //update key of bucket if necessary
                            if (b2x < b1x) {
                                bucket1.top_left_back = bucket2.top_left_back;
                                bucket1.key = bucket2.key;
                            }
                            if (b2x === b1x) {
                                if (b2y < b1y) {
                                    bucket1.top_left_back = bucket2.top_left_back;
                                    bucket1.key = bucket2.key;
                                }
                                if (b2y === b1y) {
                                    if (b2z < b1z) {
                                        bucket1.top_left_back = bucket2.top_left_back;
                                        bucket1.key = bucket2.key;
                                    }
                                }
                            }
                            break;
                        }
                    }
                    if (gone_full_loop_without_merging === false) {
                        break;
                    }
                }
                if (gone_full_loop_without_merging === false) {
                    break;
                }
            }
            if (gone_full_loop_without_merging === false) {
                break;
            }
        }
    } while (gone_full_loop_without_merging === false);
    //if we went a full loop without merging
    //then theres no more blocks to merge!
    //we can return our buckets.
    return buckets;
}

//takes in list of buckets
function optimiseBuckets(input_buckets) {
    //flatten all our arrays
    let values = input_buckets.flat(Infinity);
    let output_buckets = [];
    //loop over all our inputs
    for (let value of values) {
        //if we havent already seen the key
        if (!output_buckets.map((b) => b.key).includes(value.key)) {
            //add it to our output buckets
            output_buckets.push(value);
        } else {
            //otherwise, find the key in our output buckets
            let b = output_buckets.find((b) => b.key == value.key);
            //and if necessary, replace it's blocks with the better one
            if (b.blocks.length > value.blocks.length) {
                b.blocks = value.blocks;
            }
        }
    }
    return output_buckets;
}

//sort options from biggest to smallest
function sortOptionsByLargest(a, b) {
    const sortedA = a.slice().sort((x, y) => x - y);
    const sortedB = b.slice().sort((x, y) => x - y);

    for (let i = 0; i < 3; i++) {
        if (sortedA[i] < sortedB[i]) return -1;
        if (sortedA[i] > sortedB[i]) return 1;
    }
    return 0;
}
//copy an array of options
function copyOptions(options) {
    let new_options = [];
    for (let i = 0; i < options.length; i++) {
        let x = [];
        for (let j = 0; j < options[i].length; j++) {
            x.push(options[i][j]);
        }
        new_options.push(x);
    }
    return new_options;
}

//get all the different ways to iterate over a cube for greedy (3d case)
function getOptions(parent_x, parent_y, parent_z) {
    let options = [];
    for (let z = 0; z < parent_z; z++) {
        for (let y = 0; y < parent_y; y++) {
            for (let x = 0; x < parent_x; x++) {
                options.push([x, y, z]);
            }
        }
    }

    let xyz = copyOptions(options);
    let xzy = copyOptions(options);
    let yxz = copyOptions(options);
    let yzx = copyOptions(options);
    let zxy = copyOptions(options);
    let zyx = copyOptions(options);
    //sort all of them, then tie break based of char order.
    xyz.sort((a, b) => {
        let x = sortOptionsByLargest(a, b);
        if (x !== 0) {
            return x;
        }
        if (a[0] < b[0]) {
            return -1;
        }
        if (a[0] > b[0]) {
            return 1;
        }
        if (a[1] < b[1]) {
            return -1;
        }
        if (a[1] > b[1]) {
            return 1;
        }
        if (a[2] < b[2]) {
            return -1;
        }
        if (a[2] > b[2]) {
            return 1;
        }
        return 0;
    });
    xzy.sort((a, b) => {
        let x = sortOptionsByLargest(a, b);
        if (x !== 0) {
            return x;
        }
        if (a[0] < b[0]) {
            return -1;
        }
        if (a[0] > b[0]) {
            return 1;
        }
        if (a[2] < b[2]) {
            return -1;
        }
        if (a[2] > b[2]) {
            return 1;
        }
        if (a[1] < b[1]) {
            return -1;
        }
        if (a[1] > b[1]) {
            return 1;
        }
        return 0;
    });
    yxz.sort((a, b) => {
        let x = sortOptionsByLargest(a, b);
        if (x !== 0) {
            return x;
        }
        if (a[1] < b[1]) {
            return -1;
        }
        if (a[1] > b[1]) {
            return 1;
        }
        if (a[0] < b[0]) {
            return -1;
        }
        if (a[0] > b[0]) {
            return 1;
        }
        if (a[2] < b[2]) {
            return -1;
        }
        if (a[2] > b[2]) {
            return 1;
        }
        return 0;
    });
    yzx.sort((a, b) => {
        let x = sortOptionsByLargest(a, b);
        if (x !== 0) {
            return x;
        }
        if (a[1] < b[1]) {
            return -1;
        }
        if (a[1] > b[1]) {
            return 1;
        }
        if (a[2] < b[2]) {
            return -1;
        }
        if (a[2] > b[2]) {
            return 1;
        }
        if (a[0] < b[0]) {
            return -1;
        }
        if (a[0] > b[0]) {
            return 1;
        }
        return 0;
    });
    zxy.sort((a, b) => {
        let x = sortOptionsByLargest(a, b);
        if (x !== 0) {
            return x;
        }
        if (a[2] < b[2]) {
            return -1;
        }
        if (a[2] > b[2]) {
            return 1;
        }
        if (a[0] < b[0]) {
            return -1;
        }
        if (a[0] > b[0]) {
            return 1;
        }
        if (a[1] < b[1]) {
            return -1;
        }
        if (a[1] > b[1]) {
            return 1;
        }

        return 0;
    });
    zyx.sort((a, b) => {
        let x = sortOptionsByLargest(a, b);
        if (x !== 0) {
            return x;
        }
        if (a[2] < b[2]) {
            return -1;
        }
        if (a[2] > b[2]) {
            return 1;
        }
        if (a[1] < b[1]) {
            return -1;
        }
        if (a[1] > b[1]) {
            return 1;
        }
        if (a[0] < b[0]) {
            return -1;
        }
        if (a[0] > b[0]) {
            return 1;
        }

        return 0;
    });

    return [xyz, xzy, yxz, yzx, zxy, zyx];
}

module.exports = {
    extractBlock,
    createBuckets,
    optimiseBuckets,
    getOptions,
};
