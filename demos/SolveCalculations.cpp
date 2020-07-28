#include <iostream>
#include "../CalculationParser.h"

int main() {
	char input[64]; //Create input buffer

	while(true) {
		std::cout << ">> ";
		std::cin.getline(input, 64);

		try {
			CalculationParser parser;
			parser.input = input;

			Tree* tree = parser.parse();
			std::cout << tree->solve() << "\n\n";
		} catch(std::exception exception) {
			std::cerr << exception.what() << "\n\n";
		}
	}
}