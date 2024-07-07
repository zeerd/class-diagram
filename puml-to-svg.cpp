#include "class-diagram.hpp"

void PUmlToSvg::save(std::string input, std::string output)
{
    if (jar == "") {
        return;
    }

    std::ostringstream cmd;
    cmd << java << " -Djava.awt.headless=true -Dfile.encoding=UTF-8 -jar "
        << jar << " -charset UTF-8 -tsvg " << input;
    if (std::system(cmd.str().c_str()) == 0) {
        if (verbose) {
            llvm::outs() << output << " generated\n";
        }
    }
    else {
        llvm::errs() << "Failed to generate " << output << "\n";
        return;
    }
}

