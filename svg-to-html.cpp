#include "class-diagram.hpp"

void SvgToHtml::save(std::string input, std::string output)
{
    std::ifstream svg_file(input);
    if (!svg_file.is_open()) {
        return;
    }

    std::stringstream svg_content;
    svg_content << svg_file.rdbuf();

    std::string js_code = R"(
        var polygons = document.querySelectorAll('polygon');
        var paths = document.querySelectorAll('path');
        var rects = document.querySelectorAll('rect');

        for (var i = 0; i < polygons.length; i++) {
            var polygon = polygons[i];

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

        for (var i = 0; i < paths.length; i++) {
            var path = paths[i];

            path.addEventListener('click', function(event) {
                if (event.currentTarget.style.strokeWidth === '3px') {
                    event.currentTarget.style.strokeWidth = '1px';
                } else {
                    for (var j = 0; j < paths.length; j++) {
                        paths[j].style.strokeWidth = '1px';
                    }

                    event.currentTarget.style.strokeWidth = '3px';
                }
            });
        }

        for (var i = 0; i < rects.length; i++) {
            var rect = rects[i];
            rect.addEventListener('click', function(event) {
                for (var j = 0; j < paths.length; j++) {
                    paths[j].style.strokeWidth = '1px';
                }
                var rectId = this.id;
                for (var i = 0; i < paths.length; i++) {
                    var path = paths[i];
                    if (path.id.includes(rectId)) {
                        path.style.strokeWidth = '3px';
                    }
                }
            });
        }

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

        document.body.addEventListener('dblclick', function(event) {
            var selectedText = window.getSelection().toString();

            for (var i = 0; i < rects.length; i++) {
                var rect = rects[i];

                if (rect.id === selectedText) {
                    rect.style.stroke = '#FF0000';
                    rect.style.strokeWidth = '3px';
                    rect.scrollIntoView({ behavior: 'smooth', block: 'center' });
                } else {
                    rect.style.stroke = '#181818';
                    rect.style.strokeWidth = '0.5px';
                }
            }
        });
    )";

    std::stringstream html_content;
    html_content << R"(
    <html>
    <body>
    )" << svg_content.str()
                 << R"(
    <script>
    )" << js_code << R"(
    </script>
    </body>
    </html>
    )";

    std::ofstream html_file(output);
    html_file << html_content.str();
    html_file.close();

    if (verbose) {
        llvm::outs() << output << " generated\n";
    }
}
