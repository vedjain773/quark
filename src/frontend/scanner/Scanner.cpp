#include "frontend/scanner/Scanner.hpp"
#include "utils/Error.hpp"
#include <fstream>
#include <iostream>

Scanner::Scanner(std::string source_arg) :
    source(source_arg) {}

void Scanner::scanFile() {
    std::ifstream file(source);

    std::string text;

    while (getline(file, text)) {
        sourceContent = sourceContent + text + '\n';
    }

    file.close();
}

void Scanner::scanToken() {
    char curr = peekCurr();
    switch (curr) {
        case '{': {
            getNextChar();
            addToken(TokenType::LEFT_CURLY);
        } break;

        case '}': {
            getNextChar();
            addToken(TokenType::RIGHT_CURLY);
        } break;

        case '(': {
            getNextChar();
            addToken(TokenType::LEFT_ROUND);
        } break;

        case ')': {
            getNextChar();
            addToken(TokenType::RIGHT_ROUND);
        } break;

        case '[': {
            getNextChar();
            addToken(TokenType::LEFT_SQUARE);
        } break;

        case ']': {
            getNextChar();
            addToken(TokenType::RIGHT_SQUARE);
        } break;

        case '.': {
            getNextChar();
            addToken(TokenType::DOT);
        } break;

        case ',': {
            getNextChar();
            addToken(TokenType::COMMA);
        } break;

        case ';': {
            getNextChar();
            addToken(TokenType::SEMICOLON);
        } break;

        case '+': {
            getNextChar();
            if (peekCurr() == '=') {
                getNextChar();
                addToken(TokenType::PLUS_EQUALS);
            } else {
                addToken(TokenType::PLUS);
            }
        } break;

        case '-': {
            getNextChar();
            if (peekCurr() == '>') {
                getNextChar();
                addToken(TokenType::ARROW);
            } else if (peekCurr() == '=') {
                getNextChar();
                addToken(TokenType::MINUS_EQUALS);
            } else {
                addToken(TokenType::MINUS);
            }
        } break;

        case '*': {
            getNextChar();
            if (peekCurr() == '=') {
                getNextChar();
                addToken(TokenType::ASTERISK_EQUALS);
            } else {
                addToken(TokenType::ASTERISK);
            }
        } break;

        case '%': {
            getNextChar();
            if (peekCurr() == '=') {
                getNextChar();
                addToken(TokenType::MODULUS_EQUALS);
            } else {
                addToken(TokenType::MODULUS);
            }
        } break;

        case '=': {
            getNextChar();
            if (peekCurr() == '=') {
                getNextChar();
                addToken(TokenType::EQUALS_EQUALS);
            } else {
                addToken(TokenType::EQUALS);
            }
        } break;

        case '<': {
            getNextChar();
            if (peekCurr() == '=') {
                getNextChar();
                addToken(TokenType::LESS_EQUALS);
            } else {
                addToken(TokenType::LESS_THAN);
            }
        } break;

        case '>': {
            getNextChar();
            if (peekCurr() == '=') {
                getNextChar();
                addToken(TokenType::GREATER_EQUALS);
            } else {
                addToken(TokenType::GREATER_THAN);
            }
        } break;

        case '!': {
            getNextChar();
            if (peekCurr() == '=') {
                getNextChar();
                addToken(TokenType::BANG_EQUALS);
            } else {
                addToken(TokenType::BANG);
            }
        } break;

        case '&': {
            getNextChar();
            if (peekCurr() == '&') {
                getNextChar();
                addToken(TokenType::AND);
            } else {
                addToken(TokenType::AMPERSAND);
            }
        } break;

        case '|': {
            getNextChar();
            if (peekCurr() == '|') {
                getNextChar();
                addToken(TokenType::OR);
            } else {
                Error error(line, tokStartCol, "Unexpected character: |");
                printErrorMsg(error);
            }
        } break;

        case '/': {
            getNextChar();
            if (peekCurr() == '/') {
                lookAhead('\n');
            } else if (peekCurr() == '=') {
                getNextChar();
                addToken(TokenType::SLASH_EQUALS);
            } else if (peekCurr() == '*') {
                bool asterisk = lookAhead('*');

                if (!asterisk) {
                    Error error(line, tokStartCol, "No matching * found");
                    printErrorMsg(error);
                }

                if (peekCurr() == '/') {
                    getNextChar();
                } else {
                    Error error(line, tokStartCol, "No matching / found");
                    printErrorMsg(error);
                }

            } else {
                addToken(TokenType::SLASH);
            }
        } break;

        case '\'': {
            getNextChar();

            // Consume the actual char
            getNextChar();

            // consume '

            if (peekCurr() != '\'') {
                Error error(line, tokStartCol, "No matching \' found");
                printErrorMsg(error);
            }

            getNextChar();
            addToken(TokenType::CHARACTER);
        } break;

        case '"': {
            bool termInvComma = lookAhead('"');

            if (termInvComma) {
                addToken(TokenType::STRING);
            } else {
                Error error(line, tokStartCol, "No matching \" found");
                printErrorMsg(error);
            }
        }

        case ' ': {
            getNextChar();
        }

        case '\r': {
            // ignore
        }

        case '\t': {
            // ignore
        } break;

        case '\n': {
            getNextLine();
        } break;

        default: {
            if (isDigit(curr)) {
                scanNumber();
            } else if (isAlpha(curr)) {
                scanWord();
            } else {
                Error error(line, tokStartCol, "Unexpected character");
                printErrorMsg(error);
            }
        }
    }
}

