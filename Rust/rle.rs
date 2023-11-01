use std::io;
use std::collections::HashMap;


fn main() {
    let mut input = String::new();

    // Read a line of input
    io::stdin().read_line(&mut input).expect("Failed to read input");

    // Split the input line into individual values
    let values: Vec<f64> = input
        .trim()
        .split(',')
        .map(|s| s.trim().parse::<f64>().expect("Invalid input"))
        .collect();
    
    // Ensure we have exactly six values
    if values.len() != 6 {
        panic!("Header must contain six values");
    }
    let px = values[3] as usize;

    let mut dictionary: HashMap<char, String> = HashMap::new();
    let mut input = String::new();

    // Read lines until a blank line is encountered
    loop {
        input.clear();
        io::stdin().read_line(&mut input).expect("Failed to read input");

        // Trim whitespace from the input line
        let trimmed_input = input.trim();

        // If the line is blank, exit the loop
        if trimmed_input.is_empty() {
            break;
        }

        // Split the line into key and value parts
        let parts: Vec<&str> = trimmed_input.split(',').map(|s| s.trim()).collect();

        // Ensure each line has two parts
        if parts.len() == 2 {
            if let Some(key) = parts[0].chars().next() {
                dictionary.insert(key, parts[1].to_string());
            }
        }
    }

    let mut x = 0;
    let mut y = 0;
    let mut z = 0;

    // While there is a new line in stdin
    while io::stdin().read_line(&mut input).expect("Failed to read additional lines") > 0 {
        // Check if line is empty if so move to next z
        if input.trim().is_empty() {
            z += 1;
            y = 0;
            continue;
        }

        input = input.trim().to_string();
        let mut previous_char: char = input.chars().next().unwrap();
        let mut current_length = 0;
        let mut start_x = x;

        // For each char in the line
        for current_char in input.chars() {
            // If the next block does not match the current contigious block, print the current block and start a new one.
            if (current_char != previous_char) || ((x%px == 0) && (x != 0)) {
                println!("{},{},{},{},{},{},{}",start_x,y,z,current_length,1,1,dictionary[&previous_char]);
                current_length = 1;
                previous_char = current_char;
                start_x = x;
            }
            // Else add block to current block and move along
            else {
                current_length += 1;
                previous_char = current_char
            }
            x += 1;
        }
        // Print the final encoded block
        println!("{},{},{},{},{},{},{}",start_x,y,z,current_length,1,1,dictionary[&previous_char]);
        x = 0;
        y += 1;
        input.clear();
    }
}