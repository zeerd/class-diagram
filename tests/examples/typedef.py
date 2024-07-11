from examples.utils import *

def case_typedef():
    data = read_json('typedef.cpp.json')
    expect_eq(data['Ast']['kind'] , 'struct')
    expect_eq(data['AEnum']['kind'] , 'enum')
    expect_eq(data['AUnion']['kind'] , 'union')
    expect_eq(data['EnumClass']['fields'][0]['types'][0]['name'] , 'AEnum')
    expect_eq(data['StClass']['fields'][0]['types'][0]['name'] , 'Ast')
    expect_eq(data['ConstClass']['fields'][0]['types'][0]['name'] , 'Ast')
    expect_eq(data['UnionClass']['fields'][0]['types'][0]['name'] , 'AUnion')
