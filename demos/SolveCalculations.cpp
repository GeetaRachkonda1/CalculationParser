#include <iostream>
#include "../CalculationParser.h"

using namespace CalculationParser;

int main() {

	char input[64]; //Create input buffer

	while(true) {

		std::cout << ">> ";
		std::cin.getline(input, 64);

		char* input_modifiable = input; //Create a copy of the pointer, that will later be offset

		BinaryTree* tree = Parser::parseCalculation(input_modifiable);
		std::cout << Parser::solve(*tree) << '\n';
		delete tree;

	}

}