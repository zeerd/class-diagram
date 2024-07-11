from examples.utils import *

def case_template():
    data = read_json('template.cpp.json')
    expect_eq(data['UseTemplate']['fields'][0]['templates'][0]['name'] , 'Template')
    expect_eq(data['UseTemplate']['fields'][0]['types'][0]['name'] , 'uint8_t')
    expect_true(any((t['name'] == 'map' for t in data['UseStd']['fields'][0]['templates'])))
    expect_true(any((t['name'] == 'pair' for t in data['UseStd']['fields'][0]['templates'])))
    expect_true(any((t['name'] == 'uint8_t' for t in data['UseStd']['fields'][0]['types'])))
    expect_true(any((t['name'] == 'Foo' for t in data['UseStd']['fields'][0]['types'])))
    expect_true(any((t['name'] == '_Bool' for t in data['UseStd']['fields'][0]['types'])))
