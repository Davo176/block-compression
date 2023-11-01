FLAGS = -Wall -Wextra -pedantic -Wconversion -static

default:
	gcc rle.c -o cRLE.exe ${FLAGS}

trle:
	gcc main.c trle.c IO.c ${FLAGS} -o trle.exe

no_compression:
	./main.exe no_compression < ../Inputs/input.txt > ../Outputs/output2.txt
