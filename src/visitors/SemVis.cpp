#include "nodes/Expression.hpp"
#include "nodes/ExternalDecl.hpp"
#include "nodes/Function.hpp"
#include "nodes/Program.hpp"
#include "nodes/Statement.hpp"
#include "utils/Error.hpp"
#include "visitors/Visitor.hpp"
#include <cstddef>
#include <iostream>

using size_t = std::size_t;

Scope &SemanticVisitor::getCurrScope() {
    return scopeVec[scopeVec.size() - 1];
}

void SemanticVisitor::reportError(Statement &stmt, std::string msg) {
    Error error(stmt.line, stmt.column, msg);
    numOfErrors += 1;
}

void SemanticVisitor::reportError(Expression &expr, std::string msg) {
    Error error(expr.line, expr.column, msg);
    numOfErrors += 1;
}

void SemanticVisitor::reportError(GlobalDecl &globaldecl, std::string msg) {
    Error error(globaldecl.line, globaldecl.column, msg);
    numOfErrors += 1;
}

void SemanticVisitor::visitProgram(Program &program) {
    Scope globalScope;
    scopeVec.push_back(globalScope);

    for (auto &edecl : program.root) {
        if (edecl == nullptr)
            continue;

        edecl->accept(*this);
    }

    scopeVec.pop_back();
}

void SemanticVisitor::visitGlobalDecl(GlobalDecl &globaldecl) {
    if (getCurrScope().search(globaldecl.name)) {
        reportError(globaldecl, std::format("redeclaration of {}", globaldecl.name));
    } else {
        getCurrScope().addRow(globaldecl.name, globaldecl.type, SymbolKind::VARIABLE);
    }

    Expression *expr = (globaldecl.expression).get();
    TypeKind *declType = globaldecl.type;

    if (expr != nullptr) {
        expr->accept(*this);
        TypeKind *exprType = expr->infType;
        
        if (exprType == getType("void")) {
            return reportError(globaldecl, "Variables cannot be of type: void");
        }
        
        if (exprType->type != declType->type) {
            std::string errmsg = std::format(
                    "Cannot assign expression of type {} to '{}' of type {}",
                    exprType->name, globaldecl.name, declType->name);

            return reportError(globaldecl, errmsg);
        } 

        if (exprType != declType) {
            auto castexpr = std::make_unique<CastExpr>(std::move(globaldecl.expression),
                    exprType, declType);

            Expression *cexpr = castexpr.get();
            cexpr->accept(*this);

            globaldecl.expression = std::move(castexpr);
        }

        globaldecl.expression->infType = declType;
    }
}

void SemanticVisitor::visitParameter(Parameter &parameter) {
    getCurrScope().addRow(parameter.name, parameter.type, SymbolKind::VARIABLE);
}

void SemanticVisitor::visitPrototype(Prototype &prototype) {    
    std::string funcName = prototype.funcName;

    if (scopeVec[0].search(funcName)) {
        Error error(prototype.line, prototype.column,
                std::format("redeclaration of {}", funcName));
        numOfErrors += 1;
    } else {
        scopeVec[0].addRow(funcName, prototype.retType, SymbolKind::FUNCTION);
        currFuncRetType = prototype.retType;
    }

    for (auto &param : prototype.paramList) {
        param->accept(*this);
        scopeVec[0].addParam(funcName, param->type);
    }
}

void SemanticVisitor::visitFuncDef(FuncDef &funcdef) {
    Scope funcScope;
    scopeVec.push_back(funcScope);

    Prototype *proto = (funcdef.prototype).get();
    BlockStmt *body = (funcdef.funcBody).get();

    proto->accept(*this);

    for (auto &statmt : body->statements) {
        statmt->accept(*this);
    }

    scopeVec.pop_back();
}

void SemanticVisitor::visitEmptyStmt(EmptyStmt &emptystmt) {
    // no checks
}

void SemanticVisitor::visitBlockStmt(BlockStmt &blockstmt) {
    Scope locScope;
    scopeVec.push_back(locScope);

    bool isTerm = false;

    for (auto &stmt : blockstmt.statements) {
        if (isTerm) {
            Warning warning(stmt->line, stmt->column, "Statement is unreachable");
        }

        stmt->accept(*this);

        if (stmt->isTerminator())
            isTerm = true;
    }

    scopeVec.pop_back();
}

