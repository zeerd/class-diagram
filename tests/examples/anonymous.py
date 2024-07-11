from examples.utils import *

def case_anonymous():
    data = read_json('anonymous.cpp.json')
    expect_eq(data['AnonymousUnion']['fields'][0]['name'] , '(anonymous)')
    expect_eq(data['AnonymousUnion']['fields'][0]['type'] , 'union')
    expect_eq(data['AnonymousUnion']['fields'][0]['types'][0]['name'] , '')
    expect_eq(data['AnonymousStruct']['fields'][0]['name'] , '(anonymous)')
    expect_eq(data['AnonymousStruct']['fields'][0]['type'] , 'struct')
    expect_eq(data['AnonymousStruct']['fields'][0]['types'][0]['name'] , '')
