#include "class-diagram.hpp"

#include "class-parser.hpp"

int main(int argc, const char **argv)
{
    ClassParser gen;
    gen.parseProject(argc, argv);

    JsonToFile f(gen.classes(), gen.verbose());
    f.save(gen.output() + ".json");
    JsonToPUml p(gen.classes(), gen.verbose(), gen.all(), gen.hide());
    p.save(gen.output() + ".puml");
    PUmlToSvg s(gen.verbose(), gen.java(), gen.plantUML(), gen.theme());
    s.save(gen.output() + ".puml", gen.output() + ".svg");
    SvgToHtml h(gen.verbose());
    h.save(gen.output() + ".svg", gen.output() + ".html");

    return 0;
}
