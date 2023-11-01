default:
	g++ ../main.cpp -o cppTagtableTest
test:
	./cppTagtableTest < ../../Inputs/input.txt > ../../Outputs/tagTable/tagtableOutput.txt
	diff ../../Outputs/tagTable/tagtableOutput.txt ../../Outputs/tagTable/exTagtableOutput.txt