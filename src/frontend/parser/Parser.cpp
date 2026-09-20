#include "frontend/parser/Parser.hpp"
#include "nodes/Program.hpp"
#include "utils/Error.hpp"
#include "visitors/Visitor.hpp"
#include <iostream>

BinOpPrec getBinPrecedence(Operators Op) {
    switch (Op) {
        case Operators::MULT:
        case Operators::DIVIDE:
        case Operators::MODULUS:
            return FACTOR;
            break;

        case Operators::PLUS:
        case Operators::MINUS:
            return TERM;
            break;

        case Operators::GREATER:
        case Operators::GREATER_EQUALS:
        case Operators::LESS:
        case Operators::LESS_EQUALS:
            return COMP;
            break;

        case Operators::EQUALS:
        case Operators::NOT_EQUALS:
            return COMP_EQL;
            break;

        case Operators::AND:
            return LAND;
            break;

        case Operators::OR:
            return LOR;
            break;

        default:
            return MISC;
    }
}

bool isPostFixOp(TokenType tokenType) {
    switch (tokenType) {
        case TokenType::LEFT_ROUND:
        case TokenType::LEFT_SQUARE:
        case TokenType::DOT:
        case TokenType::ARROW:
            return true;
            break;

        default:
            return false;
    }
}

bool isAssignOp(TokenType tokenType) {
    switch (tokenType) {
        case TokenType::EQUALS:
        case TokenType::PLUS_EQUALS:
        case TokenType::MINUS_EQUALS:
        case TokenType::ASTERISK_EQUALS:
        case TokenType::SLASH_EQUALS:
        case TokenType::MODULUS_EQUALS:
            return true;

        default:
            return false;
    }
}

std::tuple<TypeKind *, std::string> Parser::ParseTypePrefix() {
    std::string typeName = "";

    if (peekCurr().tokentype == TokenType::STRUCT) {
        typeName += peekCurr().lexeme + " ";
        getNextToken();
    }

    typeName += peekCurr().lexeme;

    getNextToken();

    TypeKind *typek = getType(typeName);

    while (peekCurr().tokentype == TokenType::ASTERISK) {
        typeName += '*';
        typek = getType(typeName);

        getNextToken();
    }

    return std::make_tuple(typek, typeName);
}

TypeKind *Parser::ParseTypeSuffix(TypeKind *typek, std::string &typeName) {
    TypeKind *suffixType = typek;

    while (peekCurr().tokentype == TokenType::LEFT_SQUARE) {
        getNextToken();

        auto iExpr = ParseIntExpr();
        IntExpr *intExpr = static_cast<IntExpr *>(iExpr.get());

        if (peekCurr().tokentype != TokenType::RIGHT_SQUARE) {
            expect(peekCurr(), "Expected ']'");
        }

        getNextToken();
        suffixType = getArrType(typeName, intExpr->Val);
        typeName = suffixType->name;
    }

    return suffixType;
}

TypeKind *Parser::ParseType() {
    auto [prefixType, typeName] = ParseTypePrefix();
    return ParseTypeSuffix(prefixType, typeName);
}

std::tuple<TypeKind *, std::string, int, int> Parser::getTypeNamePair() {
    auto [typek, typeName] = ParseTypePrefix();

    if (peekCurr().tokentype != TokenType::IDENTIFIER) {
        expect(peekCurr(), "Expected identifier");
    }

    std::string varname = peekCurr().lexeme;

    int tline = peekCurr().line;
    int tcol = peekCurr().column;

    getNextToken();

    TypeKind *suffixType = ParseTypeSuffix(typek, typeName);

    return std::make_tuple(suffixType, varname, tline, tcol);
};

void Parser::expect(const Token &token, const std::string &msg) {
    Error error(token.line, token.column, msg);
    numOfErrors += 1;
    advToSyncPoint();
}

bool Parser::expectAndConsume(TokenType tokenType, const std::string &msg) {
    if (peekCurr().tokentype != tokenType) {
        Error error(peekCurr().line, peekCurr().column, msg);
        numOfErrors += 1;
        advToSyncPoint();
        return false;
    } else {
        getNextToken();
        return true;
    }
}

void Parser::advToSyncPoint() {
    auto isSyncPoint = [](const Token &token) {
        switch (token.tokentype) {
            case TokenType::SEMICOLON:
            case TokenType::LEFT_CURLY:
            case TokenType::RIGHT_CURLY:
            case TokenType::INT:
            case TokenType::CHAR:
            case TokenType::STRUCT:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::END_OF_FILE:
                return true;

            default:
                return false;
        }
    };

    while (!isSyncPoint(peekCurr()))
        getNextToken();
}

Parser::Parser(std::vector<Token> tokenlist)
    : TokenList(tokenlist),
      current(0) {}

Token Parser::getNextToken() {
    if (current < TokenList.size()) {
        return TokenList[current++];
    } else {
        return Token(TokenType::END_OF_FILE, "", 0, 0);
    }
}

Token Parser::peekCurr() {
    return TokenList[current];
}

Token Parser::peekNext() {
    return TokenList[current + 1];
}

Token Parser::peekPrev() {
    return TokenList[current - 1];
}

Token Parser::peekAhead(int n) {
    return TokenList[current + n];
}

std::unique_ptr<Expression> Parser::ParseDeclExpr() {
    int lastTokenLine, lastTokenCol;
    std::unique_ptr<Expression> expr;

    if (peekCurr().tokentype == TokenType::EQUALS) {
        getNextToken();
        expr = ParseExpr();

        lastTokenLine = expr->line;
        lastTokenCol = expr->column;
    } else {
        lastTokenLine = peekCurr().line;
        lastTokenCol = peekCurr().column;

        expr = nullptr;
    }

    if (peekCurr().tokentype != TokenType::SEMICOLON) {
        Error error(lastTokenLine, lastTokenCol, "Missing ';' after declaration");
        numOfErrors += 1;
        advToSyncPoint();
        return nullptr;
    }

    getNextToken();
    return expr; 
}

std::unique_ptr<ExternalDecl> Parser::ParseExDecl() {
    TypeNameInfo typeNameData = getTypeNamePair();
    auto [typek, name, tline, tcol] = typeNameData;

    if (peekCurr().tokentype == TokenType::LEFT_ROUND) {
        auto proto = ParsePrototype(typeNameData); 
        auto block = ParseBlockStmt();

        return std::make_unique<FuncDef>(std::move(proto), std::move(block));
    } else {
        auto expr = ParseDeclExpr();
        return std::make_unique<GlobalDecl>(typek, name, std::move(expr), tline, tcol);
    }
}

std::unique_ptr<Program> Parser::ParseProgram() {
    auto program = std::make_unique<Program>();

    while (peekCurr().tokentype != TokenType::END_OF_FILE) {

        if (peekAhead(2).tokentype == TokenType::LEFT_CURLY) {
            auto edecl = ParseStructDecl();
            program->add(std::move(edecl));
        } else {
            auto edecl = ParseExDecl();
            program->add(std::move(edecl));
        }
    }

    return program;
}
