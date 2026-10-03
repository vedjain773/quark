target_sources(quark PRIVATE
    expression/Expression.cpp
    expression/ExprCodegen.cpp
    
    statement/Statement.cpp
    statement/StmtCodegen.cpp

    external/ExtCodegen.cpp
    external/ExternalDecl.cpp

    function/Function.cpp
    function/FuncCodegen.cpp

    Program.cpp
)
