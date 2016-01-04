bunch of DFPWM implementations

yes

* aucmp.py: (signed 8-bit) The original. Has a description in a comment!
* aucmp.c, audecmp.c: (signed 8-bit) The first implementation done in a language that was actually meant to be fast.
* DFPWM.java: (unsigned 8-bit) The implementation you've all learnt to love. Or hate.
* lutdecmp.c: (signed 8-bit) A rather hissy approximate decoder. This one's faster and can be done in parallel, though! (But if audecmp.c is fast enough, you can just parallel overlap and crossfade and get better results.)

and of course:

* Wabbitoe.java: Source code for that tool I made that not even I use, because sox is great and this thing makes ffmpeg's resampler look good.



