#include "frontend/parser/Parser.hpp"
#include "nodes/Program.hpp"
#include "utils/Error.hpp"
#include "visitors/Visitor.hpp"

std::unique_ptr<Statement> Parser::ParseExprStmt() {
    std::unique_ptr<Expression> expr = ParseExpr();

    if (!expectAndConsume(TokenType::SEMICOLON, "Missing ';' after expression"))
        return nullptr;

    return std::make_unique<ExprStmt>(std::move(expr));
}

std::unique_ptr<BlockStmt> Parser::ParseBlockStmt() {
    getNextToken();
    auto Result = std::make_unique<BlockStmt>();

    while (peekCurr().tokentype != TokenType::RIGHT_CURLY) {
        auto stmt = ParseStmt();

        if (stmt)
            Result->addStmt(std::move(stmt));

        if (peekCurr().tokentype == TokenType::END_OF_FILE) {
            expect(peekCurr(), "Expected '}'");
            return nullptr;
        }
    }

    getNextToken();
    return Result;
}

std::unique_ptr<Statement> Parser::ParseIfStmt() {
    getNextToken();

    if (!expectAndConsume(TokenType::LEFT_ROUND, "Expected '(' after if"))
        return nullptr;

    auto condn = ParseExpr();

    if (!expectAndConsume(TokenType::RIGHT_ROUND, "Expected ')'"))
        return nullptr;

    auto ifbody = ParseStmt();

    auto elsestmt = ParseElseStmt();
    return std::make_unique<IfStmt>(std::move(condn), std::move(ifbody), std::move(elsestmt));
}

std::unique_ptr<Statement> Parser::ParseElseStmt() {
    if (peekCurr().tokentype != TokenType::ELSE)
        return nullptr;

    getNextToken();

    auto elsebody = ParseStmt();
    return std::make_unique<ElseStmt>(std::move(elsebody));
}

std::unique_ptr<Statement> Parser::ParseWhileStmt() {
    getNextToken();

    if (!expectAndConsume(TokenType::LEFT_ROUND, "Expected '(' after while"))
        return nullptr;

    auto condn = ParseExpr();

    if (!expectAndConsume(TokenType::RIGHT_ROUND, "Expected ')'"))
        return nullptr;

    auto whilebody = ParseStmt();
    return std::make_unique<WhileStmt>(std::move(condn), std::move(whilebody));
}

std::unique_ptr<Statement> Parser::ParseForStmt() {
    getNextToken();

    if (!expectAndConsume(TokenType::LEFT_ROUND, "Expected '(' after for"))
        return nullptr;

    std::unique_ptr<Statement> init;

    if (isTypeStarter(peekCurr().tokentype)) {
        init = ParseDeclStmt();
    } else if (peekCurr().tokentype == TokenType::SEMICOLON) {
        init = std::make_unique<EmptyStmt>();
        getNextToken();
    } else {
        init = ParseExprStmt();
    }

    std::unique_ptr<Expression> condn, iter;

    if (peekCurr().tokentype == TokenType::SEMICOLON) {
        condn = std::make_unique<EmptyExpr>();
        getNextToken();
    } else {
        condn = ParseExpr();

        if (!expectAndConsume(TokenType::SEMICOLON, "Expected ';'"))
            return nullptr;
    }

    if (peekCurr().tokentype == TokenType::RIGHT_ROUND) {
        iter = std::make_unique<EmptyExpr>();
        getNextToken();
    } else {
        iter = ParseExpr();

        if (!expectAndConsume(TokenType::RIGHT_ROUND, "Expected ')'"))
            return nullptr;
    }

    auto body = ParseStmt();
    return std::make_unique<ForStmt>(std::move(init), std::move(condn), std::move(iter),
                                     std::move(body));
}

