import math

STEREO = 1

#PREC_TABLE_LEN = 4096
PREC_TABLE_LEN = 2048
#PREC_TABLE_LEN = 1024
#PREC_TABLE_LEN = 512

PREC_TABLE_BOTTOM = 2
PREC_TABLE_MID_UP = int(PREC_TABLE_LEN//6)
PREC_TABLE_MID_UP_2 = int(PREC_TABLE_LEN//9)
PREC_TABLE_MID_DOWN = int(PREC_TABLE_LEN//4)

def curve(x):
	#return (x**0.5)*min(1.0,x*8.0)
	#return (x**2.0)
	return x
tab = [
	int(math.floor((curve(float(amt)/(PREC_TABLE_LEN-1)))*32767+1+0.5))
	for amt in xrange(PREC_TABLE_LEN)]

s = ""
s += "#define STEREO %d\n" % (STEREO, )
s += "#define PREC_TABLE_BOTTOM %d\n" % (PREC_TABLE_BOTTOM, )
s += "#define PREC_TABLE_MID_UP %d\n" % (PREC_TABLE_MID_UP, )
s += "#define PREC_TABLE_MID_UP_2 %d\n" % (PREC_TABLE_MID_UP_2, )
s += "#define PREC_TABLE_MID_DOWN %d\n" % (PREC_TABLE_MID_DOWN, )
s += "#define PREC_TABLE_LEN %d\n" % (PREC_TABLE_LEN, )
s += "uint16_t prec_table[PREC_TABLE_LEN] = {%s};" % (str(tab)[1:-1], )

fp = open("prectable.h", "wb")
fp.write(s)
fp.close()

print s

