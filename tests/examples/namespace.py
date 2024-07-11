from examples.utils import *

def case_namespace():
    data = read_json('namespace.cpp.json')
    expect_eq(data['NsClass']['fields'][0]['types'][0]['isReduced'] , True)
    expect_eq(data['NsClass']['isInner'] , False)
    expect_eq(data['abc::Ns']['isInner'] , False)
    expect_eq(data['abc::Ns::Inner']['isInner'] , True)
    expect_eq(data['abc::NsEnum']['isInner'] , False)
    expect_eq(data['NsInnerClass']['fields'][0]['types'][0]['name'] , 'abc::Ns::Inner')
    expect_eq(data['abc::Ns::InSt']['isInner'] , True)
    expect_eq(data['NsInnerStruct']['fields'][0]['types'][0]['name'] , 'abc::Ns::InSt')
    expect_true(any((field["name"] == "i" and field["types"][0]["name"] == "abc::Ns::Inner" for field in data["abc::Ns"]["fields"])))
    expect_eq(data['Outer']['fields'][0]['types'][0]['name'] , 'Outer::In')
    expect_eq(data['Outer2']['fields'][0]['types'][0]['name'] , 'Outer2::In2')
