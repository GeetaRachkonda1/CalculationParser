#pragma once
#ifndef CALCULATION_PARSER_H
#define CALCULATION_PARSER_H
	
#include <cctype>
#include <exception>
#include <iostream>
	
	enum class TokenType : char {
		END_OF_FILE = '\0',
		OPEN_PARENTHESIS = '(',
		CLOSE_PARENTHESIS = ')',
	
		NUMBER,
		BINARY_OPERATOR
	};
	
	struct Token {
		TokenType type;
	
		union {
			int integer; //TODO: Allow floating-point numbers
			char character;
		};
	
		Token(TokenType type) : type(type) {};
		Token(TokenType type, int integer) : type(type), integer(integer) {};
		Token(TokenType type, char character) : type(type), character(character) {};
	};
	
	static std::ostream& operator<<(std::ostream& stream, const Token& token) {
		stream << '[';
	
		switch(token.type) {
			case TokenType::END_OF_FILE:	   return stream << "EOF]";
			case TokenType::OPEN_PARENTHESIS:  return stream << "(]";
			case TokenType::CLOSE_PARENTHESIS: return stream << ")]";
	
			case TokenType::BINARY_OPERATOR:   return stream << "Binary Operator: " << (char) token.character << ']';
			case TokenType::NUMBER:			   return stream << "Number: " << token.integer << ']';
		}
	}
	
	enum class TreeType {
		BINARY_OPERATION,
		NUMBER
	};
	
	struct Tree {
		TreeType type;
		Tree* children[2]; //TODO: Make these 2 pointers somehow optional
	
		union {
			int integer;
			char character;
		};
	
		Tree(TreeType type) : type(type) {};
		Tree(int integer) : type(TreeType::NUMBER), integer(integer) {};
		Tree(Tree* left, char character, Tree* right) : type(TreeType::BINARY_OPERATION), character(character) {
			children[0] = left;
			children[1] = right;
		};
	
		int solve() const noexcept {
			if(type == TreeType::NUMBER) return integer;
	
			int left = children[0]->solve();
			int right = children[1]->solve();
	
			switch(character) { //Apply the specified operator to the values
				case '+':  return (left + right);
				case '-':  return (left - right);
				case '*':  return (left * right);
				case '/':  return (left / right);
				case '%':  return (left % right);
				case '&':  return (left & right);
				case '^':  return (left ^ right);
				case '|':  return (left | right);
				case '\'': return (left && right);
				case '}':  return (left || right);
			}
		}
	
		void print(std::ostream& stream = std::cout, int representation_offset = 0) {
			stream << std::string(representation_offset, ' ') << "> ";
	
			if(type == TreeType::BINARY_OPERATION) {
				stream << (char) character << '\n';
	
				representation_offset += 2;
				children[0]->print(stream, representation_offset);
				children[1]->print(stream, representation_offset);
			} else {
				stream << integer << '\n';
			}
		}
	};

	struct CalculationParser {
		static int getPrecedence(char operation) {
			switch(operation) {
				case '*': case '/': case '%': return 8;
				case '+': case '-':			  return 7;
				case '&':					  return 6;
				case '^':					  return 4;
				case '|':					  return 3;
				case '\'':					  return 2; //=('&' + 1)
				case '}':					  return 1; //=('|' + 1)

				default: return 0;
			}
		}

		static bool getAssociativity(char operation) {
			switch(operation) {
				case '*': case '/': case '%': case '+': case '-': case '&': case '^': case '|': case '\'': case '}': return true; //Left-to-right
				default: return false; //Right-to-left
			}
		}

		char* input;

		CalculationParser()							  {};
		CalculationParser(char* input) : input(input) {};

		Token nextToken() {
			while(isspace(*input)) input++; //Skip all whitespaces not coresponding to a token

			switch(*input) {
				case '\0': return Token(TokenType::END_OF_FILE);

				case ')':
				case '(': return Token((TokenType) * (input++));

				case '+':
				case '-':
				case '*':
				case '/':
				case '%':
				case '^': return Token(TokenType::BINARY_OPERATOR, *(input++)); //Single-character operators

				#ifdef UNICODE
					case '&':
					case '|': {
						if(*input == *(input + 1)) {
							input++;
							return Token(TokenType::BINARY_OPERATOR, (char) (*(input++) + 1));
						}
						return Token(TokenType::BINARY_OPERATOR, *(input++));
					}
				#else
					case '&': {
						if(*(++input) == '&') {
							input++;
							return Token(TokenType::BINARY_OPERATOR, '\'');	//=('&' + 1)
						}

						return Token(TokenType::BINARY_OPERATOR, '&');
					}

					case '|': {
						if(*(++input) == '|') {
							input++;
							return Token(TokenType::BINARY_OPERATOR, '}');	//=('|' + 1)
						}

						return Token(TokenType::BINARY_OPERATOR, '|');
					}
				#endif

				default: { //Non single-character Tokens
					if(isdigit(*input)) {
						int primary = (*input - '0');

						while(isdigit(*(++input))) {
							primary *= 10;

							#ifdef UNICODE	//Convert character to representing digit
								primary += (*input - '0');
							#elif
								switch(*input) {
									case '1': { primary += 1; } break;
									case '2': { primary += 2; } break;
									case '3': { primary += 3; } break;
									case '4': { primary += 4; } break;
									case '5': { primary += 5; } break;
									case '6': { primary += 6; } break;
									case '7': { primary += 7; } break;
									case '8': { primary += 8; } break;
									case '9': { primary += 9; } break;
								}
							#endif
						}

						return Token(TokenType::NUMBER, primary);
					}

					throw std::exception("Unknown token!");
				}
			}
		};

		Tree* parseSub(int previous_precedence) throw(std::exception) {
			char* previous_input = input;
			Token primary = nextToken();

			Tree* left;

			switch(primary.type) {
				case TokenType::NUMBER: {
					left = new Tree(primary.integer);
					break;
				}

				case TokenType::OPEN_PARENTHESIS: {
					left = parseSub(0); //Parse this sub-part as another calculation

					if(nextToken().type != TokenType::CLOSE_PARENTHESIS) { //Check for closing parenthesis
						throw std::exception("Exspected closing Parenthesis!");
					}

					break;
				}

				default: {
					throw std::exception("Exspected primary!");
				}
			}

			while(true) {
				previous_input = input;

				Token operation = nextToken();
				int precedence = getPrecedence(operation.character);

				if((operation.type != TokenType::BINARY_OPERATOR) || !precedence || (getAssociativity(operation.character) ? (precedence <= previous_precedence) : (precedence < previous_precedence))) { //Check type, precedence and associativity
					input = previous_input; //Unget the operator-token
					break;
				}

				left = new Tree(left, operation.character, parseSub(precedence));
			}

			return left;
		}

		Tree* parse() {
			Tree* result = parseSub(0);

			if(nextToken().type != TokenType::END_OF_FILE) {
				throw std::exception("Exspected operator!");
			}

			return result;
		}
	};
#endif