import sys
import os
from collections import Counter
import numpy
import math

#Given the start and end point the following list is the displacement values of the 
#individual operation from their start location. This is machine dependent
op_start = [9,26,43,60,77,95,112,114,131,148,164,182,184,202,218,239,256,273,290,307,325,342,344,361,378,394,412,414,432,448]
op_dur =[17,17,17,17,17,17,2,17,17,16,17,2,17,16,17,17,17,17,17,17,17,2,17,17,16,17,2,17,16,17]



os.system("rm -rf for_samples/for_every_0/*")
os.system("rm -rf for_samples/for_every_1/*")
os.system("rm -rf reconstructed/sample")
os.system("rm -rf for_samples/traces_0/output")
os.system("rm -rf for_samples/traces_1/output")
os.system("rm -rf reconstructed/output")
os.system("rm -rf for_samples/temp_0/*")
os.system("rm -rf for_samples/temp_1/*")
os.system("gcc edward_for_0.c -o edw_0");
os.system("gcc edward_for_1.c -o edw_1");
os.system("gcc edward_known.c -o edw_k");
os.system("rm -rf for_samples/bmsim_0/*")
os.system("rm -rf for_samples/bmsim_1/*")
os.system("gcc bm_sim_0.c -o bm_0");
os.system("gcc bm_sim_1.c -o bm_1");


#Find the prediction
def predictor(state,line,val):
	total_miss = 0
	#initialize prediction
	for j in line:
		if (j == '1'):
			br = 1
		if (j == '0'):
			br = 0
		if (int(val) >= 0):
			if (state < 4):
				pred = 0
			if (state > 3):
				pred = 1
			if (state > 0 and state <7):
				if(br == 1):
					state = state + 1
				else: 
					state = state - 1
			if (state == 0):
				if(br == 1):
					state = 1
			if (state == 7):
				if(br == 0):
					state = 6
			miss = 0
			if ((br == 1) and (pred == 0)):
				miss = 1
				total_miss = total_miss + miss
			if ((br == 0) and (pred == 1)):
				miss = 1
				total_miss = total_miss + miss

	if(int(val) == 0):
		return state
	else:
		return state,total_miss

	#print miss


#Enter the position from LSB
#pos = raw_input("Please enter the position you want to target??\n")

pos = int(sys.argv[1])
print pos

#Read the binary split scalars
fp = open("saved/random","r")
full_scalar = fp.readlines()
fp.close()

#Copy the known bytes till position - 1
fw = open("reconstructed/known_random","w")
for lines in full_scalar:
	prevbits=""
	for i in range(int(pos)-1):
		bit = lines[len(lines) - 2 - i]
		#bits.append(lines[len(lines)-2-int(pos)])
		prevbits = str(bit) + prevbits
	fw.write(prevbits + "\n")
fw.close()

fp = open("reconstructed/known_random","r")
known_bits = fp.readlines()
fp.close()

binary = []
for lines in known_bits:
	binary.append("1" + str(lines[:len(lines)-1]))# Just to preserve the size if there are leading zeroes
#print binary


#############################################################################################
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

	#Simulate Branch mispredictions for two guess of the target position 
	buf1 = "./bm_0 " + buf + "\n"
	buf2 = "./bm_1 " + buf + "\n"
	#print buf1, buf2	
	os.system(str(buf1))
	os.system(str(buf2))


	#Obtain perf misprediction samples for the known part of the scalar with guesses 0 and 1
	buf1 = "./edw_0 " + buf + "\n"
	buf2 = "./edw_1 " + buf + "\n"
	buf3 = "./edw_k " + buf + "\n" 
	#print buf
	
	os.system(str(buf1))
	os.system(str(buf2))
	os.system(str(buf3))

	os.system("cp for_samples/for_every_0/sample for_samples/for_every_0/bit_"+ str(pos))
	os.system("cp for_samples/for_every_1/sample for_samples/for_every_1/bit_"+ str(pos))
	os.system("cp reconstructed/sample reconstructed/bit_"+ str(pos))
######################################################################################################




#Building Templates for Guess 0
######################################################################################################
#Calculating the states of the branch predictor for guess 0 bit and the total number of misses
fp = open("for_samples/bmsim_0/bm_save", "r")
bm_trace = fp.readlines()
fp.close()

trace0_miss= [[] for i in xrange(30)]
avg_miss0 = []
fname = "for_samples/bmsim_0/op_"
lno =0
while(lno < (len(bm_trace)/30)):
	state = 0
	for op in range(30):
		values = bm_trace[lno*30 + op]
		state,miss = predictor(state,values,1)
		fp = open(fname + str(op), "a")
		fp.write(str(miss)+ "\n")
		fp.close()
		trace0_miss[op].append(miss)
	lno = lno +1
for lno in range(30):
	avg_miss0.append(numpy.mean(trace0_miss[lno]))
print avg_miss0

#Calculating the states of the branch predictor for guess 1 bit and the total number of misses
fp = open("for_samples/bmsim_1/bm_save", "r")
bm_trace = fp.readlines()
fp.close()

