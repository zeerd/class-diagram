from examples.utils import *

def case_template():
    data = read_json('template.cpp.json')
    expect_eq(data['UseTemplate']['fields'][0]['templates'][0]['type'] , 'Template')
    expect_eq(data['UseTemplate']['fields'][0]['types'][0]['type'] , 'unsigned char')
    expect_true(any((t['type'] == 'map' for t in data['UseStd']['fields'][0]['templates'])))
    expect_true(any((t['type'] == 'pair' for t in data['UseStd']['fields'][0]['templates'])))
    expect_true(any((t['type'] == 'unsigned char' for t in data['UseStd']['fields'][0]['types'])))
    expect_true(any((t['type'] == 'Foo' for t in data['UseStd']['fields'][0]['types'])))
    expect_true(any((t['type'] == '_Bool' for t in data['UseStd']['fields'][0]['types'])))
    expect_false(data['Template']['fields'][0]['isReduced'])
    expect_true(data['Template']['fields'][0]['types'][0]['isReduced'])
    expect_eq(data['Template']['templates'][0] , 'T')
    expect_true(any((t == 'Type' for t in data['TypenameTemplate']['templates'])))
    expect_true(any((t == 'SubType' for t in data['TypenameTemplate']['templates'])))
