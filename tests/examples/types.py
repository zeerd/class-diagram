from examples.utils import *

def case_types():
    data = read_json('types.cpp.json')
    expect_eq(data['Ast']['kind'] , 'struct')
    expect_eq(data['AEnum']['kind'] , 'enum')
    expect_eq(data['AUnion']['kind'] , 'union')
    expect_eq(data['EnumClass']['fields'][0]['types'][0]['type'] , 'AEnum')
    expect_eq(data['StClass']['fields'][0]['types'][0]['type'] , 'Ast')
    expect_eq(data['ConstClass']['fields'][0]['types'][0]['type'] , 'Ast')
    expect_eq(data['UnionClass']['fields'][0]['types'][0]['type'] , 'AUnion')
    expect_true(data['ThreadClass']['fields'][0]['types'][0]['isReduced'])
