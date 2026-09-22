#ifndef PROGRAM_H
#define PROGRAM_H

#include "nodes/ExternalDecl.hpp"
#include "nodes/Statement.hpp"
#include "visitors/Visitor.hpp"
#include <memory>
#include <string>
#include <vector>

class Program {
  private:
    std::string fileName;
    std::string target;
    CodegenVis codegenvis;

  public:
    std::vector<std::unique_ptr<ExternalDecl>> root;

    void setFileName(const std::string &file_name);
    void setTarget(const std::string &target_name);
    void accept(Visitor &visitor);
    void add(std::unique_ptr<ExternalDecl> edecl);

    void printAST();
    int semAnalyse();
    void opt();
    void codegen();

    void emitIR();
    void emitObj(const std::string &fileName);
};

#endif