void SemanticVisitor::visitDeclStmt(DeclStmt &declstmt) {
    if (getCurrScope().search(declstmt.name)) {
        reportError(declstmt, std::format("redeclaration of {}", declstmt.name));
    } else {
        getCurrScope().addRow(declstmt.name, declstmt.type, SymbolKind::VARIABLE);
    }

    Expression *expr = (declstmt.expression).get();
    TypeKind *declType = declstmt.type;

    if (expr != nullptr) {
        expr->accept(*this);
        TypeKind *exprType = expr->infType;
        
        if (exprType == getType("void")) {
            return reportError(declstmt, "Variables cannot be of type: void");
        }
        
        if (exprType->type != declType->type) {
            std::string errmsg = std::format(
                    "Cannot assign expression of type {} to '{}' of type {}",
                    exprType->name, declstmt.name, declType->name);

            return reportError(declstmt, errmsg);
        } 

        if (exprType != declType) {
            auto castexpr = std::make_unique<CastExpr>(std::move(declstmt.expression),
                    exprType, declType);

            Expression *cexpr = castexpr.get();
            cexpr->accept(*this);

            declstmt.expression = std::move(castexpr);
        }

        declstmt.expression->infType = declType;
    }
}

void SemanticVisitor::visitStructDecl(StructDecl &structdecl) {
    std::string typeName = "struct ";
    typeName += structdecl.tag;

    if (structdecl.fields.empty())
        return;

    TypeKind *typek = getType(typeName);
    typek->size = 0;

    int offset = 0;

    for (auto &structField : structdecl.fields) {
        int fieldAlign = structField->type->align;
        int fieldSize = structField->type->size;

        if (offset % fieldAlign != 0)
            offset += fieldAlign - offset % fieldAlign;

        offset += fieldSize;

        typek->fields.push_back({structField->type, structField->fName});
    }

    if (offset > 0)
        typek->size = offset;
}

void SemanticVisitor::visitStructField(StructField &structfield) {
    // do nothing
}

void SemanticVisitor::visitIfStmt(IfStmt &ifstmt) {
    Expression *condn = (ifstmt.condition).get();
    Statement *ifbody = (ifstmt.body).get();
    Statement *elsestmt = (ifstmt.elseStmt).get();

    condn->accept(*this);

    if (condn->infType != getType("int")) {
        std::string errmsg = "Invalid (if) condition expression; Expected: int, Got: ";
        std::string typeName = condn->infType->name;
        return reportError(*condn, errmsg + typeName);
    }

    ifbody->accept(*this);

    if (elsestmt != nullptr) {
        elsestmt->accept(*this);
    }
}

void SemanticVisitor::visitElseStmt(ElseStmt &elsestmt) {
    Statement *elsebody = (elsestmt.body).get();

    elsebody->accept(*this);
}

void SemanticVisitor::visitForStmt(ForStmt &forstmt) {
    Scope forScope;
    scopeVec.push_back(forScope);

    Statement *init = (forstmt.init).get();
    Expression *condn = (forstmt.condn).get();
    Expression *iter = (forstmt.iter).get();

    Statement *forbody = (forstmt.body).get();

    init->accept(*this);
    condn->accept(*this);
    iter->accept(*this);

    insideLoop++;
    forbody->accept(*this);
    insideLoop--;

    scopeVec.pop_back();
}

void SemanticVisitor::visitWhileStmt(WhileStmt &whilestmt) {
    Expression *condn = (whilestmt.condition).get();
    Statement *whilebody = (whilestmt.body).get();

    condn->accept(*this);

    if (condn->infType != getType("int")) {
        std::string errmsg = "Invalid (while) condition expression, Expected: int, Got: ";
        std::string typeName = condn->infType->name;
        return reportError(whilestmt, errmsg + typeName);
    }

    insideLoop++;
    whilebody->accept(*this);
    insideLoop--;
}

void SemanticVisitor::visitBreakStmt(BreakStmt &breakstmt) {
    if (insideLoop <= 0) {
        return reportError(breakstmt, "Break statements must be inside while loops");
    }
}

void SemanticVisitor::visitContinueStmt(ContinueStmt &continuestmt) {
    if (insideLoop <= 0) {
        return reportError(continuestmt, "Continue statements must be inside while loops");
    }
}

void SemanticVisitor::visitReturnStmt(ReturnStmt &returnstmt) {
    Expression *retexpr = (returnstmt.retExpr).get();

    retexpr->accept(*this);
    TypeKind *retExprType = retexpr->infType;

    if (isErrorType(retExprType))
        return;

    if (retExprType != currFuncRetType) {
        std::string msg = std::format(
                "Return type ({}) does not match function return type ({})",
                retExprType->name, currFuncRetType->name);

        return reportError(*retexpr, msg);
    }
}

