#!/usr/bin/env python3
import os, subprocess, time, sys

SIZE = os.path.getsize('pg.txt')
DEF_RUNS = 10
EXE = '.exe' if os.name == 'nt' else ''

def run1(args, src_name, num_runs):
	t = []
	for i in range(num_runs):
		tm = time.perf_counter()
		r = subprocess.Popen(args + ['pg.txt', 'out.txt'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
		r.wait()
		t.append(time.perf_counter() - tm)
	t = sorted(t)
	print('| %.3f..%.3fs | %.1f | %s |' % (t[0], t[-1], SIZE / t[0] / 1000000, src_name))

runs = [
	[['java', '-jar', './bin/freq01scala.jar'], 'freq01.scala', 3],
	[['python', './src/freq01.py'], 'freq01.py', 3],
	[['node', './src/freq01.js'], 'freq01.js', 3],
	[['./bin/freq03cpp' + EXE], 'freq03.cpp'],
	[['./bin/freq02cpp' + EXE], 'freq02.cpp'],
	[['./bin/freq01cpp' + EXE], 'freq01.cpp'],
	[['./bin/freq01rs' + EXE], 'freq01.rs'],
	[['./bin/freq01go' + EXE], 'freq01.go'],
	[['php', './src/freq01.php'], 'freq01.php', 3],

	[['./bin/hack01cpp' + EXE], 'hack01.cpp']]

for run in runs:
	ok = (sys.argv[1] == run[1]) if len(sys.argv) == 2 else True
	if ok:
		run1(run[0], run[1], run[2] if len(run) > 2 else DEF_RUNS)
