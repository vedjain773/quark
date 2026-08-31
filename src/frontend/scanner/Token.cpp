#include "frontend/scanner/Token.hpp"
#include <iostream>
#include <array>

std::unordered_map<std::string, TokenType> keywords = {
    {"int", TokenType::INT},           {"uint8_t", TokenType::UINT8},
    {"uint16_t", TokenType::UINT16},   {"char", TokenType::CHAR},
    {"void", TokenType::VOID},         {"if", TokenType::IF},
    {"else", TokenType::ELSE},         {"while", TokenType::WHILE},
    {"for", TokenType::FOR},           {"return", TokenType::RETURN},
    {"struct", TokenType::STRUCT},     {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE}, {"sizeof", TokenType::SIZEOF}};

constexpr std::array<std::string, 50> TokenTypeNames {
    // keywords
    "INT", "UINT8", "UINT16", "CHAR", "VOID", "IF", "ELSE", "WHILE", "FOR",
    "RETURN", "STRUCT", "BREAK", "CONTINUE", "SIZEOF",

    // access
    "DOT", "ARROW",

    // unary
    "BANG", "AMPERSAND",

    // operators
    "PLUS", "PLUS_EQUALS", "MINUS", "MINUS_EQUALS", "ASTERISK",
    "ASTERISK_EQUALS", "SLASH", "SLASH_EQUALS", "MODULUS",
    "MODULUS_EQUALS", "EQUALS", "LESS_THAN", "GREATER_THAN",

    "EQUALS_EQUALS", "BANG_EQUALS", "LESS_EQUALS", "GREATER_EQUALS",

    "AND", "OR",

    // delimiters
    "LEFT_CURLY", "RIGHT_CURLY", "LEFT_ROUND", "RIGHT_ROUND",
    "LEFT_SQUARE", "RIGHT_SQUARE", "COMMA", "SEMICOLON",

    // literals
    "IDENTIFIER", "INTEGER", "CHARACTER", "STRING",

    "END_OF_FILE"
};

bool isTypeStarter(TokenType tokentype) {
    switch (tokentype) {
        case TokenType::INT:
        case TokenType::CHAR:
        case TokenType::STRUCT:
        case TokenType::UINT8:
        case TokenType::UINT16:
        case TokenType::VOID:
            return true;
            break;

        default:
            return false;
    }
}

Token::Token(TokenType token_type, const std::string &lex, int line_num, int col_num) {
    tokentype = token_type;
    lexeme = lex;
    line = line_num;
    column = col_num;
}

std::string Token::getTokenStr() {
    return TokenTypeNames[static_cast<size_t>(tokentype)];
}

void Token::printToken(tabulate::Table &table) {
    table.add_row({lexeme, getTokenStr(), std::to_string(line), std::to_string(column)});
}
