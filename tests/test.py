#!/usr/bin/env python3

import os
import subprocess
import sys

from examples.anonymous import case_anonymous
from examples.basic import case_basic
from examples.function import case_function
from examples.namespace import case_namespace
from examples.pointer import case_pointer
from examples.reduce import case_reduce
from examples.template import case_template
from examples.typedef import case_typedef
from examples.types import case_types

function_map = {
    "anonymous.cpp": case_anonymous,
    "basic.cpp": case_basic,
    "function.cpp": case_function,
    "namespace.cpp": case_namespace,
    "pointer.cpp": case_pointer,
    "reduce.cpp": case_reduce,
    "template.cpp": case_template,
    "typedef.cpp": case_typedef,
    "types.cpp": case_types,
}

def run_test(binary, test_folder, test_file):
    case_file = os.path.join(test_folder, test_file)
    cmd = [binary, "--input="+case_file, "--exclude=/usr",
           "--verbose", "--exclude-ns=abc"]
    # print(' '.join(cmd))

    with open(test_file+'.log', 'w') as f:
        result = subprocess.run(cmd, stdout=f, stderr=f)

    function_map[test_file]()

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 run_test.py <executable_path> <test_file_path>")
        sys.exit(1)

    executable_path = sys.argv[1]
    test_file_path = sys.argv[2]

    for root, dirs, files in os.walk(test_file_path):
        for file in files:
            if file.endswith('.cpp'):
                run_test(executable_path, root, file)

    print("\nDone!")
