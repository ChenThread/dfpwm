/*
Wabbitoe - .wav to DFPWM converter
by Ben "GreaseMonkey" Russell, 2014 - Public Domain

Handles 8- and 16-bit wav files of any sane rate and channel count.
*/
import java.io.*;
import java.nio.*;
import javax.swing.*;
import javax.swing.filechooser.*;
import java.util.*;

public class Wabbitoe
{
	public static final int OUTFREQ = 32768;

	// Pretty sure Java provides something but I really cannot remember where and DDG isn't helping
	public static short e16(short v)
	{
		return (short)((0xFF & (int)(v >>> 8)) | (v << 8));
	}

	public static int e32(int v)
	{
		int h = (v>>>16);
		int l = v & 0xFFFF;
		h = 0xFFFF & e16((short)h);
		l = 0xFFFF & e16((short)l);

		return (l<<16) | h;
	}

	public static void spewError(String err)
	{
		JOptionPane.showMessageDialog(null, err, "Error", JOptionPane.ERROR_MESSAGE);
		throw new RuntimeException(err);
	}

	public static void main(String[] args) throws Exception
	{
		JFileChooser jfc = null;
		File infile = null;
		File outfile = null;

		// Choose the input file
		jfc = new JFileChooser(new File("."));
		jfc.setDialogTitle("Choose a file to load");
		jfc.setFileFilter(new FileNameExtensionFilter(
			"WAV audio", "wav"));
		if(jfc.showOpenDialog(null) != JFileChooser.APPROVE_OPTION)
			return;
		infile = jfc.getSelectedFile();

		// Choose the output file
		jfc = new JFileChooser(infile);
		jfc.setDialogTitle("Now choose what you're going to save it as");
		jfc.setFileFilter(new FileNameExtensionFilter(
			"DFPWM audio", "dfpwm"));
		if(jfc.showSaveDialog(null) != JFileChooser.APPROVE_OPTION)
			return;
		outfile = jfc.getSelectedFile();

		// Open the first file
		DataInputStream fp = new DataInputStream(new FileInputStream(infile));
		byte[] b = new byte[20];

		// Check some things first
		fp.readFully(b, 0, 8);
		if(b[0] != 'R' || b[1] != 'I' || b[2] != 'F' || b[3] != 'F')
			spewError("not a RIFF WAVE file");
		// ignore length - we're not here to validate these files

		fp.readFully(b, 0, 4);
		if(b[0] != 'W' || b[1] != 'A' || b[2] != 'V' || b[3] != 'E')
			spewError("not a RIFF WAVE file");

		// Read tags...
		boolean has_fmt = false;
		byte[] inbuf = null;
		int chns = -1;
		int freq = -1;
		int bps = -1;

		while(true)
		{
			try
			{
				fp.readFully(b, 0, 4);
			} catch(EOFException _) {
				// All good! (I think!)
				break;
			}

			int cnklen = e32(fp.readInt());
			if(cnklen < 0)
				spewError("Chunk size too large to be coherent");

			try
			{
				System.out.printf("tag %c%c%c%c (%d)\n", b[0], b[1], b[2], b[3], cnklen);
			} catch(IllegalFormatCodePointException ex){
				System.out.printf("<eek>\n");
			}

			// Prepare input stream
			byte[] sb = new byte[cnklen];
			fp.readFully(sb);
			DataInputStream sfp = new DataInputStream(new ByteArrayInputStream(sb));

			if(b[0] == 'f' && b[1] == 'm' && b[2] == 't' && b[3] == ' ')
			{
				if(e16(sfp.readShort()) != 1)
					spewError("Not a raw PCM WAVE file");

				chns = e16(sfp.readShort());
				freq = e32(sfp.readInt());
				e32(sfp.readInt());
				e16(sfp.readShort());
				bps = e16(sfp.readShort());

				has_fmt = true;
			} else if(b[0] == 'd' && b[1] == 'a' && b[2] == 't' && b[3] == 'a') {
				inbuf = sb;
			} else {
				// other tags are ignored
			}
		}

		if((!has_fmt) || inbuf == null)
			spewError("Missing fmt and/or data tag");

		System.out.printf("Format: %dHz %d-bit, %d channels\n", freq, bps, chns);

		fp.close();

		// Check our parameters
		if(bps != 8 && bps != 16) spewError("Only 8-bit and 16-bit samples are supported");
		if(chns < 1) spewError("Invalid channel count");
		if(freq < 1) spewError("Invalid frequency");

		// Prepare for translation and stuff
		int units = 1;
		if(bps == 16) units *= 2;
		units *= chns;

		int inbuflen = inbuf.length;

		float fgrad = ((float)freq) / ((float)OUTFREQ);
		int midbuflen = (int)(((int)(inbuflen / units)) / fgrad);
		while(midbuflen % 8 != 0) midbuflen++;

		byte[] midbuf = new byte[midbuflen];
		byte[] outbuf = new byte[midbuflen/8];

		// Translate from one sample format to another
		System.out.printf("Translating!\n");
		for(int i = 0; i < midbuflen; i++)
		{
			int si = units * (int)(i*fgrad);

			if(si >= inbuflen)
				si = inbuflen - units;

			int v = 0;

			for(int j = 0; j < chns; j++)
			{
				if(bps == 16)
				{
					int l = 0xFF&(int)inbuf[si++];
					int h = 0xFF&(int)inbuf[si++];
					int s = l | (h<<8);
					s &= 0xFFFF;
					s = (s >= 0x8000 ? s - 0x10000 : s);
					v += s;
				} else {
					int s = inbuf[si++];
					s &= 0xFF;
					s <<= 8;
					v += s;
				}
			}
			
			v = (v*2+chns)/(chns*2);
			midbuf[i] = (byte)(v/256);
		}

		// Compress
		new DFPWM().compress(outbuf, midbuf, 0, 0, midbuflen/8);

		// Write new file
		DataOutputStream ofp = new DataOutputStream(new FileOutputStream(outfile));
		ofp.write(outbuf);
		ofp.close();
	}
}

