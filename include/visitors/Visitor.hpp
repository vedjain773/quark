#ifndef VISITOR_H
#define VISITOR_H

#include "utils/Scope.hpp"
#include <stack>
#include <string>
#include <vector>

class Expression;
class Statement;

class IntExpr;
class CharExpr;
class VarExpr;
class CallExpr;
class CastExpr;
class DerefExpr;
class AddressExpr;
class SizeOfExpr;
class UnaryExpr;
class BinaryExpr;
class AssignExpr;
class CompAssignExpr;
class EmptyExpr;
class MemberAccessExpr;

class ExprStmt;
class BlockStmt;
class IfStmt;
class ElseStmt;
class WhileStmt;
class ForStmt;
class BreakStmt;
class ContinueStmt;
class ReturnStmt;
class DeclStmt;
class EmptyStmt;

class StructDecl;
class StructField;

class Parameter;
class Prototype;
class FuncDef;

class ExternalDecl;
class GlobalDecl;
class Program;

class Visitor {
  public:
    int depth = 0;

    virtual void visitIntExpr(IntExpr &intexpr) = 0;
    virtual void visitCharExpr(CharExpr &charexpr) = 0;
    virtual void visitVarExpr(VarExpr &varexpr) = 0;
    virtual void visitCallExpr(CallExpr &callexpr) = 0;
    virtual void visitCastExpr(CastExpr &castexpr) = 0;
    virtual void visitDerefExpr(DerefExpr &derefexpr) = 0;
    virtual void visitAddressExpr(AddressExpr &addressexpr) = 0;
    virtual void visitSizeOfExpr(SizeOfExpr &sizeofexpr) = 0;
    virtual void visitUnaryExpr(UnaryExpr &unaryexpr) = 0;
    virtual void visitBinaryExpr(BinaryExpr &binaryexpr) = 0;
    virtual void visitAssignExpr(AssignExpr &assignexpr) = 0;
    virtual void visitCompAssignExpr(CompAssignExpr &compassignexpr) = 0;
    virtual void visitEmptyExpr(EmptyExpr &emptyexpr) = 0;
    virtual void visitMemberAccessExpr(MemberAccessExpr &memexpr) = 0;

    virtual void visitExprStmt(ExprStmt &exprstmt) = 0;
    virtual void visitBlockStmt(BlockStmt &blockstmt) = 0;
    virtual void visitIfStmt(IfStmt &ifstmt) = 0;
    virtual void visitElseStmt(ElseStmt &elsestmt) = 0;
    virtual void visitWhileStmt(WhileStmt &whilestmt) = 0;
    virtual void visitForStmt(ForStmt &forstmt) = 0;
    virtual void visitBreakStmt(BreakStmt &breakstmt) = 0;
    virtual void visitContinueStmt(ContinueStmt &continuestmt) = 0;
    virtual void visitReturnStmt(ReturnStmt &returnstmt) = 0;
    virtual void visitDeclStmt(DeclStmt &declstmt) = 0;
    virtual void visitEmptyStmt(EmptyStmt &emptystmt) = 0;

    virtual void visitStructDecl(StructDecl &structdecl) = 0;
    virtual void visitStructField(StructField &structfield) = 0;

    virtual void visitParameter(Parameter &parameter) = 0;
    virtual void visitPrototype(Prototype &prototype) = 0;
    virtual void visitFuncDef(FuncDef &funcdef) = 0;

    virtual void visitGlobalDecl(GlobalDecl &globaldecl) = 0;
    virtual void visitProgram(Program &program) = 0;
};

