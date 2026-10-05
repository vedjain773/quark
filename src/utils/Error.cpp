#include "utils/Error.hpp"
#include <fstream>
#include <iostream>

#define RED "\033[31m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define PURPLE "\033[35m"
#define BOLD "\033[1m"
#define RESET "\033[0m"

std::vector<std::string> sourceLines;

Error::Error(int l, int c, const std::string &msg) :
    line(l),
    column(c),
    message(msg) {
    printErrorMsg(*this);
}

Warning::Warning(int l, int c, const std::string &msg) :
    line(l),
    column(c),
    message(msg) {
    printWarning(*this);
}

void printErrorMsg(Error &error) {
    std::cerr << "--> " << error.line << ":" << error.column << "\n";
    std::string msg;

    if (error.line - 1 == sourceLines.size()) msg = "END OF FILE";
    else msg = sourceLines[error.line - 1];

    std::cerr << error.line << "|" << msg << "\n";
    int offset = getNumDig(error.line);

    for (int i = 0; i < error.column + offset; i++) {
        std::cerr << " ";
    }

    std::cerr << "^\n";

    std::cerr << RED << "[ERROR]" << RESET << " ";
    std::cerr << error.message << "\n\n";
}

void printWarning(Warning &warning) {
    std::cerr << "--> " << warning.line << ":" << warning.column << "\n";
    std::string msg;

    if (warning.line - 1 == sourceLines.size()) msg = "END OF FILE";
    else msg = sourceLines[warning.line - 1];

    std::cerr << warning.line << "|" << msg << "\n";
    int offset = getNumDig(warning.line);

    for (int i = 0; i < warning.column + offset; i++) {
        std::cerr << " ";
    }

    std::cerr << "^\n";

    std::cerr << PURPLE << "[WARNING]" << RESET << " ";
    std::cerr << warning.message << "\n\n";
}

void getSourceLines(const std::string &source) {
    std::ifstream file(source);

    std::string text;

    while (getline(file, text)) {
        sourceLines.push_back(text);
    }

    file.close();
}

int getNumDig(int line) {
    if (line >= 1000) return 4;
    else if (line >= 100) return 3;
    else if (line >= 10) return 2;
    else return 1;
}
