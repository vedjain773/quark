#include "frontend/parser/Parser.hpp"
#include "frontend/scanner/Scanner.hpp"
#include "nodes/Program.hpp"
#include "utils/Error.hpp"
#include <iostream>

struct CIConfig {
    bool optimize = false;
    bool printAST = false;
    bool printTokens = false;
    bool printIR = false;
    bool notCompile = false;
};

int main(int argc, char **argv) {

    CIConfig config;

    if (argc < 2) {
        std::cerr << "Usage: quark <src> [flags] [-o <dest>]\n";
        return 1;
    }

    std::string filename = argv[1];
    std::string destname = "output.o";

    for (int i = 2; i < argc; ++i) {
        std::string_view arg = argv[i];

        if (arg == "--print-tokens") {
            config.printTokens = true;
        } else if (arg == "--print-ast") {
            config.printAST = true;
        } else if (arg == "--print-llvm") {
            config.printIR = true;
        } else if (arg == "--optimize") {
            config.optimize = true;
        } else if (arg == "--no-compile") {
            config.notCompile = true;
        } else if (arg == "-o") {
            if (++i >= argc) {
                std::cerr << "error: -o requires an argument\n";
                return 1;
            }

            destname = argv[i];
        } else {
            std::cerr << "error: unknown argument: " << arg << '\n';
            return 1;
        }
    }

    getSourceLines(filename);

    Scanner scanner(filename);
    scanner.scanFile();
    scanner.scanProg();

    if (config.printTokens) {
        scanner.printTokens();
    }

    std::vector<Token> tokenlist = scanner.getTokenList();

    Parser parser(tokenlist);
    auto prog = parser.ParseProgram();
    prog->setFileName(filename);

    int noErr = prog->semAnalyse();

    if (config.printAST) {
        prog->printAST();
        std::cout << "\n";
    }

    int totalErrors = noErr + parser.numOfErrors;

    if (totalErrors > 0) {
        std::cout << "Build failed with " << totalErrors << " error(s)\n";
        return -1;
    }

    if (!config.notCompile) {
        prog->codegen();

        if (config.optimize)
            prog->opt();

        if (config.printIR)
            prog->emitIR();

        prog->emitObj(destname);
    }

    return 0;
}
