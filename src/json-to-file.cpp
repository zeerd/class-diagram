#include "json-to-file.hpp"

#include <fstream>
#include <iostream>

void JsonToFile::save(std::string output)
{
    std::ofstream json_file(output);
    std::string json_str = classes.dump(2);
    json_file.write(json_str.c_str(), json_str.length());
    json_file.close();
    if (verbose) {
        std::cout << output << " generated\n\n";
    }
}
