import os

SRC_FNAME = "mse_src.raw"
MAX_RI = 40
MAX_LEN = 32000*50
#srcdata = open("mse_src.raw", "rb").read(MAX_LEN)

best_mse = 0
best_ri = 0
best_rd = 0

# what's most likely true: ri > rd works, ri < rd doesn't
for ri in xrange(1,MAX_RI+1,1):
	print "round %d" % (ri, )
	for rd in xrange(1,ri+1,1):
		#print "round %d %d" % (ri, rd, )
		os.system("cc -Wall -Wextra -O2 -g -o cmp aucmp.c -DCONST_RI=%d -DCONST_RD=%d" % (ri, rd, ))
		os.system("cc -Wall -Wextra -O2 -g -o decmp audecmp.c -DCONST_RI=%d -DCONST_RD=%d" % (ri, rd, ))
		os.system("sh -c 'cat %s | ./cmp | ./decmp > tmpout.raw'" % (SRC_FNAME, ))

		mse = int(os.popen("./domse %s tmpout.raw" % (SRC_FNAME, )).read())

		if best_ri == 0 or best_mse > mse:
			best_mse = mse
			best_ri = ri
			best_rd = rd
			print "MSE %3d,%3d = %d" % (ri, rd, mse, )