void SemanticVisitor::visitExprStmt(ExprStmt &exprstmt) {
    Expression *expr = (exprstmt.expression).get();

    expr->accept(*this);
}

void SemanticVisitor::visitMemberAccessExpr(MemberAccessExpr &memexpr) {
    Expression *expr = (memexpr.base).get();

    expr->accept(*this);
    TypeKind *exprType = expr->infType;

    if (!isStructType(exprType)) {
        std::cout << exprType->name << "\n";
        return reportError(*expr, "Base expression is not a struct");
    }

    bool found = false;
    int index = 0;

    for (size_t i = 0; i < exprType->fields.size(); i++) {
        if (exprType->fields[i].name == memexpr.fName) {
            found = true;
            index = i;
            break;
        }
    }

    if (!found) {
        std::string msg = std::format("Struct has no field: {}", memexpr.fName);

        return reportError(*expr, msg);
    }

    memexpr.infType = exprType->fields[index].fType;
}

void SemanticVisitor::visitEmptyExpr(EmptyExpr &emptyexpr) {
    emptyexpr.infType = getType("void");
}

void SemanticVisitor::visitAssignExpr(AssignExpr &assignexpr) {
    Expression *lExpr = (assignexpr.LHS).get();
    Expression *rExpr = (assignexpr.RHS).get();

    lExpr->accept(*this);

    rExpr->accept(*this);

    if (!lExpr->isLValue()) {
        return reportError(*lExpr, "Expression is not assignable");
    }

    if (rExpr->infType == getType("void")) {
        return reportError(assignexpr, "Assignment operand cannot be of type: void");
    }

    if (lExpr->infType != rExpr->infType) {
        auto castexpr = std::make_unique<CastExpr>(
            std::move(assignexpr.RHS), assignexpr.RHS->infType, assignexpr.LHS->infType);

        Expression *cexpr = castexpr.get();
        cexpr->accept(*this);

        assignexpr.RHS = std::move(castexpr);
    }

    assignexpr.infType = assignexpr.LHS->infType;
}

void SemanticVisitor::visitCompAssignExpr(CompAssignExpr &compassignexpr) {
    Expression *lExpr = (compassignexpr.LHS).get();
    Expression *rExpr = (compassignexpr.RHS).get();

    lExpr->accept(*this);

    rExpr->accept(*this);

    if (!lExpr->isLValue()) {
        return reportError(*lExpr, "Expression is not assignable");
    }

    if (rExpr->infType == getType("void")) {
        return reportError(*rExpr, "Assignment operand cannot be of type void");
    }

    compassignexpr.infType = compassignexpr.LHS->infType;
}

void SemanticVisitor::visitBinaryExpr(BinaryExpr &binexpr) {
    Expression *lExpr = (binexpr.LHS).get();
    Expression *rExpr = (binexpr.RHS).get();
 
    lExpr->accept(*this);
    rExpr->accept(*this);

    TypeKind *leftType = lExpr->infType;
    TypeKind *rightType = rExpr->infType;

    if (leftType == getType("void")) {
        return reportError(*lExpr, "Binary operand cannot be of type: void");
    }

    if (rightType == getType("void")) {
        return reportError(*rExpr, "Binary operand cannot be of type: void");
    }

    bool isLPointerOrArray = isPointerType(leftType) || isArrayType(leftType);
    bool isRPointerOrArray = isPointerType(rightType) || isArrayType(rightType);

    if (leftType != rightType) {
        if (!isLPointerOrArray && !isRPointerOrArray) {
            auto castexpr = std::make_unique<CastExpr>(std::move(binexpr.RHS), binexpr.RHS->infType,
                                                       binexpr.LHS->infType);

            Expression *cexpr = castexpr.get();
            cexpr->accept(*this);

            binexpr.RHS = std::move(castexpr);
        } else {
            handlePointerArithmetic(binexpr);
        }
    }

    binexpr.infType = binexpr.LHS->infType;
}

void SemanticVisitor::handlePointerArithmetic(BinaryExpr &binexpr) {
    Expression *lExpr = (binexpr.LHS).get();
    Expression *rExpr = (binexpr.RHS).get();

    bool isLPointerOrArray = isPointerType(lExpr->infType) || isArrayType(lExpr->infType);
    bool isRPointerOrArray = isPointerType(rExpr->infType) || isArrayType(rExpr->infType);

    if (isLPointerOrArray && isRPointerOrArray) {
        return reportError(*lExpr, "Pointer-Pointer operations are not supported");
    }

    if (isRPointerOrArray) {
        return reportError(*rExpr, "Pointers must be left operands");
    }

    binexpr.infType = lExpr->infType;
    return;
}

