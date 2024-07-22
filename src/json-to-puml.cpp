#include "json-to-puml.hpp"

#include <iostream>
#include <regex>

void JsonToPUml::drawBases(json &cls)
{
    if (!cls["isReduced"]) {
        for (auto &base : cls["bases"].items()) {
            std::ostringstream ss;
            std::string left = cls["name"];
            if (!cls["isInner"]) {
                left = getLastName(left);
            }
            std::string right = base.value();
            if (classes.contains(right)) {
                if (classes[right]["isReduced"]) {
                    /** A base-class might be reduced class(i.e. std::xxx).
                     *  This will add a dotted block for QA-3636 that guessing
                     *  a classname with namespace as field incorrectly.
                     *  https://forum.plantuml.net/3636
                     */
                    std::ostringstream c;
                    c << "class \"" << right << "\" #line.dotted {}\n";
                    if (written.insert(c.str()).second) {
                        ss << c.str();
                    }
                }
                if ((!classes[right]["isInner"])
                    && (!classes[right]["isReduced"])) {
                    right = getLastName(right);
                }
            }
            ss << left << " -up[#red]-|> \"" << right << "\"\n";
            write(ss.str());
        }
    }
}

void JsonToPUml::drawFields(json &cls)
{
    auto draw = [&](auto &type, const char *dash) {
        if (type.value()["isReduced"]) {
        }
        else {
            auto removeNS = [&](const std::string &from, std::string ns) {
                std::string ret = from;
                if (ns != "") {
                    if (verbose) {
                        std::cout << "Remove namespace[" << ns << "] from ["
                                  << ret << "]" << "\n";
                    }
                    if (ret.compare(0, ns.length(), ns) == 0) {
                        ret = ret.substr(ns.length() + 2, -1);
                    }
                }
                return ret;
            };

            std::ostringstream ss;
            std::string left = cls["name"];
            left             = removeNS(left, cls["namespace"]);
            if (!(cls["isInner"])) {
                left = getLastName(left);
            }

            std::string right = type.value()["type"];

            if (classes.contains(right) && !classes[right]["isComplete"]) {
                drawClass(classes[right], true);
            }

            right = removeNS(right, type.value()["namespace"]);
            {
                std::regex r(R"((\w+)<.*>)");
                std::smatch m;
                if (std::regex_match(right, m, r)) {
                    right = m[1];
                }
            }

            auto isTemplates = [&](json &clz, std::string &n) {
                if (std::find(clz["templates"].begin(), clz["templates"].end(),
                              n)
                    != clz["templates"].end()) {
                    return true;
                }
                return false;
            };
            if (classes.contains(right) && !(classes[right]["isInner"])) {
                right = getLastName(right);
            }

            if (left != "" && right != "") {
                ss << "\"" << left << "\"" << dash << "\"" << right << "\"\n";
                if (written.insert(ss.str()).second) {
                    write(ss.str());
                    drawTypedef(right);
                }
            }
        }
    };

    if (!cls["isReduced"]) {
        if (verbose) {
            std::cout << "Draw fields for class [" << cls["name"] << "]\n";
        }

        for (auto &fld : cls["fields"].items()) {
            for (auto &type : fld.value()["types"].items()) {
                draw(type, " -down[#blue]-> ");
            }

            for (auto &type : fld.value()["templates"].items()) {
                draw(type, " -down[#green]-> ");
            }
        }
    }
}