void Scanner::scanProg() {
    while (current < sourceContent.size()) {
        tokStartCol = column;
        start = current;
        scanToken();
    }

    Token token = Token(TokenType::END_OF_FILE, "", line, column);
    tokenList.push_back(token);
}

void Scanner::printTokens() {
    tabulate::Table table;
    table.add_row({"Token", "Type", "Line", "Column"});

    for (Token token : tokenList) {
        token.printToken(table);
    }

    std::cout << table << "\n";
}

char Scanner::peekCurr() {
    return sourceContent[current];
}

char Scanner::peekNext() {
    return sourceContent[current + 1];
}

void Scanner::getNextChar() {
    current++;
    column++;
}

void Scanner::getNextLine() {
    line++;
    current++;
    column = 1;
}

void Scanner::addToken(const TokenType &tokenType) {
    std::string lex = sourceContent.substr(start, current - start);
    Token token = Token(tokenType, lex, line, tokStartCol);
    tokenList.push_back(token);
}

bool Scanner::lookAhead(char expEnd) {
    bool isPresent = false;

    while (current < sourceContent.size()) {

        if (peekCurr() == expEnd) {
            isPresent = true;

            if (expEnd == '\n') line++;

            getNextChar();
            break;
        }

        if (peekCurr() == '\n') {
            getNextLine();
        }

        getNextChar();
    }

    return isPresent;
}

bool Scanner::isDigit(char c) {
    if (c >= '0' && c <= '9') {
        return true;
    } else {
        return false;
    }
}

bool Scanner::isAlpha(char c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
        return true;
    } else {
        return false;
    }
}

void Scanner::scanNumber() {
    while (isDigit(peekCurr())) {
        getNextChar();
    }

    addToken(TokenType::INTEGER);
}

void Scanner::scanWord() {
    while (isAlpha(peekCurr()) || isDigit(peekCurr())) {
        getNextChar();
    }

    std::string sub = sourceContent.substr(start, current - start);
    auto it = keywords.find(sub);

    if (it != keywords.end()) {
        addToken(keywords[sub]);
    } else {
        addToken(TokenType::IDENTIFIER);
    }
}

std::vector<Token> Scanner::getTokenList() {
    return tokenList;
}
