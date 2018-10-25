import sys 
import os

pos = int(sys.argv[1])
print pos

#Read the binary split scalars
fp = open("reconstructed/known_split","r")
split_scalar = fp.readlines()
fp.close()

fp = open("reconstructed/known_random","r")
random = fp.readlines()
fp.close()

split_list = []
#Copy the known bytes till position - 1
for lines in split_scalar:
	prevbits=""
	for i in range(int(pos)-1):
		bit = lines[len(lines) - 2 - i]
		#bits.append(lines[len(lines)-2-int(pos)])
		prevbits = str(bit) + prevbits
	#print prevbits

	split_list.append(int(prevbits,2))


rand_list = []
#Copy the known bytes till position - 1
for lines in random:
	prevbits=""
	for i in range(int(pos)-1):
		bit = lines[len(lines) - 2 - i]
		#bits.append(lines[len(lines)-2-int(pos)])
		prevbits = str(bit) + prevbits
	#print prevbits
	rand_list.append(int(prevbits,2))

vote_0 = 0
vote_1 = 0
for i in range(len(split_list)):
	value = split_list[i] + rand_list[i]
	if(int((value>>(pos-2))&1)):
		vote_1 = vote_1 + 1
	else:
		vote_0 = vote_0 + 1
if(vote_0 > vote_1):
	print "====> 0 is the Secret Scalar bit"
else:
	print "====> 1 is the Secret Scalar bit"


	


