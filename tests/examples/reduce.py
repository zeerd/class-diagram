from examples.utils import *

def case_reduce():
    data = read_json('reduce.cpp.json')
    expect_true(data['Basic']['fields'][0]['types'][0]['isReduced'])
    expect_true(data['Std']['fields'][0]['types'][0]['isReduced'])
    expect_true(data['Std']['fields'][0]['templates'][0]['isReduced'])
    expect_true(data['NonStd']['fields'][0]['types'][0]['isReduced'])
    expect_false(data['NonStd']['fields'][0]['templates'][0]['isReduced'])
    expect_true(data['Func']['fields'][0]['types'][0]['isReduced'])
    expect_true(data['StdMutex']['fields'][0]['isReduced'])
    expect_true(data['StdMutex']['fields'][0]['types'][0]['isReduced'])
    expect_false(data['StdVector']['fields'][0]['isReduced'])
    expect_false(data['StdVector']['fields'][0]['types'][0]['isReduced'])
