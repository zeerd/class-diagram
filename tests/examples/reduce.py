from examples.utils import *

def case_reduce():
    data = read_json('reduce.cpp.json')
    expect_eq(data['Basic']['fields'][0]['types'][0]['isReduced'] , True)
    expect_eq(data['Std']['fields'][0]['types'][0]['isReduced'] , True)
    expect_eq(data['Std']['fields'][0]['templates'][0]['isReduced'] , True)
    expect_eq(data['NonStd']['fields'][0]['types'][0]['isReduced'] , True)
    expect_eq(data['NonStd']['fields'][0]['templates'][0]['isReduced'] , False)
    expect_eq(data['Func']['fields'][0]['types'][0]['isReduced'] , True)
    expect_eq(data['StdMutex']['fields'][0]['isReduced'] , True)
    expect_eq(data['StdMutex']['fields'][0]['types'][0]['isReduced'] , True)
    expect_eq(data['StdVector']['fields'][0]['isReduced'] , False)
    expect_eq(data['StdVector']['fields'][0]['types'][0]['isReduced'] , False)
