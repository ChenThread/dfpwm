bunch of DFPWM implementations

yes

RECOMMENDATION: Unless you care about backwards compatibility, use ri=1, rd=1, LPF strength=140, precision=10 - the original coefficients suck.

* aucmp.py: (signed 8-bit) The original. Has a description in a comment!
* aucmp.c, audecmp.c: (signed 8-bit) The first implementation done in a language that was actually meant to be fast.
* DFPWM.java: (unsigned 8-bit) The implementation you've all learnt to love. Or hate.
* lutdecmp.c: (signed 8-bit) A rather hissy approximate decoder. This one's faster and can be done in parallel, though! (But if audecmp.c is fast enough, you can just parallel overlap and crossfade and get better results.)

and of course:

* Wabbitoe.java: Source code for that tool I made that not even I use, because sox is great and this thing makes ffmpeg's resampler look good.

plus:

* domse.c, msecalc.py: Tools for calculating mean square error. Not recommended. I used something similar like this to get the coefficients of 7,20 and it turns out they kinda suck, at least for 32000Hz.


