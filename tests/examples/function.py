from examples.utils import *

def case_function():
    data = read_json('function.cpp.json')
    expect_true(data['FuncClass']['fields'][0]['types'][0]['isReduced'])
    expect_true(data['StdFuncClass']['fields'][0]['types'][0]['isReduced'])
    expect_false(data['ClzFuncClass']['fields'][0]['types'][0]['isReduced'])
    expect_eq(data['ClzFuncClass']['fields'][0]['types'][0]['type'] , 'Clz')
    expect_false(data['Clz']['isComplete'])
