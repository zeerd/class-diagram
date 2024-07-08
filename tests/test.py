#!/usr/bin/env python3

import subprocess
import sys
import os
import json

executable_path = ""
test_file_path = ""

def read_json(file_path):
    with open(file_path, 'r') as f:
        data = json.load(f)
    return data

def case_basic():
    data = read_json('basic.cpp.json')
    assert len(data) == 6, "class number"
    assert any(data[d]['name'] == 'Bar' for d in data)
    assert data['Bar']['kind'] == 'class'
    assert any(data[d]['name'] == 'Foo' for d in data)
    assert any(t['name'] == 'Foo' for t in data['Bar']['fields'][0]['types'])
    assert data['Bar']['fields'][0]['types'][0]['isBasic'] == False
    assert data['Foo']['fields'][0]['types'][0]['isBasic'] == True
    assert data['Child']['bases'][0] == 'Bar'
    assert data['UseTempl']['fields'][0]['templates'][0]['name'] == 'Templ'
    assert data['UseTempl']['fields'][0]['types'][0]['name'] == 'uint8_t'
    assert any(t['name'] == 'map' for t in data['UseStd']['fields'][0]['templates'])
    assert any(t['name'] == 'pair' for t in data['UseStd']['fields'][0]['templates'])
    assert any(t['name'] == 'uint8_t' for t in data['UseStd']['fields'][0]['types'])
    assert any(t['name'] == 'Foo' for t in data['UseStd']['fields'][0]['types'])
    assert any(t['name'] == '_Bool' for t in data['UseStd']['fields'][0]['types'])

def case_array():
    data = read_json('array.cpp.json')
    assert data['Array']['fields'][0]['types'][0]['name'] == 'Foo'
    assert data['Pointer']['fields'][0]['types'][0]['name'] == 'Foo'
    assert data['PointerArray']['fields'][0]['types'][0]['name'] == 'Foo'

def case_types():
    data = read_json('types.cpp.json')
    assert data['Ast']['kind'] == 'struct'
    assert data['AEnum']['kind'] == 'enum'
    assert data['AUnion']['kind'] == 'union'
    assert data['EnumClass']['fields'][0]['types'][0]['name'] == 'AEnum'
    assert data['StClass']['fields'][0]['types'][0]['name'] == 'Ast'
    assert data['ConstClass']['fields'][0]['types'][0]['name'] == 'Ast'
    assert data['UnionClass']['fields'][0]['types'][0]['name'] == 'AUnion'

def case_typedef():
    data = read_json('typedef.cpp.json')
    assert data['Ast']['kind'] == 'struct'
    assert data['AEnum']['kind'] == 'enum'
    assert data['AUnion']['kind'] == 'union'
    assert data['EnumClass']['fields'][0]['types'][0]['name'] == 'AEnum'
    assert data['StClass']['fields'][0]['types'][0]['name'] == 'Ast'
    assert data['ConstClass']['fields'][0]['types'][0]['name'] == 'Ast'
    assert data['UnionClass']['fields'][0]['types'][0]['name'] == 'AUnion'

function_map = {
    "basic.cpp": case_basic,
    "array.cpp": case_array,
    "types.cpp": case_types,
    "typedef.cpp": case_typedef,
}

def run_test(test_folder, test_file):
    case_file = os.path.join(test_folder, test_file)
    cmd = [executable_path, "--input="+case_file, "--exclude=/usr", "--verbose"]
    with open(test_file+'.log', 'w') as f:
        result = subprocess.run(cmd, stdout=f)

    function_map[test_file]()

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 run_test.py <executable_path> <test_file_path>")
        sys.exit(1)

    executable_path = sys.argv[1]
    test_file_path = sys.argv[2]

    for root, dirs, files in os.walk(test_file_path):
        for file in files:
            run_test(root, file)
