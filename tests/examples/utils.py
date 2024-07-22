import inspect
import json
import os

ok = 0
ng = 0
total_ok = 0
total_ng = 0

def read_json(file_path):
    with open(file_path, 'r') as f:
        data = json.load(f)
    return data

def _get_top_script():
    stack = inspect.stack()
    top_level_frame = stack[-1]
    top_level_script = top_level_frame.filename
    return os.path.realpath(top_level_script)

def _basename(input):
    script_dir = os.path.dirname(_get_top_script())
    common_prefix = os.path.commonprefix([input, script_dir])
    relative_path = input.replace(common_prefix, '').lstrip('/')
    return relative_path

def _log(frame, expr):
    info = inspect.getframeinfo(frame)
    print(f"[\033[31mFailed\033[0m]({_basename(info.filename)}:{info.lineno})"
          f" {info.code_context[0].strip()},"
          f" {expr}")

def setup(test_file):
    global ok, ng
    print("Testing for " + test_file)
    ok = 0
    ng = 0

def teardown(flag = 0):
    global ok, ng, total_ok, total_ng
    if flag > 0:
        ng = ng + 1
    if ng > 0:
        print("OK: " + str(ok))
        print("NG: " + str(ng))
    total_ok = total_ok + ok
    total_ng = total_ng + ng

def end():
    global total_ok, total_ng
    print("")
    print("Total OK: " + str(total_ok))
    print("Total NG: " + str(total_ng))
    print("")

def expect_eq(a, b):
    global ok, ng
    if a == b:
        ok = ok + 1
    else:
        ng = ng + 1
        _log(inspect.currentframe().f_back, f"got '{a}' vs '{b}'")

def expect_ne(a, b):
    global ok, ng
    if a != b:
        ok = ok + 1
    else:
        ng = ng + 1
        _log(inspect.currentframe().f_back, f"got '{a}' vs '{b}'")

def expect_true(expr):
    global ok, ng
    if expr:
        ok = ok + 1
    else:
        ng = ng + 1
        _log(inspect.currentframe().f_back, f"got '{expr}'")

def expect_false(expr):
    global ok, ng
    if not expr:
        ok = ok + 1
    else:
        ng = ng + 1
        _log(inspect.currentframe().f_back, f"got '{expr}'")
