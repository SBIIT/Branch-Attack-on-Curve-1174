import os
import sys
from random import randint

nob = int(sys.argv[1])
tlen = int(sys.argv[2])

secret_scalar = 2 ** (nob+1) + 2*randint(0,2 ** (nob))
fw = open("saved/secret","w")
fw.write("{0:b}".format(secret_scalar))
fw.close()


fp = open("saved/split","w") 
fw = open("saved/random","w")
for i in range(tlen):
	random = 2 ** nob + 2*randint(0,2**nob-1) + 1
	split_scalar = secret_scalar - random

	fw.write("{0:b}".format(random) + "\n")
	fp.write("{0:b}".format(split_scalar) + "\n")

fp.close()
fw.close()
	

	

	
