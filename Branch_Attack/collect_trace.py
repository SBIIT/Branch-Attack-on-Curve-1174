import os
import random
from random import randint

os.system("gcc measure_edwards.c -o edw");
fp = open("saved/split","r")
lines = fp.readlines()
fp.close()

binary = []
for line in lines:
	binary.append(str(line[:len(line)-1]))
#print binary

for lines in binary:
	bits = lines
	hexstr= str(hex(int(bits,2)))
	buf = ""
	hex_mask = hexstr[2:]
	line_len = len(hex_mask) -1
	while(line_len >= 0):
		if (line_len == 2):
			buf = buf + " " + hex_mask[line_len - 2] + hex_mask[line_len -1] + hex_mask[line_len]
		elif (line_len == 1):
			buf = buf + " " + hex_mask[line_len -1] + hex_mask[line_len]
		elif (line_len == 0):
			buf = buf + " " + hex_mask[line_len]
		else:
			buf = buf + " " + hex_mask[line_len-3] + hex_mask[line_len - 2] + hex_mask[line_len -1] + hex_mask[line_len]
		line_len = line_len -4
	buf1 = "./edw " + buf + "\n"
	os.system(buf1)
os.system("mv to_match/unknown to_match/traces_split")

fp = open("saved/random","r")
lines = fp.readlines()
fp.close()

binary = []
for line in lines:
	binary.append(str(line[:len(line)-1]))
#print binary

for lines in binary:
	bits = lines
	hexstr= str(hex(int(bits,2)))
	buf = ""
	hex_mask = hexstr[2:]
	line_len = len(hex_mask) -1
	while(line_len >= 0):
		if (line_len == 2):
			buf = buf + " " + hex_mask[line_len - 2] + hex_mask[line_len -1] + hex_mask[line_len]
		elif (line_len == 1):
			buf = buf + " " + hex_mask[line_len -1] + hex_mask[line_len]
		elif (line_len == 0):
			buf = buf + " " + hex_mask[line_len]
		else:
			buf = buf + " " + hex_mask[line_len-3] + hex_mask[line_len - 2] + hex_mask[line_len -1] + hex_mask[line_len]
		line_len = line_len -4
	buf1 = "./edw " + buf + "\n"
	os.system(buf1)
os.system("mv to_match/unknown to_match/traces_random")