std::unique_ptr<Statement> Parser::ParseReturnStmt() {
    getNextToken();

    std::unique_ptr<Statement> Result;

    if (peekCurr().tokentype == TokenType::SEMICOLON) {
        auto retexpr = std::make_unique<EmptyExpr>();
        retexpr->line = peekCurr().line;
        retexpr->column = peekCurr().column;
        Result = std::make_unique<ReturnStmt>(std::move(retexpr));
    } else {
        auto retexpr = ParseExpr();
        Result = std::make_unique<ReturnStmt>(std::move(retexpr));
    }

    getNextToken();
    return Result;
}

std::unique_ptr<Statement> Parser::ParseDeclStmt() {
    auto [typek, varname, tline, tcol] = getTypeNamePair();

    auto expr = ParseDeclExpr();
    return std::make_unique<DeclStmt>(typek, varname, std::move(expr), tline, tcol);
}

std::unique_ptr<StructField> Parser::ParseStructField() {
    auto [typek, fieldName, tline, tcol] = getTypeNamePair();

    return std::make_unique<StructField>(typek, fieldName, tline, tcol);
}

std::unique_ptr<StructDecl> Parser::ParseStructDecl() {
    int line = peekCurr().line;
    int column = peekCurr().column;
    getNextToken();

    std::string tag = peekCurr().lexeme;

    TypeKind *structType = createStructType(tag);

    auto Result = std::make_unique<StructDecl>(tag, line, column);
    getNextToken();

    if (!expectAndConsume(TokenType::LEFT_CURLY, "Expected '{'"))
        return nullptr;

    int offset = 0;

    while (peekCurr().tokentype != TokenType::RIGHT_CURLY) {
        auto structField = ParseStructField();

        int fieldAlign = structField->type->align;
        int fieldSize = structField->type->size;

        if (offset % fieldAlign != 0)
            offset += fieldAlign - offset % fieldAlign;

        offset += fieldSize;

        Result->addField(std::move(structField));
        getNextToken();
    }

    if (offset > 0)
        structType->size = offset;

    if (!expectAndConsume(TokenType::RIGHT_CURLY, "Expected '}'"))
        return nullptr;

    if (!expectAndConsume(TokenType::SEMICOLON, "Expected ';'"))
        return nullptr;

    return Result;
}

std::unique_ptr<Statement> Parser::ParseStmt() {
    switch (peekCurr().tokentype) {
        case TokenType::RIGHT_CURLY: {
            expect(peekCurr(), "Unexpected '}'");
            return nullptr;
        } break;

        case TokenType::RIGHT_ROUND: {
            expect(peekCurr(), "Unexpected ')'");
            return nullptr;
        } break;

        case TokenType::LEFT_CURLY: return ParseBlockStmt(); 
        break;

        case TokenType::INT:
        case TokenType::CHAR:
        case TokenType::UINT8:
        case TokenType::UINT16: return ParseDeclStmt();
        break;

        case TokenType::STRUCT: {
            if (peekAhead(2).tokentype == TokenType::LEFT_CURLY) return ParseStructDecl();
            else return ParseDeclStmt();
        } break;

        case TokenType::IF: return ParseIfStmt();
        break;

        case TokenType::WHILE: return ParseWhileStmt();
        break;

        case TokenType::FOR: return ParseForStmt();
        break;

        case TokenType::BREAK:
        case TokenType::CONTINUE: {
            std::unique_ptr<Statement> Result;

            if (peekCurr().tokentype == TokenType::BREAK) {
                Result = std::make_unique<BreakStmt>(peekCurr().line, peekCurr().column);
            } else {
                Result = std::make_unique<ContinueStmt>(peekCurr().line, peekCurr().column);
            }

            // consume break/Consume
            getNextToken();

            if (!expectAndConsume(TokenType::SEMICOLON, "Expected ';'"))
                return nullptr;

            return Result;
        } break;

        case TokenType::RETURN: return ParseReturnStmt();
        break;

        default: return ParseExprStmt();
    }
}
