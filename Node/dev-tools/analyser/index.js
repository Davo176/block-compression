const fs = require("fs");

let dict1 = {};
let dict2 = {};
//first arg is path to your input file
//cause need parent block size
fs.readFile(process.argv[2], "utf8", (err, data) => {
    if (err) {
        console.log(err);
    }
    const file = data.split("\n");
    //get parent block size
    [x_count, y_count, z_count, parent_x, parent_y, parent_z] = file
        .shift()
        .trim()
        .split(",")
        .map((e) => parseInt(e));
    //then file 1, aka left
    fs.readFile(process.argv[3], "utf8", (err, data2) => {
        const file1 = data2.split("\n");
        //loop over file and count up how many blocks you compressed each parent block to.
        while (file1.length > 0) {
            line = file1[0];
            if (!line) {
                break;
            }
            const [x_coord, y_coord, z_coord, x_size, y_size, z_size, label] = file1
                .shift()
                .trim()
                .split(",")
                .map((e) => (parseInt(e) >= 0 ? parseInt(e) : e));
            let x_main = x_coord - (x_coord % parent_x);
            let y_main = y_coord - (y_coord % parent_y);
            let z_main = z_coord - (z_coord % parent_z);
            //add that to a dict
            dict1[`${x_main},${y_main},${z_main}`] = dict1[`${x_main},${y_main},${z_main}`] + 1 || 1;
        }
        //then file 2, aka right
        //same process.
        fs.readFile(process.argv[4], "utf8", (err, data1) => {
            const file2 = data1.split("\n");
            while (file2.length > 0) {
                line = file2[0];
                if (!line) {
                    break;
                }
                const [x_coord, y_coord, z_coord, x_size, y_size, z_size, label] = file2
                    .shift()
                    .trim()
                    .split(",")
                    .map((e) => (parseInt(e) >= 0 ? parseInt(e) : e));
                let x_main = x_coord - (x_coord % parent_x);
                let y_main = y_coord - (y_coord % parent_y);
                let z_main = z_coord - (z_coord % parent_z);
                dict2[`${x_main},${y_main},${z_main}`] = dict2[`${x_main},${y_main},${z_main}`] + 1 || 1;
            }
            //then for everything in our dict (should be same parent blocks in both dicts.)
            for (let key of Object.keys(dict1)) {
                //if one is better than the other, let me know.
                if (dict1[key] < dict2[key]) {
                    console.log("Left", key, dict1[key], dict2[key]);
                }
                if (dict1[key] > dict2[key]) {
                    console.log("Right", key, dict1[key], dict2[key]);
                }
            }
        });
    });
});
