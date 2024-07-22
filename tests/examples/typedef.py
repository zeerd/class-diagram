from examples.utils import *

def case_typedef():
    data = read_json('typedef.cpp.json')
    expect_eq(data['TypeDefAst']['kind'] , 'struct')
    expect_eq(data['AEnum']['kind'] , 'enum')
    expect_eq(data['AUnion']['kind'] , 'union')
    expect_eq(data['NamedTypeDefAst']['kind'] , 'struct')
    expect_eq(data['NamedTypeDefAst']['typedef'] , 'StAst')
    expect_eq(data['StAst']['kind'] , 'struct')
    expect_eq(data['EnumClass']['fields'][0]['types'][0]['type'] , 'AEnum')
    expect_eq(data['StClass']['fields'][0]['types'][0]['type'] , 'StAst')
    expect_eq(data['ConstClass']['fields'][0]['types'][0]['type'] , 'TypeDefAst')
    expect_eq(data['UnionClass']['fields'][0]['types'][0]['type'] , 'AUnion')
    expect_eq(data['UsingClass']['fields'][0]['types'][0]['type'] , 'unsigned long')
    expect_false(data['abc::UsingTemplateTypenameClass']['fields'][0]['templates'][0]['isReduced'])
