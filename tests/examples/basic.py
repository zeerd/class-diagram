from examples.utils import *

def case_basic():
    data = read_json('basic.cpp.json')
    expect_eq(len(data), 3)
    expect_true(any((data[d]['name'] == 'Bar' for d in data)))
    expect_eq(data['Bar']['kind'] , 'class')
    expect_true(any((data[d]['name'] == 'Foo' for d in data)))
    expect_true(any((t['name'] == 'Foo' for t in data['Bar']['fields'][0]['types'])))
    expect_eq(data['Bar']['fields'][0]['types'][0]['isReduced'] , False)
    expect_eq(data['Foo']['fields'][0]['types'][0]['isReduced'] , True)
    expect_eq(data['Child']['bases'][0] , 'Bar')

