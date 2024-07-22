from examples.utils import *

def case_deduction():
    data = read_json('deduction.cpp.json')
    expect_true(any((f['type'] == 'auto' for f in data['AutoClass']['fields'])))
    expect_eq(data['DecltypeClass']['fields'][0]['types'][0]['type'] , 'int')
    expect_true(any((f['name'] == 'a' and f['types'][0]['type'] == 'T' for f in data['DependentTypeClass']['fields'])))
    expect_true(any((f['name'] == 'b' and f['types'][0]['type'] == '<dependent type>' for f in data['DependentTypeClass']['fields'])))
