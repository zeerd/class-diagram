#include "class-diagram.hpp"

std::string JsonToPUml::getLoc(json &cls)
{
    std::string location     = cls["location"].get<std::string>();
    std::size_t lastSlashPos = location.find_last_of('/');
    if (lastSlashPos != std::string::npos) {
        std::string afterLastSlash = location.substr(lastSlashPos + 1);
        return afterLastSlash;
    }
    else {
        return location;
    }
}

void JsonToPUml::draw_bases(json &cls)
{
    for (auto &base : cls["bases"].items()) {
        std::ostringstream ss;
        ss << cls["name"].get<std::string>() << " -up[#red]-|> \""
           << base.value().get<std::string>() << "\"\n";
        std::string s = ss.str();
        puml.write(s.c_str(), s.length());
    }
}

void JsonToPUml::draw_fields(json &cls)
{
    std::set<std::string> written;

    for (auto &fld : cls["fields"].items()) {
        for (auto &type : fld.value()["types"].items()) {
            if (type.value()["isBasic"].get<bool>()) {
                continue;
            }
            std::ostringstream ss;
            ss << cls["name"].get<std::string>()
               << (fld.value()["is_aggregate"] ? " -down[#blue]-o "
                                               : " -down[#green]-* ")
               << "\"" << type.value()["name"].get<std::string>() << "\"\n";
            std::string s = ss.str();
            if (written.insert(s).second) {
                puml.write(s.c_str(), s.length());
            }
        }

        for (auto &type : fld.value()["templates"].items()) {
            if (type.value()["isBasic"].get<bool>()) {
                continue;
            }
            std::ostringstream ss;
            ss << cls["name"].get<std::string>() << " -down-* \""
               << type.value()["name"].get<std::string>() << "\"\n";
            std::string s = ss.str();
            if (written.insert(s).second) {
                puml.write(s.c_str(), s.length());
            }
        }
    }
}

void JsonToPUml::draw_class(json &cls)
{
    std::ostringstream ss;
    ss << "class \"" << cls["name"].get<std::string>() << "\" {\n";
    ss << getLoc(cls) << "\n";
    ss << "==\n";
    std::string s = ss.str();
    puml.write(s.c_str(), s.length());
    for (auto &fld : cls["fields"].items()) {
        if (!basic) {
            if (fld.value()["isBasic"].get<bool>()) {
                continue;
            }
        }
        std::ostringstream ss;
        if (fld.value()["is_static"]) {
            ss << "{static} ";
        }
        ss << "  " << fld.value()["name"].get<std::string>() << " : "
           << fld.value()["type"].get<std::string>() << "\n";
        std::string s = ss.str();
        puml.write(s.c_str(), s.length());
    }
    puml.write("}\n", 2);
}

void JsonToPUml::save(std::string output)
{
    puml.open(output);
    if (!puml) {
        llvm::errs() << "Could not open output file: " << output << "\n";
        return;
    }

    puml.write("@startuml\n", 10);
    puml.write("left to right direction\n", 24);
    for (const auto &cls : classes.items()) {
        draw_class(cls.value());
        draw_bases(cls.value());
        draw_fields(cls.value());
    }
    puml.write("@enduml\n", 8);

    puml.close();
    if (verbose) {
        llvm::outs() << output << " generated\n";
    }
}