trace1_miss = [[] for i in xrange(30)]
avg_miss1 = []
fname = "for_samples/bmsim_1/op_"
lno =0
while(lno < (len(bm_trace)/30)):
	state = 0
	for op in range(30):
		values = bm_trace[lno*30 + op]
		state,miss = predictor(state,values,1)
		fp = open(fname + str(op), "a")
		fp.write(str(miss)+ "\n")
		fp.close()
		trace1_miss[op].append(miss)
	lno = lno +1
for lno in range(30):
	avg_miss1.append(numpy.mean(trace1_miss[lno]))
print avg_miss1
######################################################################################################

#Selecting samples based on the position which we are targeting
#Guess 0
fp = open("reconstructed/bit_"+ str(int(pos)),"r")
start_0 = map(int,fp.readlines())
fp.close()
fp = open("for_samples/for_every_0/bit_"+ str(pos),"r")
end_0 = map(int,fp.readlines())
fp.close()
#print pos,start_0,end_0

op_start0 = [[] for i in xrange(30)]
op_end0 = [[] for i in xrange(30)]
for op in range(30):
	for lno in range(len(start_0)):
		op_start0[op].append(int(start_0[lno] + op_start[op]))
		op_end0[op].append(int(start_0[lno] + op_start[op] + op_dur[op]))




#Guess 1
fp = open("reconstructed/bit_"+ str(int(pos)),"r")
start_1 = map(int,fp.readlines())
fp.close()
fp = open("for_samples/for_every_1/bit_"+ str(pos),"r")
end_1 = map(int,fp.readlines())
fp.close()

op_start1 = [[] for i in xrange(30)]
op_end1 = [[] for i in xrange(30)]
for op in range(30):
	for lno in range(len(start_1)):
		op_start1[op].append(start_1[lno] + op_start[op])
		op_end1[op].append(start_1[lno] + op_start[op] + op_dur[op])


#print pos,start_1,end_1
######################################################################################################
#Reading traces in order to build templates, we have op_start0[30][len], op_end0[30][len] and likewise


#Guess 0
fp = open("for_samples/traces_0/output","r")
traces_0 = fp.readlines()
fp.close()


bmiss_0 = [[] for i in xrange(30)]
fname0 = "for_samples/temp_0/less_op_"
fname1 = "for_samples/temp_0/more_op_"
lno = 0
for lines in traces_0:
	ints_list = map(int, lines.strip().split(' '))
	for cn in range(30):
		sample = ints_list[op_start0[cn][lno]:op_end0[cn][lno]]
		#print str(end_0[lno] - start_0[lno])
		if(trace0_miss[cn][lno] < avg_miss0[cn]):
			fw0 = open(fname0 + str(cn),"a")
			fw0.write(' '.join(map(str, sample)))
			fw0.write("\n")
			fw0.close()
			bmiss_0[cn].append(int(0))
		elif[trace0_miss[cn][lno] >= avg_miss0[cn]]:
			fw1 = open(fname1 + str(cn),"a")
			fw1.write(' '.join(map(str, sample)))
			fw1.write("\n")
			fw1.close()
			bmiss_0[cn].append(int(1))
	lno = lno + 1
#Guess 1
fp = open("for_samples/traces_1/output","r")
traces_1 = fp.readlines()
fp.close()

bmiss_1 = [[] for i in xrange(30)]
fname0 = "for_samples/temp_1/less_op_"
fname1 = "for_samples/temp_1/more_op_"
lno = 0
for lines in traces_1:
	ints_list = map(int, lines.strip().split(' '))
	for cn in range(30):
		sample = ints_list[op_start1[cn][lno]:op_end1[cn][lno]]
		#print str(end_1[lno] - start_1[lno])
		if(trace1_miss[cn][lno] < avg_miss1[cn]):
			fw0 = open(fname0 + str(cn),"a")
			fw0.write(' '.join(map(str, sample)))
			fw0.write("\n")
			fw0.close()
			bmiss_1[cn].append(int(0))
		elif[trace1_miss[cn][lno] >= avg_miss1[cn]]:
			fw1 = open(fname1 + str(cn),"a")
			fw1.write(' '.join(map(str, sample)))
			fw1.write("\n")
			fw1.close()
			bmiss_1[cn].append(int(1))
	lno = lno + 1
fw0.close()
fw1.close()


