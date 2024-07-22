
Traverse C++ files in a given folder and generate a class diagram using PlantUML.

# Usage

```
C++ Class Diagram Generator:
Generate Class Diagram from C++ source/header files.

  --all                - Show all members(false as default)
  --unlinked           - Show unlinked classes(false as default)
  --input=<string>     - input a folder of a project or a single file
  --exclude=<string>   - Exclude location for the input(could use several times)
  --include=<string>   - Extra include location of header files(could use several times)
  --output=<string>    - Specify output basename(use the last part of input if not given)
  --reduce=<string>    - Exclude location for the output(could use several times)
  --reduce-ns=<string> - Exclude namespace for the output(could use several times)
  --plantuml=<string>  - Specify the path of plantuml.jar file
  --java=<string>      - Specify the full path of java binary for plantuml.jar
  --theme=<string>     - Specify the path of theme file for plantuml.jar
  --mod-version        - Show the version of this
  --verbose            - Show more logs(false as default)
```

# Build

```
sudo apt-get install libclang-dev libclang-cpp-dev llvm-dev clang
mkdir .build
cd .build
cmake ..
make -j
```

# Test

```
cd .build
make -j && ../tests/test.py --bin ./class-diagram --cases ../tests/examples/
```

or

```
cd .build
cmake .. -DTEST_COVERAGE=ON
make -j && ../tests/test.py --bin ./class-diagram --cases ../tests/examples/ --jar /path/to/plantuml.jar --cov
```

# Code Style and Check

```
find src/ -name "*.cpp" -o -name "*.hpp" | xargs -x clang-format-18 -i
cppcheck --enable=all --inconclusive --force src/
```

# Known Issues

* Any type that could not be found in the include paths would be identify as int.
