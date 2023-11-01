# SEP - Block 6
## Motivation
Our software is a stream processor, it converts an input format to a specified output format. This needs to happen because storing a large number of individual blocks requires a lot of space and computational power to process. The specified output format drastically reduces these two requirements. Allowing more efficient use in data visualisation, modelling and analysis algorithms, and reporting and data transfer. Specifically, given a 3-dimensional arrangement of unit sized cubes, we need to return the fewest number of larger cuboids (or rectangular prisms), all containing the same material.
## Git Branch Name Convetion:
When creating a new Git branch use the following template:

> {language}/{feature_name}

For example, if the branch pertains to creating C code to get and parse an input file:

> C/read-parse-input

## Git Commit Message Convetion:
Git commit messsages must contain 3 parts:
- Title/Summary
- Detailed Description
- Ticket/Bug/Issue ID

## Function Name Convetion:
Function names will use camel case:
> thisIsAFunction

## Variable Name Convetion:
Variable names will use snake case:
> this_is_a_variable

## Function Commenting Convetion:
Function comments will be as follows:
```C
/*
Function Name: xxxx
Function Arguments:
    Name: Description
Function Description:
    xxxx
Function Return:
    Name: Description
*/
```

## Pointer convention
Asterisk connected to the variable name, disconnected from type name.  
For example:  
`char *ptr;`  
not  
`char* ptr;`  

## User story boilerplate
[[title]]: [[time]]
As a user, I want to be able to:
* x
* y
* z
DoD: 
    This will be done when x

## JS Code Styling
Prettier is used. Install the prettier extension: https://marketplace.visualstudio.com/items?itemName=esbenp.prettier-vscode
add this to your vscode .settings file:
```json
"editor.defaultFormatter": "esbenp.prettier-vscode",
"[javascript]": {
    "editor.defaultFormatter": "esbenp.prettier-vscode",
    "editor.formatOnSave": true
}
```
prettierrc settings (dont need to do anything with this):
```json
{
    "tabWidth": 4,
    "printWidth": 500
}
```

## Algorithm Research
https://docs.google.com/document/d/1OxEdttA4rNf-VRusaADonLZHDT1XVDOwRN_kkWUwcD8/edit?pli=1
