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
            std::cout << output << " generated\n";
        }
    }
    else {
        std::cerr << "Failed to generate " << output << "\n";
        return;
    }
}
