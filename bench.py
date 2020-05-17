import os, subprocess, time

if os.name != 'nt':
	print('sorry, Windows only for now')
	os.exit(0)

SIZE = os.path.getsize('pg.txt')

def run1(args, prog, RUNS=10):
	t = []
	for i in range(RUNS):
		tm = time.perf_counter()
		r = subprocess.Popen(args + ['pg.txt', 'out.txt'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
		r.wait()
		t.append(time.perf_counter() - tm)
	t = sorted(t)
	print('| %.3f..%.3fs | %.1f | %s |' % (t[0], t[-1], SIZE / t[0] / 1000000, prog))

run1(['./bin/freq01cpp.exe'], 'freq01.cpp')
run1(['./bin/freq01rs.exe'], 'freq01.rs')
run1(['./bin/freq01go.exe'], 'freq01.go')
run1(['php', 'freq01.php'], 'freq01.php', RUNS=3)
