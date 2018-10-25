import sys
import os
import time

##### Start from position= 2
pos = int(sys.argv[1])
print pos

start = time.time()
os.system("python cut_samples_split.py " + str(pos))
os.system("python cut_samples_random.py " + str(pos))
os.system("python check_correctness.py " + str(pos))
end = time.time()

diff = end - start
print diff