void JsonToPUml::drawClass(const json &cls, bool drawInComplete)
{
    std::ostringstream ss;
    std::ostringstream ss_close;
    std::string part;

    std::map<std::string, std::string> visibility = {
        {"public", "+"},
        {"protected", "#"},
        {"private", "-"},
        {"unknown", ""},
    };

    auto drawContent = [&](const std::string &part) {
        ss << (cls["kind"].get<std::string>().length() > 0
                   ? (cls["kind"] == "union" ? "class"
                                             : cls["kind"].get<std::string>())
                   : "class");
        ss << " \"" << part << "\"";
        if (cls.contains("templates") && cls["templates"].size() > 0) {
            std::ostringstream t;
            auto items = cls["templates"];
            t << std::accumulate(
                std::next(items.begin()), items.end(),
                std::string(items.begin()->get<std::string>()),
                [](const std::string &a, const nlohmann::json &b) {
                    return a + ", " + b.get<std::string>();
                });
            ss << "<" << wrap(t.str(), 60) << ">";
        }
        if (cls["kind"] == "union") {
            /** Plantuml: Spot */
            ss << " << (U,#FF7700) >>";
        }
        if (!cls["isComplete"]) {
            ss << " #line.dotted";
        }
        ss << " {\n";
        ss << cls["location"].get<std::string>() << "\n";
        ss << "==\n";

        if (cls.contains("fields")) {
            for (auto &fld : cls["fields"].items()) {
                if (!basic) {
                    if (fld.value()["isReduced"].get<bool>()) {
                        continue;
                    }
                }
                if (fld.value()["isStatic"]) {
                    ss << "{static} ";
                }

                ss << visibility[fld.value()["visibility"]];
                std::ostringstream f;
                f << "  " << fld.value()["name"].get<std::string>() << " : "
                  << fld.value()["type"].get<std::string>() << "\n";

                ss << wrap(f.str(), 80);
            }
        }

        ss << "}\n";
    };

    auto drawNamespace = [&](std::string &n) {
        std::istringstream iss(n);
        while (std::getline(iss, part, ':')) {
            if (!part.empty()) {
                ss << "package \"" << part << "\" {\n";
                ss_close << "}\n";
            }
        }
    };

    auto drawPureName = [&](std::string &n) {
        std::istringstream iss(n);
        while (std::getline(iss, part, ':')) {
            if (!part.empty()) {
                if (iss.eof()) {
                    drawContent(part);
                    ss << ss_close.str();
                }
                else {
                    ss << "package \"" << part << "\" {\n";
                    ss_close << "}\n";
                }
                // std::cout << part << std::endl;
            }
        }
    };

    if (cls["isReduced"]) {
    }
    else if (!cls["isComplete"] && !drawInComplete) {
    }
    else {
        std::string name = cls["name"];
        std::string ns   = cls["namespace"];
        std::string clz  = name;
        if (ns != "" && name.compare(0, ns.length(), ns) == 0) {
            clz = name.substr(ns.length() + 2, -1);
            drawNamespace(ns);
        }
        if (cls["isInner"]) {
            drawContent(clz);
            ss << ss_close.str();
        }
        else {
            drawPureName(clz);
        }
    }

    if (written.insert(ss.str()).second) {
        write(ss.str());
    }
}

void JsonToPUml::drawNested(const json &cls)
{
    if (cls["isInner"] && (!cls["isReduced"])) {
        std::string name = cls["name"];
        {
            std::string ns = cls["namespace"];
            if (verbose) {
                std::cout << "Remove namespace[" << ns << "] from [" << name
                          << "]" << "\n";
            }
            if (ns != "" && name.compare(0, ns.length(), ns) == 0) {
                name = name.substr(ns.length() + 2, -1);
            }
        }

        if (!cls["isComplete"]) {
            drawClass(cls, true);
        }

        std::size_t pos = name.rfind("::");
        if (pos != std::string::npos) {
            std::string base = getLastName(name.substr(0, pos));
            std::ostringstream ss;
            ss << "\"" << base << "\" +-down[#purple]- \"" << name << "\"\n";
            write(ss.str());
        }
    }
}

void JsonToPUml::drawTypedef(const std::string &to)
{
    for (const auto &cls : classes.items()) {
        if ((!cls.value()["isReduced"]) && (cls.value()["typedef"] == to)) {
            std::string left  = cls.value()["name"];
            std::string right = cls.value()["typedef"];
            std::ostringstream ss;
            ss << "\"" << left << "\" .down[#C8902A].> \"" << right
               << "\" : <<typedef>> \n";
            std::string s = ss.str();
            if (written.insert(s).second) {
                write(s);
            }
        }
    }
}

void JsonToPUml::save(std::string output)
{
    uml.open(output);
    // LCOV_EXCL_START
    if (!uml) {
        std::cerr << "Could not open output file: " << output << "\n";
        return;
    }
    // LCOV_EXCL_STOP

    write("@startuml\n");
    if (reversal) {
        write("left to right direction\n");
    }
    if (hide) {
        write("remove @unlinked\n");
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
    write("@enduml\n");

    uml.close();
    if (verbose) {
        std::cout << output << " generated\n"
                  << "\n";
    }
}

std::string JsonToPUml::getLastName(const std::string &clz)
{
    std::string ret = clz;
    std::size_t pos = ret.rfind("::");
    if (pos != std::string::npos) {
        ret = ret.substr(pos + 2, -1);
    }
    return ret;
}

std::string JsonToPUml::wrap(const std::string &str, size_t width)
{
    std::string result;
    size_t begin = 0;

    while (begin < str.size()) {
        bool hit   = false;
        size_t end = begin + width;
        if (end > str.size()) {
            end = str.size();
        }
        else {
            hit = true;
        }

        while (end > begin && str[end] != ' ' && str[end] != ',') {
            --end;
        }

        if (end == begin) {
            end = begin + width;
        }

        result += str.substr(begin, end - begin);
        if (hit && end < str.size()) {
            result += "\\n";
        }

        begin = end;
    }

    return result;
}
