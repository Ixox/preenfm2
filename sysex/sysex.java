/*
 * Copyright 2013 Xavier Hosxe
 *
 * Author: Xavier Hosxe
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Sysex creation from bin
 * http://www.preenfm.net
 */

import java.io.BufferedInputStream;
import java.io.BufferedWriter;
import java.io.ByteArrayInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.Writer;



public class sysex {

	public static void main(String[] args) {
		InputStream input = null;
		try {
			if (args.length !=1) {
				printHelp();
				return;
			}

			System.out.println("Input file : "+ args[0]);

			File file = new File(args[0]);
			int length = (int) file.length();
			System.out.println("Input file length : "+ length);

			input = new BufferedInputStream(new FileInputStream(file));
			
			byte[] bin = new byte[length+4];

			for (int k=length; k < length + 4; k++) {
				bin[k] = 0;
			}

			int totalByteRead = input.read(bin, 0, length);

			System.out.println("Bytes read : "+ length);

			int mod = totalByteRead % 4;
			int length4 = totalByteRead / 4;
			System.out.println("mod : "+ mod);
			if (mod > 0) {
				length4 += 1;
			}

			System.out.println("Size of file in integer: " + length4);

			String syxName = args[0].replace(".bin", ".syx");
			File writeFile = new File(syxName);

			// if file does'nt exists, then create it
			if (!writeFile.exists()) {
				writeFile.createNewFile();
			}
			FileOutputStream fos = new FileOutputStream(writeFile);
			DataOutputStream dos = new DataOutputStream(fos);

			System.out.println("Output file : "+ syxName);

			dos.writeByte( 0xf0);
			dos.writeByte( 0x7d);
			dos.writeByte( 100);
			// Write length
			writeInt(dos, length4);
			long check = 0;
			for (int k=0; k< length4; k++) {
				long toWrite = (long)bin[k*4+3] & 0xff;
				toWrite <<= 8;
				toWrite |= (long)bin[k*4+2] & 0xff;
				toWrite <<= 8;
				toWrite |= (long)bin[k*4+1] & 0xff; 
				toWrite <<= 8;
				toWrite |= (long)bin[k*4] & 0xff;
				writeInt(dos, toWrite);					

				if (k>0 && (k % 1000) == 0) {
					writeInt(dos, k);
				}
				check += toWrite;
			}
			check &= 0xffffffff;
			writeInt(dos, (int)check);			
			
			dos.writeByte( 0xf7);

			dos.close();

			

		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} finally {
			try {
				if (input != null) {
					input.close();
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
	
	static void writeInt(DataOutputStream dos, long toWrite) throws IOException {
		byte[] sysex = new byte[5];
		for (int j=0; j<5; j++) {
			sysex[4-j] = (byte)(toWrite & 0x7f);
			toWrite >>= 7;
		}
		dos.write(sysex, 0, 5);		
	}


	static void printHelp() {
		System.out.println("##");
		System.out.println("## Must have one argument : bin file...");
		System.out.println("##");
	}

}
