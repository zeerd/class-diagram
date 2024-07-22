
#include "puml-to-svg.hpp"

#include <filesystem>
#include <iostream>

std::string PUmlToSvg::replaceHomeDirectory(std::string path)
{
    size_t pos = path.find('~');
    if (pos != std::string::npos) {
        const char* home = std::getenv("HOME");
        // LCOV_EXCL_START
        if (home == nullptr) {
            throw std::runtime_error("Failed to get home directory");
        }
        // LCOV_EXCL_STOP
        path.replace(pos, 1, home);
    }
    return path;
}

std::string PUmlToSvg::makeCmd(const std::string& input)
{
    if (verbose) {
        std::cout << "Parameters:\n";
        std::cout << "  java  : " << java << "\n";
        std::cout << "  jar   : " << jar << "\n";
        std::cout << "  theme : " << theme << "\n";
    }

    std::ostringstream cmd;
    cmd << java << " -Djava.awt.headless=true" << " -Dfile.encoding=UTF-8"
        << " -jar " << jar << " -charset UTF-8" << " -tsvg";
    if (theme != "") {
        cmd << " -I" << replaceHomeDirectory(theme);
    }
    cmd << " " << input;

    if (verbose) {
        std::cout << "Run: " << cmd.str() << "\n\n";
    }
    return cmd.str();
}

void PUmlToSvg::save(const std::string& input, const std::string& output)
{
    // LCOV_EXCL_START
    if (jar == "") {
        if (verbose) {
            std::cout << "no plantuml.jar given." << "\n";
        }
        return;
    }
    // LCOV_EXCL_STOP

    if (std::system(makeCmd(input).c_str()) == 0) {
        if (verbose) {
            std::cout << output << " generated\n\n";
        }
    }
    else {
        // LCOV_EXCL_START
        std::cerr << "Failed to generate " << output << "\n\n";
        return;
        // LCOV_EXCL_STOP
    }
}
