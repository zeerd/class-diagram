#include <set>

#include "class-diagram.hpp"

std::string JsonToPUml::getLoc(json &cls)
{
#if 1
    return cls["location"].get<std::string>();
#else
    std::string location     = cls["location"].get<std::string>();
    std::size_t lastSlashPos = location.find_last_of('/');
    if (lastSlashPos != std::string::npos) {
        std::string afterLastSlash = location.substr(lastSlashPos + 1);
        return afterLastSlash;
    }
    else {
        return location;
    }
#endif
}

std::string JsonToPUml::getLastName(std::string clz)
{
    std::string ret = clz;
    std::size_t pos = ret.rfind("::");
    if (pos != std::string::npos) {
        ret = ret.substr(pos + 2, -1);
    }
    return ret;
}

void JsonToPUml::drawBases(json &cls)
{
    for (auto &base : cls["bases"].items()) {
        std::ostringstream ss;
        std::string left = cls["name"];
        if (!cls["isInner"]) {
            left = getLastName(left);
        }
        std::string right = base.value();
        if (classes.contains(right) && (!classes[right]["isInner"])) {
            right = getLastName(right);
        }
        ss << left << " -up[#red]-|> \"" << right << "\"\n";
        std::string s = ss.str();
        uml.write(s.c_str(), s.length());
    }
}

void JsonToPUml::drawFields(json &cls)
{
    std::set<std::string> written;

    auto write = [&](auto &type, std::string dash) {
        if (type.value()["isReduced"]) {
        }
        else {
            std::string left = cls["name"];
            if (!(cls["isInner"])) {
                left = getLastName(left);
            }
            std::string right = type.value()["name"];
            if (classes.contains(right) && !(classes[right]["isInner"])) {
                right = getLastName(right);
            }
            if (left != "" && right != "") {
                std::ostringstream ss;
                ss << "\"" << left << "\"" << dash << "\"" << right << "\"\n";
                std::string s = ss.str();
                if (written.insert(s).second) {
                    uml.write(s.c_str(), s.length());
                }
            }
        }
    };

    for (auto &fld : cls["fields"].items()) {
        for (auto &type : fld.value()["types"].items()) {
            write(type, " -down[#blue]-> ");
        }

        for (auto &type : fld.value()["templates"].items()) {
            write(type, " -down[#green]-> ");
        }
    }
}

void JsonToPUml::drawClass(json &cls)
{
    std::string name = cls["name"];
    std::ostringstream ss;

    auto draw = [&](std::string part) {
        ss << (cls["kind"].get<std::string>().length() > 0
                   ? (cls["kind"] == "union" ? "class"
                                             : cls["kind"].get<std::string>())
                   : "class");
        ss << " \"" << part << "\" {\n";
        ss << getLoc(cls) << "\n";
        ss << "==\n";

        for (auto &fld : cls["fields"].items()) {
            if (!basic) {
                if (fld.value()["isReduced"].get<bool>()) {
                    continue;
                }
            }
            if (fld.value()["is_static"]) {
                ss << "{static} ";
            }
            ss << "  " << fld.value()["name"].get<std::string>() << " : "
               << fld.value()["type"].get<std::string>() << "\n";
        }

        ss << "}\n";
    };

    if (cls["isInner"]) {
        draw(name);
    }
    else {
        std::ostringstream ss_close;
        std::istringstream iss(name);
        std::string part;
        while (std::getline(iss, part, ':')) {
            if (!part.empty()) {
                if (iss.eof()) {
                    draw(part);
                    ss << ss_close.str();
                }
                else {
                    ss << "package \"" << part << "\" {\n";
                    ss_close << "}\n";
                }
                // std::cout << part << std::endl;
            }
        }
    }

    std::string s = ss.str();
    uml.write(s.c_str(), s.length());
}

void JsonToPUml::drawNested(json &cls)
{
    if (cls["isInner"]) {
        std::string name = cls["name"];
        std::size_t pos  = name.rfind("::");
        if (pos != std::string::npos) {
            std::string base = getLastName(name.substr(0, pos));
            std::ostringstream ss;
            ss << "\"" << base << "\" +-- \"" << name << "\"\n";
            std::string s = ss.str();
            uml.write(s.c_str(), s.length());
        }
    }
}

void JsonToPUml::save(std::string output)
{
    uml.open(output);
    if (!uml) {
        std::cerr << "Could not open output file: " << output << "\n";
        return;
    }

    uml.write("@startuml\n", 10);
    uml.write("left to right direction\n", 24);
    if (hide) {
        uml.write("remove @unlinked\n", 17);
    }
    for (const auto &cls : classes.items()) {
        drawClass(cls.value());
    }
    for (const auto &cls : classes.items()) {
        drawNested(cls.value());
    }
    for (const auto &cls : classes.items()) {
        drawBases(cls.value());
    }
    for (const auto &cls : classes.items()) {
        drawFields(cls.value());
    }
    uml.write("@enduml\n", 8);

    uml.close();
    if (verbose) {
        std::cout << output << " generated\n";
    }
}
