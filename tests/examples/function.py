from examples.utils import *

def case_function():
    data = read_json('function.cpp.json')
    expect_eq(data['FuncClass']['fields'][0]['types'][0]['isReduced'] , True)
    expect_eq(data['StdFuncClass']['fields'][0]['types'][0]['isReduced'] , True)
    expect_eq(data['ClzFuncClass']['fields'][0]['types'][0]['isReduced'] , False)
    expect_eq(data['ClzFuncClass']['fields'][0]['types'][0]['name'] , 'Clz')