##############################################################################################
#Now constructing the templates for each guess
for fn in range(0,2):
	for op in range(30):
		fname1 = "for_samples/temp_" + str(fn) +"/less_op_" + str(op)
		fname2 = "for_samples/temp_" + str(fn) +"/less_tm_" + str(op)
		if(os.path.isfile(fname1)):
			fw = open(fname2,"w")
			fp = open(fname1,"r")
			Matrix = []
			for lines in fp:
				ints_list = map(int, lines.strip().split(' '))
				Matrix.append(ints_list)
				#print len(ints_list)
			fp.close()
			max_len = 0
			for ln in range(len(Matrix)):
				if (max_len < len(Matrix[ln])):
					max_len = len(Matrix[ln])
			#print max_len
			cn = 0
			for cn in range(max_len):
				newlist = []
				for ln in range(len(Matrix)):
					if (cn < len(Matrix[ln])):
						newlist.append(Matrix[ln][cn])
				fw.write(str(int(numpy.mean(newlist))) + " ")
			fw.close()

		fname1 = "for_samples/temp_" + str(fn) +"/more_op_" + str(op)
		fname2 = "for_samples/temp_" + str(fn) +"/more_tm_" + str(op)
		if(os.path.isfile(fname1)):
			fw = open(fname2,"w")
			fp = open(fname1,"r")		
			Matrix = []
			for lines in fp:
				ints_list = map(int, lines.strip().split(' '))
				Matrix.append(ints_list)
				#print len(ints_list)
			fp.close()
			max_len = 0
			for ln in range(len(Matrix)):
				if (max_len < len(Matrix[ln])):
					max_len = len(Matrix[ln])
			#print max_len

			cn = 0
			for cn in range(max_len):
				newlist = []
				for ln in range(len(Matrix)):
					if (cn < len(Matrix[ln])):
						newlist.append(Matrix[ln][cn])
				fw.write(str(int(numpy.mean(newlist))) + " ")
			fw.close()



##############################################################################################
#Reading the saved templates for both the guesses
g0_less = [[] for i in xrange(30)]
g0_more = [[] for i in xrange(30)]
g1_less = [[] for i in xrange(30)]
g1_more = [[] for i in xrange(30)]
for lno in range(30):
	fname1 = "for_samples/temp_0/less_op_" + str(lno)
	if(os.path.isfile(fname1)):
		fp = open(fname1,"r")
		template = fp.readlines()
		fp.close()
		for lines in template:
			g0_less[lno] = map(int, lines.strip().split(' '))


	fname1 = "for_samples/temp_0/more_op_" + str(lno)
	if(os.path.isfile(fname1)):
		fp = open(fname1,"r")
		template = fp.readlines()
		fp.close()
		for lines in template:
			g0_more[lno] = map(int, lines.strip().split(' '))

	fname1 = "for_samples/temp_1/less_op_" + str(lno)
	if(os.path.isfile(fname1)):
		fp = open(fname1,"r")
		template = fp.readlines()
		fp.close()
		for lines in template:
			g1_less[lno] = map(int, lines.strip().split(' '))

	fname1 = "for_samples/temp_1/more_op_" + str(lno)
	if(os.path.isfile(fname1)):
		fp = open(fname1,"r")
		template = fp.readlines()
		fp.close()
		for lines in template:
			g1_more[lno] = map(int, lines.strip().split(' '))
##################################################################################################


#Templates are constructed successfully
#Now go to the actual trace files and collect the traces which needs to be matched
#We already have the start and the end pointers for a particular guess

fp = open("to_match/traces_random/unknown","r")
unknown_traces = fp.readlines()
fp.close()

fw = open("reconstructed/known_random","w")

#Partial traces to match if guess 0 and 1
lno = 0
for lines in unknown_traces:
	vote_0 = 0
	vote_1 = 0
	for op in range(30):
		ints_list = map(int, lines.strip().split(' '))
		sample = ints_list[op_start0[op][lno]:op_end0[op][lno]]
		if(bmiss_0[op][lno] == 0):
			len1 = len(g0_less[op])
			len2 = len(sample)
			a = g0_less[op][:min(len1,len2)]
			b = sample[:min(len1,len2)]		
			#distance_0 = math.sqrt(sum( (a - b)**2 for a, b in zip(a, b)))
		elif(bmiss_0[op][lno] == 1):		
			len1 = len(g0_more[op])
			len2 = len(sample)
			a = g0_more[op][:min(len1,len2)]
			b = sample[:min(len1,len2)]		
		distance_0 = math.sqrt(sum( (a - b)**2 for a, b in zip(a, b)))

		sample = ints_list[op_start1[op][lno]:op_end1[op][lno]]
		if(bmiss_1[op][lno] == 0):
			len1 = len(g1_less[op])
			len2 = len(sample)
			a = g1_less[op][:min(len1,len2)]
			b = sample[:min(len1,len2)]		
			#distance_1 = math.sqrt(sum( (a - b)**2 for a, b in zip(a, b)))
		elif(bmiss_1[op][lno] == 1):		
			len1 = len(g1_more[op])
			len2 = len(sample)
			a = g1_more[op][:min(len1,len2)]
			b = sample[:min(len1,len2)]		
		distance_1 = math.sqrt(sum( (a - b)**2 for a, b in zip(a, b)))
		if (distance_0 < distance_1):
			vote_0 = vote_0 +1
		else: 
			vote_1 = vote_1 +1
	if(vote_0 > vote_1):
		found = 0
	else:
		found = 1 

	buf = str(found) + known_bits[lno]
	fw.write(buf)
	lno = lno +1
	print vote_0,vote_1
fw.close()