void SemanticVisitor::visitSizeOfExpr(SizeOfExpr &sizeofexpr) {
    Expression *expr = (sizeofexpr.expr).get();

    if (expr != nullptr) {
        expr->accept(*this);
        sizeofexpr.argType = expr->infType;
    }

    sizeofexpr.infType = getType("int");
}

void SemanticVisitor::visitUnaryExpr(UnaryExpr &unaryexpr) {
    Expression *Operand = (unaryexpr.Operand).get();

    Operand->accept(*this);

    if (Operand->infType == getType("int")) {
        unaryexpr.infType = getType("int");
    } else {
        std::string typeName = Operand->infType->name;
        return reportError(unaryexpr, "Expected type: int, Got: " + typeName);
    }
}

void SemanticVisitor::visitDerefExpr(DerefExpr &derefexpr) {
    Expression *expr = (derefexpr.expr).get();

    expr->accept(*this);

    if (!isPointerType(expr->infType) && !isArrayType(expr->infType)) {
        std::string typeName = expr->infType->name;
        return reportError(*expr, "Expected pointer-type, Got: " + typeName);
    }

    derefexpr.infType = expr->infType->to;
}

void SemanticVisitor::visitAddressExpr(AddressExpr &addressexpr) {
    Expression *expr = (addressexpr.expr).get();

    if (!expr->isLValue()) {
        return reportError(*expr, "Operand must be an lvalue");
    }

    expr->accept(*this);

    std::string typeName = expr->infType->name;
    addressexpr.infType = getType(typeName + '*');
}

void SemanticVisitor::visitCastExpr(CastExpr &castexpr) {
    if (isArrayType(castexpr.from)) {
        std::string errmsg = std::format(
                "Cannot cast from type: {} to {}", castexpr.from->name, castexpr.to->name);
        return reportError(castexpr, errmsg);
    }

    castexpr.infType = castexpr.to;
}

void SemanticVisitor::visitCallExpr(CallExpr &callexpr) {
    bool flag = false;

    for (int i = scopeVec.size() - 1; i >= 0; i--) {
        if (scopeVec[i].search(callexpr.callee)) {
            flag = true;

            if (scopeVec[i].getSymKind(callexpr.callee) == SymbolKind::FUNCTION) {
                callexpr.infType = scopeVec[i].getSymType(callexpr.callee);
                break;
            } else {
                return reportError(callexpr, callexpr.callee + " is not a callable function");
            }
        }
    }

    if (!flag) {
        return reportError(callexpr, "Undeclared function: " + callexpr.callee);
    }

    size_t numberOfParams = scopeVec[0].getNumParams(callexpr.callee);

    if (numberOfParams != callexpr.args.size()) {
        int expected = scopeVec[0].getNumParams(callexpr.callee);
        int got = callexpr.args.size();

        std::string errmsg = std::format("Expected: {} args, got {}", expected, got);
        return reportError(callexpr, errmsg);
    }

    for (auto &expr : callexpr.args) {
        expr->accept(*this);
    }

    std::vector<TypeKind *> paramTypes = scopeVec[0].getParams(callexpr.callee);

    for (size_t i = 0; i < numberOfParams; i++) {
        TypeKind *currentParamType = (callexpr.args[i])->infType;

        if (currentParamType != paramTypes[i]) {
            std::string errmsg = std::format(
                    "Expected argument type: {} got: ", paramTypes[i]->name, currentParamType->name); 

            return reportError(*callexpr.args[i], errmsg);
        }
    }
}

void SemanticVisitor::visitVarExpr(VarExpr &varexpr) {
    bool flag = false;

    for (int i = scopeVec.size() - 1; i >= 0; i--) {
        if (scopeVec[i].search(varexpr.Name)) {
            flag = true;

            if (scopeVec[i].getSymKind(varexpr.Name) == SymbolKind::VARIABLE) {
                varexpr.infType = scopeVec[i].getSymType(varexpr.Name);
                break;
            } else {
                return reportError(varexpr, varexpr.Name + " is not a variable");
            }

            break;
        }
    }

    if (!flag) {
        reportError(varexpr, "Undeclared variable: " + varexpr.Name);
        varexpr.infType = getType("error");
    }
}

void SemanticVisitor::visitCharExpr(CharExpr &charexpr) { 
    charexpr.infType = getType("char");
}

void SemanticVisitor::visitIntExpr(IntExpr &intexpr) {
    intexpr.infType = getType("int");
}
