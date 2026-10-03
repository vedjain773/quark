target_sources(quark PRIVATE
    parser/ParseExpr.cpp
    parser/ParseFunc.cpp
    parser/ParseStmt.cpp
    parser/Parser.cpp

    scanner/Scanner.cpp
    scanner/Token.cpp
)
