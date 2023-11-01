param(
    [string]$executablePath
)

# Check if $executablePath is not provided
if (-not $executablePath) {
    Write-Output "Error: Please provide the path to the executable using '-executablePath .\path\to\exe.exe'."
    Exit 1
}

# Get the filename from the executable path
$filename = Split-Path -Leaf $executablePath

# Function to split and add a comma between two values
function SplitAndAddComma($value) {
    $values = $value.Split(' ')
    return "$($values[0]), $($values[1])"
}

$outputIntro = python3 .\2023_runnerB.py -s $executablePath .\Inputs\the_intro_one_32768_4x4x4.csv
Write-Output ($filename + ", The Intro One, " + (SplitAndAddComma $outputIntro))
$outputFast = python3 .\2023_runnerB.py -s $executablePath .\Inputs\the_fast_one_376000_2x2x2.csv
Write-Output ($filename + ", The Fast One, " + (SplitAndAddComma $outputFast))
$outputComb = python3 .\2023_runnerB.py -s $executablePath .\Inputs\the_combinatorial_one_42000000_14x10x12.csv
Write-Output ($filename + ", The Combinatorial One, " + (SplitAndAddComma $outputComb))

