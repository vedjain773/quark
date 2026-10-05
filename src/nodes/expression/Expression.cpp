#include "nodes/Expression.hpp"
#include <iostream>
#include <map>

std::map<Operators, std::string> enumToStr = {
    {Operators::BANG, "!"},         {Operators::MINUS, "-"},
    {Operators::MODULUS, "%"},      {Operators::DIVIDE, "/"},
    {Operators::MULT, "*"},         {Operators::PLUS, "+"},
    {Operators::GREATER, ">"},      {Operators::GREATER_EQUALS, ">="},
    {Operators::LESS, "<"},         {Operators::LESS_EQUALS, "<="},
    {Operators::EQUALS, "=="},      {Operators::NOT_EQUALS, "!="},
    {Operators::AND, "&&"},         {Operators::OR, "||"},
    {Operators::ASSIGN, "="},       {Operators::SUM_ASSIGN, "+="},
    {Operators::DIFF_ASSIGN, "-="}, {Operators::PROD_ASSIGN, "*="},
    {Operators::QUOT_ASSIGN, "/="}, {Operators::MOD_ASSIGN, "%="}};

std::map<std::string, Operators> strToEnum = {
    {"!", Operators::BANG},         {"-", Operators::MINUS},
    {"%", Operators::MODULUS},      {"/", Operators::DIVIDE},
    {"*", Operators::MULT},         {"+", Operators::PLUS},
    {">", Operators::GREATER},      {">=", Operators::GREATER_EQUALS},
    {"<", Operators::LESS},         {"<=", Operators::LESS_EQUALS},
    {"==", Operators::EQUALS},      {"!=", Operators::NOT_EQUALS},
    {"&&", Operators::AND},         {"||", Operators::OR},
    {"=", Operators::ASSIGN},       {"+=", Operators::SUM_ASSIGN},
    {"-=", Operators::DIFF_ASSIGN}, {"*=", Operators::PROD_ASSIGN},
    {"/=", Operators::QUOT_ASSIGN}, {"%=", Operators::MOD_ASSIGN}};

std::string getOpStr(Operators op) {
    return enumToStr[op];
}

Operators getOp(const std::string &opStr) {
    return strToEnum[opStr];
}

Expression::Expression(int tline, int tcol) :
    infType(nullptr),
    line(tline),
    column(tcol) {}

IntExpr::IntExpr(int value, int tline, int tcol) :
    Expression(tline, tcol),
    Val(value) {}

void IntExpr::accept(Visitor &visitor) {
    visitor.visitIntExpr(*this);
}

CharExpr::CharExpr(char charac, int tline, int tcol) :
    Expression(tline, tcol),
    character(charac) {}

void CharExpr::accept(Visitor &visitor) {
    visitor.visitCharExpr(*this);
}

VarExpr::VarExpr(const std::string &name, int tline, int tcol) :
    Expression(tline, tcol),
    Name(name) {}

void VarExpr::accept(Visitor &visitor) {
    visitor.visitVarExpr(*this);
}

bool VarExpr::isLValue() {
    return true;
}

DerefExpr::DerefExpr(std::unique_ptr<Expression> expression, int tline, int tcol) :
    Expression(tline, tcol),
    expr(std::move(expression)) {}

void DerefExpr::accept(Visitor &visitor) {
    visitor.visitDerefExpr(*this);
}

bool DerefExpr::isLValue() {
    return true;
}

AddressExpr::AddressExpr(std::unique_ptr<Expression> expression, int tline, int tcol) :
    Expression(tline, tcol),
    expr(std::move(expression)) {}

void AddressExpr::accept(Visitor &visitor) {
    visitor.visitAddressExpr(*this);
}

SizeOfExpr::SizeOfExpr(std::unique_ptr<Expression> expression, int tline, int tcol) :
    Expression(tline, tcol),
    expr(std::move(expression)) {}

SizeOfExpr::SizeOfExpr(TypeKind *typek, int tline, int tcol) :
    Expression(tline, tcol),
    argType(typek) {}

void SizeOfExpr::accept(Visitor &visitor) {
    visitor.visitSizeOfExpr(*this);
}

CallExpr::CallExpr(const std::string &callee_name, int tline, int tcol) :
    Expression(tline, tcol),
    callee(callee_name) {}

void CallExpr::add(std::unique_ptr<Expression> arg) {
    args.push_back(std::move(arg));
}

void CallExpr::accept(Visitor &visitor) {
    visitor.visitCallExpr(*this);
}

CastExpr::CastExpr(std::unique_ptr<Expression> expression, TypeKind *from_tk, TypeKind *to_tk) :
    from(from_tk),
    to(to_tk),
    expr(std::move(expression)) {}

void CastExpr::accept(Visitor &visitor) {
    visitor.visitCastExpr(*this);
}

UnaryExpr::UnaryExpr(Operators op, std::unique_ptr<Expression> operand, int tline, int tcol) :
    Expression(tline, tcol),
    Op(op),
    Operand(std::move(operand)) {}

void UnaryExpr::accept(Visitor &visitor) {
    visitor.visitUnaryExpr(*this);
}

BinaryExpr::BinaryExpr(Operators op, std::unique_ptr<Expression> lhs,
                       std::unique_ptr<Expression> rhs, int tline, int tcol) :
    Expression(tline, tcol),
    Op(op),
    LHS(std::move(lhs)),
    RHS(std::move(rhs)) {}

void BinaryExpr::accept(Visitor &visitor) {
    visitor.visitBinaryExpr(*this);
}

AssignExpr::AssignExpr(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs, int tline,
                       int tcol) :
    Expression(tline, tcol),
    LHS(std::move(lhs)),
    RHS(std::move(rhs)) {}

void AssignExpr::accept(Visitor &visitor) {
    visitor.visitAssignExpr(*this);
}

CompAssignExpr::CompAssignExpr(Operators assignOp, std::unique_ptr<Expression> lhs,
                               std::unique_ptr<Expression> rhs, int tline, int tcol) :
    Expression(tline, tcol),
    Op(assignOp),
    LHS(std::move(lhs)),
    RHS(std::move(rhs)) {}

void CompAssignExpr::accept(Visitor &visitor) {
    visitor.visitCompAssignExpr(*this);
}

void EmptyExpr::accept(Visitor &visitor) {
    visitor.visitEmptyExpr(*this);
}

MemberAccessExpr::MemberAccessExpr(std::unique_ptr<Expression> baseExpr,
                                   const std::string &fieldName, int tline, int tcol) :
    Expression(tline, tcol),
    base(std::move(baseExpr)),
    fName(fieldName) {}

void MemberAccessExpr::accept(Visitor &visitor) {
    visitor.visitMemberAccessExpr(*this);
}

bool MemberAccessExpr::isLValue() {
    return true;
}
