#include "frontend/parser/Parser.hpp"
#include "nodes/Program.hpp"
#include "utils/Error.hpp"
#include "visitors/Visitor.hpp"

std::unique_ptr<Parameter> Parser::ParseParameter() {
    auto [typek, name, tline, tcol] = getTypeNamePair();

    return std::make_unique<Parameter>(typek, name);
}

std::unique_ptr<Prototype> Parser::ParsePrototype() {
    return ParsePrototype(getTypeNamePair()); 
}

std::unique_ptr<Prototype> Parser::ParsePrototype(TypeNameInfo data) {
    auto [typek, name, tline, tcol] = data;

    if (!expectAndConsume(TokenType::LEFT_ROUND, "Expected '(' after function name"))
        return nullptr;

    auto Result = std::make_unique<Prototype>(typek, name, tline, tcol);
    while (peekCurr().tokentype != TokenType::RIGHT_ROUND) {
        auto param = ParseParameter();

        if (param != nullptr) {
            Result->addParam(std::move(param));

            if (peekCurr().tokentype == TokenType::COMMA) {
                getNextToken();
            } else if (peekCurr().tokentype != TokenType::RIGHT_ROUND) {
                Error error(peekCurr().line, peekCurr().column, "Expected ,");
                numOfErrors += 1;
            }
        }
    }

    getNextToken();
    return Result;
}   

std::unique_ptr<FuncDef> Parser::ParseFuncDef() {
    auto proto = ParsePrototype();
    auto body = ParseBlockStmt();

    return std::make_unique<FuncDef>(std::move(proto), std::move(body));
}
