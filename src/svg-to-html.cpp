#include "svg-to-html.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

static const std::string jsInit = R"(
    var polygons = document.querySelectorAll('polygon');
    var ellipses = document.querySelectorAll('ellipse');
    var paths = document.querySelectorAll('path');
    var rects = document.querySelectorAll('rect');
    var texts = document.querySelectorAll('text');
)";

static const std::string jsClickArrows = R"(
    function defClickArrows(sets) {
        for (var i = 0; i < sets.length; i++) {
            var polygon = sets[i];

            var fill = polygon.getAttribute('fill');
            if (!fill || fill === 'none' || fill === 'transparent') {
                polygon.style.fill = 'rgba(0, 0, 0, 0)';
            }

            polygon.addEventListener('click', function(event) {
                var parent = event.currentTarget.parentNode;
                var path = parent.querySelector('path');
                if (!path) {
                    return;
                }
                if (path.style.strokeWidth === '3px') {
                    path.style.strokeWidth = '1px';
                } else {
                    for (var j = 0; j < paths.length; j++) {
                        paths[j].style.strokeWidth = '1px';
                    }

                    path.style.strokeWidth = '3px';
                }
            });
        }
    }
    defClickArrows(polygons);
    defClickArrows(ellipses);
)";

static const std::string jsClickLines = R"(
    function resetLines(id) {
        for (var i = 0; i < paths.length; i++) {
            var path = paths[i];
            if (path.id.split('-').some(part => part === id)) {
                path.style.strokeWidth = '3px';
            }
        }
    }
    for (var i = 0; i < paths.length; i++) {
        var path = paths[i];

        path.addEventListener('click', function(event) {
            var parent = event.currentTarget.parentNode;
            if (parent.tagName === 'g' && parent.id.startsWith('cluster_')) {

                var idPart = parent.id.split('_')[1];
                resetLines(idPart);
            }
            else {
                if (event.currentTarget.style.strokeWidth === '3px') {
                    event.currentTarget.style.strokeWidth = '1px';
                } else {
                    for (var j = 0; j < paths.length; j++) {
                        paths[j].style.strokeWidth = '1px';
                    }

                    event.currentTarget.style.strokeWidth = '3px';
                }
            }
        });
    }
)";

static const std::string jsClickClass = R"(
    for (var i = 0; i < rects.length; i++) {
        var rect = rects[i];

        var fill = rect.getAttribute('fill');
        if (!fill || fill === 'none' || fill === 'transparent') {
            rect.style.fill = 'rgba(0, 0, 0, 0)';
        }
        rect.addEventListener('click', function(event) {
            for (var j = 0; j < paths.length; j++) {
                paths[j].style.strokeWidth = '1px';
            }
            resetLines(this.id);
        });
    }
    for (var i = 0; i < texts.length; i++) {
        var text = texts[i];

        var fill = text.getAttribute('fill');
        if (!fill || fill === 'none' || fill === 'transparent') {
            text.style.fill = 'rgba(0, 0, 0, 0)';
        }
        text.addEventListener('click', function(event) {
            var parent = event.currentTarget.parentNode;
            if (parent.tagName === 'g') {
                for (var j = 0; j < paths.length; j++) {
                    paths[j].style.strokeWidth = '1px';
                }
                var idPart = parent.id.split('_')[1];
                resetLines(idPart);
            }
        });
    }
)";

static const std::string jsClickBlank = R"(
    document.body.addEventListener('click', function(event) {
        if (event.target instanceof SVGSVGElement) {
            for (var i = 0; i < paths.length; i++) {
                paths[i].style.strokeWidth = '1px';
            }
            for (var i = 0; i < rects.length; i++) {
                rects[i].style.stroke = '#181818';
                rects[i].style.strokeWidth = '0.5px';
            }
        }
    });
)";

static const std::string jsDBClickText = R"(
    function getRichSelected() {
        var selection = window.getSelection();
        var range = selection.getRangeAt(0);
        var selectedText = selection.toString();

        var node = range.startContainer;
        var text = node.textContent;
        var startOffset = range.startOffset;

        if(startOffset > 1
          && text.charAt(startOffset - 1) === ':'
          && text.charAt(startOffset - 2) === ':') {
            while (startOffset > 0 && text.charAt(startOffset - 1) !== ' ') {
                startOffset--;
            }

            var newRange = document.createRange();
            newRange.setStart(node, startOffset);
            newRange.setEnd(node, range.endOffset);

            selection.removeAllRanges();
            selection.addRange(newRange);
        }
        return selection.toString();
    }

    function markRect(selectedText) {
        var found = false;
        if (selectedText != "") {
            for (var i = 0; i < rects.length; i++) {
                var rect = rects[i];
                if (rect.id === selectedText) {
                    rect.style.stroke = 'magenta';
                    rect.style.strokeWidth = '3px';
                    rect.scrollIntoView({ behavior: 'smooth', block: 'center' });
                    found = true;
                } else {
                    rect.style.stroke = '#181818';
                    rect.style.strokeWidth = '0.5px';
                }
            }
        }
        return found;
    }

    document.body.addEventListener('dblclick', function(event) {
        var text = getRichSelected();
        var pos = text.lastIndexOf(' ');
        var last = text.substring(pos + 1);
        var parts = last.split("::");
        for (var i = 0; i < parts.length; i++) {
            var part = parts.slice(i).join("::");
            if(markRect(part)) {
                break;
            }
        }
    });
)";

void SvgToHtml::save(const std::string& input, const std::string& output) const
{
    std::ifstream svg_file(input);
    // LCOV_EXCL_START
    if (!svg_file.is_open()) {
        return;
    }
    // LCOV_EXCL_STOP

    std::stringstream svg_content;
    svg_content << svg_file.rdbuf();

    std::stringstream html_content;
    html_content << R"(
    <html lang='zh-CN'>
    <head>
        <meta charset='utf-8'/>
        <title>)" << output
                 << R"(</title>
    </head>
    <body>
    )" << svg_content.str()
                 << R"(
    <script>
    )" << jsInit << jsClickArrows
                 << jsClickLines << jsClickClass << jsClickBlank
                 << jsDBClickText << R"(
    </script>
    </body>
    </html>
    )";

    std::ofstream html_file(output);
    html_file << html_content.str();
    html_file.close();

    if (verbose) {
        std::cout << output << " generated\n\n";
    }
}