class PrintVisitor : public Visitor {
  public:
    void visitIntExpr(IntExpr &intexpr);
    void visitCharExpr(CharExpr &charexpr);
    void visitVarExpr(VarExpr &varexpr);
    void visitCallExpr(CallExpr &callexpr);
    void visitCastExpr(CastExpr &castexpr);
    void visitDerefExpr(DerefExpr &derefexpr);
    void visitAddressExpr(AddressExpr &addressexpr);
    void visitSizeOfExpr(SizeOfExpr &sizeofexpr);
    void visitUnaryExpr(UnaryExpr &unaryexpr);
    void visitBinaryExpr(BinaryExpr &binaryexpr);
    void visitAssignExpr(AssignExpr &assignexpr);
    void visitCompAssignExpr(CompAssignExpr &compassignexpr);
    void visitEmptyExpr(EmptyExpr &emptyexpr);
    void visitMemberAccessExpr(MemberAccessExpr &memexpr);

    void visitExprStmt(ExprStmt &exprstmt);
    void visitBlockStmt(BlockStmt &blockstmt);
    void visitIfStmt(IfStmt &ifstmt);
    void visitElseStmt(ElseStmt &elsestmt);
    void visitWhileStmt(WhileStmt &whilestmt);
    void visitForStmt(ForStmt &forstmt);
    void visitBreakStmt(BreakStmt &breakstmt);
    void visitContinueStmt(ContinueStmt &continuestmt);
    void visitReturnStmt(ReturnStmt &returnstmt);
    void visitDeclStmt(DeclStmt &declstmt);
    void visitEmptyStmt(EmptyStmt &emptystmt);

    void visitStructDecl(StructDecl &structdecl);
    void visitStructField(StructField &structfield);

    void visitParameter(Parameter &parameter);
    void visitPrototype(Prototype &prototype);
    void visitFuncDef(FuncDef &funcdef);

    void visitGlobalDecl(GlobalDecl &globaldecl);
    void visitProgram(Program &program);

    std::string getIndent();
};

class SemanticVisitor : public Visitor {
  public:
    std::vector<Scope> scopeVec;
    TypeKind *currFuncRetType;
    int insideLoop = 0;
    int numOfErrors = 0;

    Scope &getCurrScope();
    void reportError(Statement &stmt, std::string msg);
    void reportError(Expression &expr, std::string msg);
    void reportError(GlobalDecl &globaldecl, std::string msg);

    void visitIntExpr(IntExpr &intexpr);
    void visitCharExpr(CharExpr &charexpr);
    void visitVarExpr(VarExpr &varexpr);
    void visitCallExpr(CallExpr &callexpr);
    void visitCastExpr(CastExpr &castexpr);
    void visitDerefExpr(DerefExpr &derefexpr);
    void visitAddressExpr(AddressExpr &addressexpr);
    void visitSizeOfExpr(SizeOfExpr &sizeofexpr);
    void visitUnaryExpr(UnaryExpr &unaryexpr);
    void visitBinaryExpr(BinaryExpr &binaryexpr);
    void visitMemberAccessExpr(MemberAccessExpr &memexpr);

    void handlePointerArithmetic(BinaryExpr &binaryexpr);

    void visitAssignExpr(AssignExpr &assignexpr);
    void visitCompAssignExpr(CompAssignExpr &compassignexpr);
    void visitEmptyExpr(EmptyExpr &emptyexpr);

    void visitExprStmt(ExprStmt &exprstmt);
    void visitBlockStmt(BlockStmt &blockstmt);
    void visitIfStmt(IfStmt &ifstmt);
    void visitElseStmt(ElseStmt &elsestmt);
    void visitWhileStmt(WhileStmt &whilestmt);
    void visitBreakStmt(BreakStmt &breakstmt);
    void visitContinueStmt(ContinueStmt &continuestmt);
    void visitForStmt(ForStmt &forstmt);
    void visitReturnStmt(ReturnStmt &returnstmt);
    void visitDeclStmt(DeclStmt &declstmt);
    void visitEmptyStmt(EmptyStmt &emptystmt);

    void visitStructDecl(StructDecl &structdecl);
    void visitStructField(StructField &structfield);

    void visitParameter(Parameter &parameter);
    void visitPrototype(Prototype &prototype);
    void visitFuncDef(FuncDef &funcdef);

    void visitGlobalDecl(GlobalDecl &globaldecl);
    void visitProgram(Program &program);
};

#endif
