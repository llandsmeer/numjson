import pytest
import numjson

def test_int():
    assert numjson.parse("1") == 1
    assert isinstance(numjson.parse("10"), int)

def test_float():
    assert numjson.parse("10.0") == 10
    assert numjson.parse("10e-2") == .1
    assert numjson.parse("33.3") == 33.3

def test_true():
    assert numjson.parse("true") == True
    with pytest.raises(ValueError):
        numjson.parse("t");
    with pytest.raises(ValueError):
        numjson.parse("tf");
    with pytest.raises(ValueError):
        numjson.parse("taaaaaa");

def test_false():
    assert numjson.parse("false") == False
    with pytest.raises(ValueError):
        numjson.parse("f");
    with pytest.raises(ValueError):
        numjson.parse("ft");
    with pytest.raises(ValueError):
        numjson.parse("faaaaaa");

def test_obj():
    assert numjson.parse("{}") == {}
    assert numjson.parse('{"a":10}') == {"a": 10}
    assert numjson.parse('{"a":10, "b" : 20 } ') == {"a": 10,"b": 20}

def test_array():
    assert numjson.parse("[]") == []
    assert numjson.parse("[1]") == [1]
    assert numjson.parse('[1,{"a":1.0}]') == [1,{"a":1.0}]

def test_string():
    assert numjson.parse('""') == b''
    assert numjson.parse('"a"') == b'a'
    assert numjson.parse('"a\na"') == b'a\na'
