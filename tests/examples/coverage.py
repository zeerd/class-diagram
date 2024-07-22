import os
import subprocess

from pathlib import Path

infos = []

def cov_do(test_file):
    global infos
    with open(test_file+'.cov.log', 'w') as f:
        subprocess.run(['lcov', '-c', '-d', '..', '-o', test_file+'.info'],
                    stdout=f, stderr=f)
        infos.extend(['-a', test_file+'.info'])

def cov_end():
    global infos
    with open('class-diagram.cov.log', 'w') as f:
        cmd = ['lcov', '-d', '..', '-o', 'class-diagram.info']
        cmd.extend(infos)
        subprocess.run(cmd, stdout=f, stderr=f)
        subprocess.run(['lcov', '-e', 'class-diagram.info',
                        os.path.realpath('../src')+'/*',
                        '-o', 'class-diagram.info.cleaned'],
                        stdout=f, stderr=f)
        subprocess.run(['genhtml', 'class-diagram.info.cleaned',
                        '-o', 'coverage'], stdout=f, stderr=f)

