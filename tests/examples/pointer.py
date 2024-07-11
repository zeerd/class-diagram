from examples.utils import *

def case_pointer():
    data = read_json('pointer.cpp.json')
    expect_eq(data['Array']['fields'][0]['types'][0]['name'] , 'Foo')
    expect_eq(data['BasicPointer']['fields'][0]['types'][0]['name'] , 'int')
    expect_eq(data['BasicPointer']['fields'][0]['isReduced'] , True)
    expect_eq(data['BasicPointer']['fields'][0]['types'][0]['isReduced'] , True)
    expect_eq(data['ClzPointer']['fields'][0]['types'][0]['name'] , 'Foo')
    expect_eq(data['ClzPointer']['fields'][0]['isReduced'] , False)
    expect_eq(data['ClzPointer']['fields'][0]['types'][0]['isReduced'] , False)
    expect_eq(data['PointerArray']['fields'][0]['types'][0]['name'] , 'Foo')
    expect_eq(data['Reference']['fields'][0]['types'][0]['name'] , 'Foo')
