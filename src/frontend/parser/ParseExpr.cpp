#include "frontend/parser/Parser.hpp"
#include "nodes/Program.hpp"
#include "utils/Error.hpp"
#include "visitors/Visitor.hpp"

std::unique_ptr<Expression> Parser::ParseIntExpr() {
    std::string NumStr = peekCurr().lexeme;
    int NumVal = std::stoi(NumStr);

    auto Result = std::make_unique<IntExpr>(NumVal, peekCurr().line, peekCurr().column);
    getNextToken();
    return std::move(Result);
}

std::unique_ptr<Expression> Parser::ParseCharExpr() {
    std::string charStr = peekCurr().lexeme;
    char charac = charStr[1];

    auto Result = std::make_unique<CharExpr>(charac, peekCurr().line, peekCurr().column);
    getNextToken();
    return std::move(Result);
}

std::unique_ptr<Expression> Parser::ParseVarExpr() {
    std::string Var = peekCurr().lexeme;

    auto Result = std::make_unique<VarExpr>(Var, peekCurr().line, peekCurr().column);
    getNextToken();
    return std::move(Result);
}

std::unique_ptr<Expression> Parser::ParseParenExpr() {
    getNextToken();

    auto Result = ParseExpr();

    if (!expectAndConsume(TokenType::RIGHT_ROUND, "Expected ')'")) return nullptr;

    return Result;
}

std::unique_ptr<Expression> Parser::ParsePrimaryExpr() {
    switch (peekCurr().tokentype) {
        case TokenType::INTEGER: {
            return ParseIntExpr();
        } break;

        case TokenType::CHARACTER: {
            return ParseCharExpr();
        }

        case TokenType::IDENTIFIER: {
            return ParseVarExpr();
        } break;

        case TokenType::LEFT_ROUND: {
            return ParseParenExpr();
        } break;

        default: {
            expect(peekCurr(), "Expected expression");
            return nullptr;
        }
    }
}

std::unique_ptr<Expression> Parser::ParsePostFixExpr() {
    std::string name = peekCurr().lexeme;
    int line = peekCurr().line;
    int column = peekCurr().column;
    auto Prim = ParsePrimaryExpr();

    while (isPostFixOp(peekCurr().tokentype)) {
        switch (peekCurr().tokentype) {
            case TokenType::LEFT_ROUND: {
                getNextToken();

                auto Result = std::make_unique<CallExpr>(name, line, column);
                while (peekCurr().tokentype != TokenType::RIGHT_ROUND) {
                    auto var = ParseExpr();
                    Result->add(std::move(var));

                    if (peekCurr().tokentype == TokenType::COMMA) {
                        getNextToken();
                    }
                }

                getNextToken();
                Prim = std::move(Result);
            } break;

            case TokenType::LEFT_SQUARE: {
                getNextToken();

                auto inner = ParseExpr();

                auto binExpr = std::make_unique<BinaryExpr>(Operators::PLUS, std::move(Prim),
                                                            std::move(inner), line, column);

                auto Result = std::make_unique<DerefExpr>(std::move(binExpr), line, column);

                getNextToken();

                Prim = std::move(Result);
            } break;

            case TokenType::DOT: {
                getNextToken();

                std::string fieldName = peekCurr().lexeme;

                auto Result =
                    std::make_unique<MemberAccessExpr>(std::move(Prim), fieldName, line, column);

                getNextToken();

                Prim = std::move(Result);
            } break;

            case TokenType::ARROW: {
                getNextToken();

                std::string fieldName = peekCurr().lexeme;

                auto deref = std::make_unique<DerefExpr>(std::move(Prim), line, column);

                auto Result =
                    std::make_unique<MemberAccessExpr>(std::move(deref), fieldName, line, column);

                getNextToken();

                Prim = std::move(Result);
            } break;

            default: {
                advToSyncPoint();
                return nullptr;
            }
        }
    }

    return Prim;
}

std::unique_ptr<Expression> Parser::ParseSizeOfExpr() {
    int tline = peekCurr().line;
    int tcol = peekCurr().column;

    getNextToken();

    std::unique_ptr<Expression> Result;

    if (!isTypeStarter(peekNext().tokentype)) {
        auto parenExpr = ParseParenExpr();
        Result = std::make_unique<SizeOfExpr>(std::move(parenExpr), tline, tcol);
    } else {
        // consume '('
        getNextToken();

        TypeKind *typek = ParseType();

        // consume ')'
        getNextToken();

        Result = std::make_unique<SizeOfExpr>(typek, tline, tcol);
    }

    return Result;
}

std::unique_ptr<Expression> Parser::ParseUnaryExpr() {
    switch (peekCurr().tokentype) {
        case TokenType::BANG:
        case TokenType::MINUS: {
            Operators oper = getOp(peekCurr().lexeme);

            int tline = peekCurr().line;
            int tcol = peekCurr().column;

            getNextToken();

            auto Result = std::make_unique<UnaryExpr>(oper, ParseUnaryExpr(), tline, tcol);
            return Result;
        } break;

        case TokenType::ASTERISK:
        case TokenType::AMPERSAND: {
            bool isDeref = peekCurr().tokentype == TokenType::ASTERISK;

            int line = peekCurr().line;
            int column = peekCurr().column;

            getNextToken();
            auto expr = ParseUnaryExpr();

            if (isDeref) return std::make_unique<DerefExpr>(std::move(expr), line, column);
            else return std::make_unique<AddressExpr>(std::move(expr), line, column);

        } break;

        case TokenType::SIZEOF: {
            return ParseSizeOfExpr();
        } break;

        default:
            return ParsePostFixExpr();
    }
}

std::unique_ptr<Expression> Parser::ParseBinExpr(BinOpPrec level) {
    auto parseOperand = [&](BinOpPrec level) {
        if (level == FACTOR) {
            return ParseUnaryExpr();
        } else {
            BinOpPrec next = static_cast<BinOpPrec>(level - 1);
            return ParseBinExpr(next);
        }
    };

    auto lhs = parseOperand(level);

    while (getBinPrecedence(getOp(peekCurr().lexeme)) == level) {
        Operators oper = getOp(peekCurr().lexeme);

        int tline = peekCurr().line;
        int tcol = peekCurr().column;

        getNextToken();
        auto rhs = parseOperand(level);
        lhs = std::make_unique<BinaryExpr>(oper, std::move(lhs), std::move(rhs), tline, tcol);
    }

    return lhs;
}

std::unique_ptr<Expression> Parser::ParseAssignExpr() {
    auto lhs = ParseBinExpr(LOR);

    if (isAssignOp(peekCurr().tokentype)) {
        Operators Op = getOp(peekCurr().lexeme);
        int tline = peekCurr().line;
        int tcol = peekCurr().column;

        getNextToken();
        auto rhs = ParseAssignExpr();

        if (Op == Operators::ASSIGN) {
            auto Result = std::make_unique<AssignExpr>(std::move(lhs), std::move(rhs), tline, tcol);
            return Result;
        } else {
            auto Result =
                std::make_unique<CompAssignExpr>(Op, std::move(lhs), std::move(rhs), tline, tcol);
            return Result;
        }
    } else {
        return lhs;
    }
}

std::unique_ptr<Expression> Parser::ParseExpr() {
    return ParseAssignExpr();
}
