#ifndef SVG_TO_HTML_HPP
#define SVG_TO_HTML_HPP

#include <string>

class SvgToHtml {
private:
    bool verbose;

public:
    explicit SvgToHtml(bool verb) : verbose(verb) {}
    void save(const std::string &input, const std::string &output) const;
};

#endif /* SVG_TO_HTML_HPP */
