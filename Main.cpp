#include <iostream>
#include <cctype>

enum class TokenType : int {

    OPERATOR,       //0
    NUMBER,         //1
    END_OF_FILE,    //2
    INVALID         //3

};

struct Token {

    TokenType type;
    
    union {

        char character;
        int number;

    };

    Token(TokenType type) : type(type), character('\0') {};
    Token(TokenType type, char character) : type(type), character(character) {};
    Token(TokenType type, int number) : type(type), number(number) {};

};

char* input;

Token nextToken() {

    while(isspace(*input)) input++;
    
    switch(*input) {

        case '\0': {

            return Token(TokenType::END_OF_FILE);

        }

        case '+':
        case '-':
        case '*':
        case '/': {

            return Token(TokenType::OPERATOR, *(input++));

        }

        default: {

            if(isdigit(*input)) {

                int number = 0;

                while(isdigit(*input)) {

                    number *= 10;
                    number += (*input - '0');

                    input++;

                }

                
                return Token(TokenType::NUMBER, number);

            }

            return Token(TokenType::INVALID);

        }

    }

}

struct TreeNode {
    
    TreeNode* left;
    char operation;

    union {

        TreeNode* right;
        int number;

    };

    TreeNode(int number) : left(nullptr), operation('\0'), number(number) {};
    TreeNode(TreeNode* left, char operation, TreeNode* right) : left(left), operation(operation), right(right) {};

};

int representation_offset = 0;

std::ostream& operator<<(std::ostream& stream, const TreeNode& tree) {

    stream << std::string(representation_offset, ' ') << "> ";

    if(tree.operation) {

        stream << tree.operation << '\n';

        representation_offset += 2;
        stream << *tree.left;
        stream << *tree.right;
        representation_offset -= 2;

    } else {

        stream << tree.number << '\n';

    }

    return stream;

}


int getPrecedence(char operation) {

    switch(operation) {

        case '+': case '-': return 1;
        case '*': case '/': return 2;

        default: return 0;

    }

}

TreeNode* parse(int previous_precedence) {

    Token primary = nextToken();
    if(primary.type != TokenType::NUMBER) {

        std::cerr << "Exspected Primary!";
        exit(-1);

    }

    TreeNode* left = new TreeNode(primary.number);

    while(true) {

        Token operation = nextToken();
        if(operation.type != TokenType::OPERATOR) {

            if(operation.type == TokenType::END_OF_FILE) break;

            std::cerr << "Exspected another operator or end!\n";
            exit(-1);

        }

        int precedence = getPrecedence(operation.character);
        if(!precedence || (precedence <= previous_precedence)) break;

        TreeNode* right = parse(precedence);
        left = new TreeNode(left, operation.character, right);

    }

    return left;

}

int main() {
    
    char* input_fixed = new char[64];

    while(true) {

        std::cout << ">> ";
        std::cin.getline(input_fixed, 64);

        if(!strcmp(input_fixed, "exit")) break;

        input = input_fixed;

        TreeNode* tree = parse(0);
        std::cout << *tree;

    }

    delete[] input;

}
