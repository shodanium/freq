import os, subprocess, time, sys

if os.name != 'nt':
	print('sorry, Windows only for now')
	sys.exit(0)

SIZE = os.path.getsize('pg.txt')
DEF_RUNS = 10

def run1(args, src_name, RUNS=10):
	t = []
	for i in range(RUNS):
		tm = time.perf_counter()
		r = subprocess.Popen(args + ['pg.txt', 'out.txt'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
		r.wait()
		t.append(time.perf_counter() - tm)
	t = sorted(t)
	print('| %.3f..%.3fs | %.1f | %s |' % (t[0], t[-1], SIZE / t[0] / 1000000, src_name))

runs = [
	[['./bin/freq02cpp.exe'], 'freq02.cpp'],
	[['./bin/hack01cpp.exe'], 'hack01.cpp'],
	[['./bin/freq01cpp.exe'], 'freq01.cpp'],
	[['./bin/freq01rs.exe'], 'freq01.rs'],
	[['./bin/freq01go.exe'], 'freq01.go'],
	[['php', './src/freq01.php'], 'freq01.php', 3]]

for run in runs:
	ok = (sys.argv[1] == run[1]) if len(sys.argv) == 2 else True
	if ok:
		run1(run[0], run[1], run[2] if 2 in run else DEF_RUNS)
