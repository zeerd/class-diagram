from examples.utils import *

def case_basic():
    data = read_json('basic.cpp.json')
    expect_eq(len(data), 4)
    expect_true(any((data[d]['name'] == 'Bar' for d in data)))
    expect_eq(data['Bar']['kind'] , 'class')
    expect_true(data['Bar']['isComplete'])
    expect_true(any((data[d]['name'] == 'Foo' for d in data)))
    expect_true(any((t['type'] == 'Foo' for t in data['Bar']['fields'][0]['types'])))
    expect_false(data['Bar']['fields'][0]['types'][0]['isReduced'])
    expect_true(data['Foo']['fields'][0]['types'][0]['isReduced'])
    expect_eq(data['Child']['bases'][0] , 'Bar')
    expect_true(any((b == 'Foo' for b in data['Two']['bases'])))
    expect_true(any((b == 'Bar' for b in data['Two']['bases'])))
