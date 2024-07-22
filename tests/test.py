#!/usr/bin/env python3

import argparse
import os
import subprocess
import sys

from examples.anonymous import case_anonymous
from examples.basic import case_basic
from examples.deduction import case_deduction
from examples.function import case_function
from examples.namespace import case_namespace
from examples.pointer import case_pointer
from examples.reduce import case_reduce
from examples.template import case_template
from examples.typedef import case_typedef
from examples.types import case_types
from examples.utils import setup, teardown, end
from examples.coverage import cov_do, cov_end

function_map = {
    "anonymous.cpp": case_anonymous,
    "basic.cpp": case_basic,
    "deduction.cpp": case_deduction,
    "function.cpp": case_function,
    "namespace.cpp": case_namespace,
    "pointer.cpp": case_pointer,
    "reduce.cpp": case_reduce,
    "template.cpp": case_template,
    "typedef.cpp": case_typedef,
    "types.cpp": case_types,
}

def run_test(binary, test_folder, test_file, args):
    case_file = os.path.join(test_folder, test_file)
    cmd = [binary, "--input="+case_file, "--reduce=/usr", "--reduce-ns=ExNS",
           "--verbose", "--all", "--reversal", "--unlinked"]
    if args.jar:
        cmd.extend(["--plantuml=" + args.jar])

    if os.path.exists(test_file + '.json'):
        os.remove(test_file + '.json')
    with open(test_file+'.log', 'w') as f:
        result = subprocess.run(cmd, stdout=f, stderr=f)
        if result.returncode != 0:
            print("[\033[31mFailed\033[0m] run test: " + ' '.join(cmd))
            teardown(1)
        else:
            setup(test_file)
            function_map[test_file]()
            teardown()
            if args.cov:
                cov_do(test_file)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Parse Parameter from Command line')
    parser.add_argument('--bin', required=True, help='executable path')
    parser.add_argument('--cases', required=True, help='test file path')
    parser.add_argument('--jar', required=False, help='plantuml.jar path')
    parser.add_argument('--cov', required=False, action='store_true', help='Code Coverage')
    args = parser.parse_args()

    for root, dirs, files in os.walk(args.cases):
        for file in files:
            if file.endswith('.cpp'):
                run_test(args.bin, root, file, args)

    if args.cov:
        cov_end()
    end()
