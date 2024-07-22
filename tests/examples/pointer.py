from examples.utils import *

def case_pointer():
    data = read_json('pointer.cpp.json')
    expect_eq(data['Array']['fields'][0]['types'][0]['type'] , 'int')
    expect_true(data['Array']['fields'][0]['isReduced'])
    expect_true(data['Array']['fields'][0]['types'][0]['isReduced'])
    expect_eq(data['ClzArray']['fields'][0]['types'][0]['type'] , 'Foo')
    expect_eq(data['BasicPointer']['fields'][0]['types'][0]['type'] , 'int')
    expect_true(data['BasicPointer']['fields'][0]['isReduced'])
    expect_true(data['BasicPointer']['fields'][0]['types'][0]['isReduced'])
    expect_eq(data['ClzPointer']['fields'][0]['types'][0]['type'] , 'Foo')
    expect_false(data['ClzPointer']['fields'][0]['isReduced'])
    expect_false(data['ClzPointer']['fields'][0]['types'][0]['isReduced'])
    expect_eq(data['PointerArray']['fields'][0]['types'][0]['type'] , 'Foo')
    expect_eq(data['Reference']['fields'][0]['types'][0]['type'] , 'Foo')
