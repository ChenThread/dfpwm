# DFPWM (Dynamic Filter Pulse Width Modulation) codec - Reference Implementation
# by Ben "GreaseMonkey" Russell, 2012
# Public Domain

"""
Mathematical definition...

We are working with signed 8-bit mono sound streams.

Let q,s,Ri,Rd be signed integers.
Let Ri,Rd be chosen constants. ((7,20) is reasonable.)

Let q be the "charge", initialised to 0.
Let s be the "strength", initialised to 1.
Let Ri be the strength increase.
Let Rd be the strength decrease.

For every input sample v, we output bit b.

If v > q or q == v == 127, then set b to 1;
  otherwise set b to 0.

Let b' be the previous instance of b,
  initialised to 0 to simplify implementation.

Let t be the "target", -128 if b is 0, and 127 if b is 1.

Let q' be an integer such that
  q' <- q + (s*(t - q) + 128)/256

If q == q', and q != t, then:
  If t < q: q' <- q' - 1
  If t > q: q' <- q' + 1

Then set q <- q'.
  
Let r,z be integers such that:
  If b == b', then r = Ri, z = 255
  If b != b', then r = Rd, z = 0

Let s' be an integer such that
  s' <- s + (r*(z - s) + 128)/256

If s == s', and s != z, then:
  If z < s: s' <- s' - 1
  If z > s: s' <- s' + 1

Then set s <- s'.

Encoding information:
The 1-bit stream is packed little-endian: LSB first, MSB last.
...that's all you need to know really.

Filtering information:
Let f be the playback frequency.
You will get a ~f/2Hz whine unless you Filter That Crap Out.

I compared the codec for Queen's "Tie Your Mother Down" at 8000Hz,
and the main difference was that the original was less... pointy.

Tried at 44100Hz and the noise issue was clearer
-- it was definitely doing some up/down crap.

My recommendation is this:
	Track the last two samples.
	If you are going in same direction as the last sample,
		set the output sample to the current sample.
	Otherwise
		set the output sample to the average of
		the previous two samples.
	
Another thing you might like to try, which may be better:
	Track the last two samples.
	If you are going in same direction as the last sample,
		set the output sample to the current sample.
	Otherwise, if you are going towards the centre,
		set the output sample to the previous sample.
	Otherwise,
		set the output sample to the one before that.

After this, you may wish to add a low-pass filter in.

By all means, feel free to develop your own filter.

"""

import sys, struct

USE_LPF = True

def f(blri, blrd):
	infp = open(sys.argv[1],"rb")
	outfp = open(sys.argv[2],"wb")
	aoutfp = open(sys.argv[3],"wb")
	
	slq = 0
	sls = 100
	lq2 = 0
	lq1 = 0
	lq = 0
	ls = 1
	lri = blri
	lrd = blrd
	lt = -128
	mse = 0
	smpcount = 0
	while True:
		s = infp.read(8)
		if s == "":
			break
		while len(s) < 8:
			s += s[-1]
		
		d = 0x00
		
		for c in s:
			smpcount += 1
			d >>= 1
			
			v = (ord(c)^0x80)-0x80
			
			t = -128
			if v >= lq or v == -128:
				d |= 0x80
				t = 127
			
			nlq = lq + (ls*(t-lq)+128)//256
			
			if nlq == lq:
				if lq < t:
					lq += 1
				elif lq > t:
					lq -= 1
			
			lq = nlq
			err = lq - v
			mse += err*err
			
			# simple post-filter
			sv = lq
			if t != lt:
				sv = (lq2+lq1)//2
			
			if USE_LPF:
				slq = slq + (sls*(sv-slq)+128)//256
				aoutfp.write(chr(slq&255))
			else:
				aoutfp.write(chr(sv&255))
			
			lq2 = lq1
			lq1 = lq
			
			if t == lt:
				nls = ls + (lri*(255-ls)+128)//256
				if nls == ls and ls < 255:
					ls += 1
				ls = nls
			else:
				nls = ls + (lrd*(0-ls)+128)//256
				if nls == ls and ls > 0:
					ls -= 1
				ls = nls
			
			lt = t
			
			#print v, lq, ls
		
		outfp.write(chr(d))
	
	outfp.close()
	aoutfp.close()
	
	infp.close()
	
	mse = float(mse)/smpcount
	
	return mse

bestmse = 10000000.0

if False:
	for i in xrange(2,255):
		for j in xrange(1,i):
			blri, blrd = i,i-j
			mse = f(blri, blrd)
			if mse < bestmse:
				bestmse = mse
				print "Mean Square Error [%i, %i]: %.4f" % (blri, blrd, mse)
else:
	f(7, 20)
