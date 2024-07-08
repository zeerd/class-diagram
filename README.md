
Traverse C++ files in a given folder and generate a class diagram using PlantUML.

# Usage

```
C++ Class Diagram Generator:
Generate Class Diagram from C++ source/header files.

  --basic                - Show basic members(false as default)
  --exclude=<string>     - Exclude location from the output(could use several times)
  --include=<string>     - Extra include location of header files(could use several times)
  --input=<string>       - Input a folder of a project or a single file
  --java=<string>        - Specify the full path of java binary for plantuml.jar
  --modversion           - Show the version of this
  --output=<string>      - Specify output basename(use the last part of input if not given)
  --plantuml=<string>    - Specify the full path of plantuml.jar file
  --verbose              - Show more logs(false as default)
```

# Build

```
sudo apt-get install libclang-dev libclang-cpp-dev llvm-dev clang
mkdir .build
cd .build
cmake ..
make
```

# Test

```
cd .build
../tests/test.py ./class-diagram ../tests/examples/
```

# Code Style

```
find . -name "*.cpp" -o -name "*.hpp" | xargs -x clang-format -i
```
