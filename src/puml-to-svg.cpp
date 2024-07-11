#include <linux/limits.h>
#include <stdlib.h>

#include <filesystem>

#include "class-diagram.hpp"

std::string PUmlToSvg::replaceHomeDirectory(std::string path)
{
    size_t pos = path.find('~');
    if (pos != std::string::npos) {
        const char *home = std::getenv("HOME");
        if (home == nullptr) {
            throw std::runtime_error("Failed to get home directory");
        }
        path.replace(pos, 1, home);
    }
    return path;
}

std::string PUmlToSvg::makeCmd(std::string input)
{
    if (verbose) {
        std::cout << "java:" << java << "\n";
        std::cout << "jar:" << jar << "\n";
        std::cout << "theme:" << theme << "\n";
    }

    std::ostringstream cmd;
    cmd << java << " -Djava.awt.headless=true"
        << " -Dfile.encoding=UTF-8"
        << " -jar " << jar << " -charset UTF-8"
        << " -tsvg";
    if (theme != "") {
        cmd << " -I" << replaceHomeDirectory(theme);
    }
    cmd << " " << input;

    if (verbose) {
        std::cout << "Run: " << cmd.str() << "\n";
    }
    return cmd.str();
}

void PUmlToSvg::save(std::string input, std::string output)
{
    if (jar == "") {
        if (verbose) {
            std::cout << "no plantuml.jar given."
                      << "\n";
        }
        return;
    }

    if (std::system(makeCmd(input).c_str()) == 0) {
        if (verbose) {
            std::cout << output << " generated\n";
        }
    }
    else {
        std::cerr << "Failed to generate " << output << "\n";
        return;
    }
}
