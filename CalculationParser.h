#pragma once

#include <iostream>
#include <cctype>

namespace CalculationParser {

	enum class TokenType : char {

		INVALID = -1,		//A invalid Token
		END_OF_FILE = 0,	//The Null-Termination Character at the end of every file

		BINARY_OPERATOR,	//A binary operator (+,-,*,/,%,&,&&,|,||,^)
		NUMBER,				//A positive non-floating number (0,1,2,...)
		OPEN_PARENTHESIS,	//A opening parenthesis: (
		CLOSE_PARENTHESIS	//A closing patenthesis: )

	};

	struct Token {

		TokenType type;

		int number;

		Token(TokenType type) : type(type), number(0) {};
		Token(TokenType type, int number) : type(type), number(number) {};

	};

	static std::ostream& operator<<(std::ostream& stream, const Token& token) {

		stream << '[';

		switch(token.type) {

			case TokenType::INVALID: {

				stream << "INVALID";
				break;

			}

			case TokenType::END_OF_FILE: {

				stream << "EOF";
				break;

			}

			case TokenType::BINARY_OPERATOR: {

				stream << "Binary Operator: " << (char) token.number;
				break;

			}

			case TokenType::NUMBER: {

				stream << "Number: " << token.number;
				break;

			}

			case TokenType::OPEN_PARENTHESIS: {

				stream << '(';
				break;

			}

			case TokenType::CLOSE_PARENTHESIS: {

				stream << ')';
				break;

			}

		}

		stream << ']';
		return stream;

	}

	struct Lexer {

		static Token nextToken(char*& input) {

			while(isspace(*input)) input++; //Skip all spaces not coresponding to a token

			switch(*input) {

				case '\0': return Token(TokenType::END_OF_FILE); //Single-character tokens

				case '(': {

					input++;
					return Token(TokenType::OPEN_PARENTHESIS);

				}
				case ')': {
					input++;
					return Token(TokenType::CLOSE_PARENTHESIS);

				}

				case '+':
				case '-':
				case '*':
				case '/':
				case '%':
				case '^': return Token(TokenType::BINARY_OPERATOR, *(input++)); //Single-character operators

				case '&':
				case '|': { //Possible double-character operators

					if(*input == *(input++)) return Token(TokenType::BINARY_OPERATOR, (*(input++) + 256));
					return Token(TokenType::BINARY_OPERATOR, *input);

				}

				default: { //Non single-character Tokens

					if(isdigit(*input)) { //Check for number token

						int number = 0;

						while(isdigit(*input)) {

							number *= 10;
							number += (*input - '0'); //Convert character to representing digit

							input++;

						}

						return Token(TokenType::NUMBER, number);

					}

					return Token(TokenType::INVALID); //Invalid token

				}

			}

		}

	};

	struct BinaryTree {

		int operation; //The operator or Null to specify that this node contains a number

		union {

			BinaryTree* children[2]; //The 2 children (left and right) of the tree
			int number;

		};

		BinaryTree(int number) : operation(0), number(number) {};
		BinaryTree(BinaryTree* left, int operation, BinaryTree* right) : operation(operation) {

			children[0] = left;
			children[1] = right;

		}

		~BinaryTree() {

			if(operation) {

				delete children[0];
				delete children[1];

			}

		}

	};

	static std::ostream& operator<<(std::ostream& stream, const BinaryTree& tree);

	static std::ostream& printBinaryTree(std::ostream& stream, const BinaryTree& tree, int representation_offset) {

		stream << std::string(representation_offset, ' ') << "> ";

		if(tree.operation) {

			stream << (char) tree.operation << '\n';

			representation_offset += 2;
			stream << *tree.children[0];
			stream << *tree.children[1];

		} else {

			stream << tree.number << '\n';

		}

		return stream;

	}

	static std::ostream& operator<<(std::ostream& stream, const BinaryTree& tree) {

		return printBinaryTree(stream, tree, 0);

	}

	struct Parser {

		static int getPrecedence(int operation) {

			switch(operation) {

				case '*': case '/': case '%': return 7;
				case '+': case '-': return 6;
				case '&': return 5;
				case '|': return 4;
				case ('&' + 256): return 3;
				case '^': return 2;
				case ('|' + 256): return 1;

				default: return 0;

			}

		}

		static int binaryOperation(int left, int operation, int right) {

			switch(operation) { //Apply the operator to the values

				case '+': return (left + right);
				case '-': return (left - right);
				case '*': return (left * right);
				case '/': return (left / right);
				case '%': return (left % right);
				case '&': return (left & right);
				case '^': return (left ^ right);
				case '|': return (left | right);
				case ('&' + 256): return (left && right);
				case ('|' + 256): return (left || right);

			}

		}

		static BinaryTree* parseCalculation(char*& input) {

			BinaryTree* result = parseSubCalculation(input, 0);

			Token last = Lexer::nextToken(input);
			if(last.type != TokenType::END_OF_FILE) {

				throw std::exception("Exspected operator!");

			}

			return result;

		}

		static BinaryTree* parseSubCalculation(char*& input, int previous_precedence) {

			char* previous_input = input;
			Token primary = Lexer::nextToken(input);

			BinaryTree* left;

			switch(primary.type) {

				case TokenType::NUMBER: {

					left = new BinaryTree(primary.number);
					break;

				}

				case TokenType::OPEN_PARENTHESIS: {

					left = parseSubCalculation(input, 0); //Parse the in parenthesis enclosed part as a sub-calculation

					if(Lexer::nextToken(input).type != TokenType::CLOSE_PARENTHESIS) { //Check for closing parenthesis

						throw std::exception("Exspected closing Parenthesis!");

					}

					break;

				}

				case TokenType::BINARY_OPERATOR: {

					left = new BinaryTree(0);
					input = previous_input; //Undo the peeking of the operator-token
					break;

				}

				default: {

					throw std::exception("Exspected primary!");

				}

			}

			while(true) {

				previous_input = input;
				Token operation = Lexer::nextToken(input);
				int precedence = getPrecedence(operation.number);

				if((operation.type != TokenType::BINARY_OPERATOR) || !precedence || (precedence <= previous_precedence)) {

					input = previous_input; //Undo the peeking of the operator-token
					break;

				}

				left = new BinaryTree(left, operation.number, parseSubCalculation(input, precedence));

			}

			return left;

		}

		static int solve(const BinaryTree& tree) {

			if(!tree.operation) return tree.number;
			return Parser::binaryOperation(solve(*(tree.children[0])), tree.operation, solve(*(tree.children[1])));

		}

	};

}