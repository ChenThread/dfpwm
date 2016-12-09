import math

STEREO = 1

#PREC_TABLE_LEN = 4096
PREC_TABLE_LEN = 2048
#PREC_TABLE_LEN = 1024
#PREC_TABLE_LEN = 512

PREC_TABLE_BOTTOM = 2

def curve(x):
	#return (x**0.5)*min(1.0,x*8.0)
	#return (x**2.0)
	return x
prec_table = [
	int(math.floor((curve(float(amt)/(PREC_TABLE_LEN-1)))*32767+1+0.5))
	for amt in xrange(PREC_TABLE_LEN)]

exc_table_low = [-3] + [-2]*5 + [-1]*3
#exc_table_low = [-1]
exc_table_high = [1]*3 + [2]*5 + [3]
exc_table = exc_table_low + exc_table_high
EXC_TABLE_MID = len(exc_table_low)
EXC_TABLE_LEN = len(exc_table)
s = ""
s += "#define STEREO %d\n" % (STEREO, )

s += "#define PREC_TABLE_BOTTOM %d\n" % (PREC_TABLE_BOTTOM, )
s += "#define PREC_TABLE_LEN %d\n" % (PREC_TABLE_LEN, )
s += "uint16_t prec_table[PREC_TABLE_LEN] = {\n%s\n};\n\n" % (str(prec_table)[1:-1], )

s += "#define EXC_TABLE_LEN %d\n" % (EXC_TABLE_LEN, )
s += "#define EXC_TABLE_MID %d\n" % (EXC_TABLE_MID, )
s += "int8_t exc_table[EXC_TABLE_LEN] = {\n%s\n};\n\n" % (str(exc_table)[1:-1], )

fp = open("prectable.h", "wb")
fp.write(s)
fp.close()

print s

