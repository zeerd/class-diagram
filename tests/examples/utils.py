import json
import os
import inspect

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

def expect_eq(a, b):
    if a != b:
        _log(inspect.currentframe().f_back, f"got '{a}' vs '{b}'")

def expect_ne(a, b):
    if a == b:
        _log(inspect.currentframe().f_back, f"got '{a}' vs '{b}'")

def expect_true(expr):
    if not expr:
        _log(inspect.currentframe().f_back, f"got '{expr}'")

def expect_false(expr):
    if expr:
        _log(inspect.currentframe().f_back, f"got '{expr}'")
