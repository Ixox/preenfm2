/*
 * Copyright 2012 Xavier Hosxe
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
 * Oscillator waveform genration for PreenFM synth
 * http://www.preenfm.net
 */

import java.util.*;
import java.text.DecimalFormat;

class waveforms {
enum WaveForm {
        SQUARE,
        SAWTOOTH,
        SIN,
        SINSQUARE,
        SINORZERO,
        SINPOS
};

public static void main(String[] args) {
        int numberOfSteps = 2048;
        int order = 20;
        df = new DecimalFormat("#.############");
        if (args.length>0) {
                try {
                        numberOfSteps = Integer.parseInt(args[0]);
                } catch (java.lang.NumberFormatException e) {
                        System.out.println(args[0] + " is not a number");
                        printHelp();
                        return;
                }
        }

        if (args.length>1) {
                try {
                        order = Integer.parseInt(args[1]);
                } catch (java.lang.NumberFormatException e) {
                        System.out.println(args[1] + " is not a number");
                        printHelp();
                        return;
                }
        }


        int a = 440; // a is 440 hz...
        System.out.println("float frequency[] __attribute__ ((section(\".ccm\"))) = {");
        for (int x = 0; x < 127; ++x)
        {
                String freqString = df.format((double)(a / 32.0d) * (Math.pow(2.0d, ((x - 9.0d) / 12.0d))));
                System.out.print(freqString);
                for (int s = 0; s<(20-freqString.length()); s++) {
                        System.out.print(" ");
                }

                if (x < 127 -1) {
                        System.out.print(", ");
                }
                if ((x % 5)==4) {
                        System.out.println();
                }

        }
        System.out.println("};");
        System.out.println("");

        System.out.println("float sinTable[] __attribute__ ((section(\".ccm\"))) = {");
        System.out.println(getWaveFormData(WaveForm.SIN, numberOfSteps, order));
        System.out.println("};");
        System.out.println("");
        System.out.println("float squareTable[] __attribute__ ((section(\".ccm\"))) = {");
        System.out.println(getWaveFormData(WaveForm.SQUARE, numberOfSteps, order));
        System.out.println("};");
        System.out.println("");
        System.out.println("float sawTable[] __attribute__ ((section(\".ccm\"))) = {");
        System.out.println(getWaveFormData(WaveForm.SAWTOOTH, numberOfSteps, order));
        System.out.println("};");
        System.out.println("");
        System.out.println("float sinPosTable[] __attribute__ ((section(\".ccm\"))) = {");
        System.out.println(getWaveFormData(WaveForm.SINPOS, numberOfSteps /2, order));
        System.out.println("};");
        System.out.println("");
        System.out.println("float sinOrZeroTable[] __attribute__ ((section(\".ccm\"))) = {");
        System.out.println(getWaveFormData(WaveForm.SINORZERO, numberOfSteps /2, order));
        System.out.println("};");
        System.out.println("");
        System.out.println("float sinSquareTable[] __attribute__ ((section(\".ccm\"))) = {");
        System.out.println(getWaveFormData(WaveForm.SINSQUARE, numberOfSteps / 2, order));
        System.out.println("};");

}

static String getWaveFormData(WaveForm type, int numberOfSteps, int order) {
        String nl = System.getProperty("line.separator");
        StringBuffer out = new StringBuffer();
        List<Double> values = new ArrayList<Double>();
        double max = -1;
        double min = 1;
        double average = 0;
        out.append(" // ================================================================= ").append(nl);
        out.append(" // "+ type.toString() +" : order "+ order + " / "+ numberOfSteps +" steps").append(nl);
        for (int i=0; i<numberOfSteps; i++) {
                double value = getValue(type, i, numberOfSteps, order);
                if (value<min) {
                        min = value;
                }
                if (value>max) {
                        max = value;
                }
                values.add(value);
                average += value;
        }
        average /= numberOfSteps;

        max -= average;
        min -= average;
        double m1 = -1 / min;
        double m2 = 1 / max;
        double m = m1 > m2 ? m2 : m1;

        if (average < 0.01d)
                average = 0;
        if (m>0.99d && m<1.01d)
                m = 1;
        out.append(" // average : " + average + " / amplitude : "+ (max - min)).append(nl);
        int i = 0;
        double nValue;
        for (double value : values) {
                if (type == WaveForm.SIN) {
                        nValue = value;
                } else {
                        nValue = (value - average) * m;
                }
                String stringValue = df.format(nValue);
                out.append(stringValue);
                for (int s = 0; s<(20-stringValue.length()); s++) {
                        out.append(" ");
                }
                if (i < numberOfSteps -1) {
                        out.append(", ");
                }
                if ((i++ % 5)==4) {
                        out.append(nl);
                }
        }

        return out.toString();
}

static double getValue(WaveForm type, int i, int numberOfSteps, int order) {
        double step = 2 * Math.PI / numberOfSteps * i;
        double value = 0;
        switch (type) {
        case SAWTOOTH:
                for (double k=1; k<=order; k++) {
                        value += 1/k* java.lang.Math.sin(step * k);
                }
                value = 1/2 - 1/Math.PI * value;
                break;
        case SQUARE:
                for (double k=1; k<=order; k+= 2) {
                        value += 1/k* java.lang.Math.sin(step * k);
                }
                value *= 4/Math.PI;
                break;
        case SIN:
                value = java.lang.Math.sin(step);
                break;
        case SINPOS:
                value = java.lang.Math.sin(step);
                if (value < 0) {
                        value =  -value;
                }
                break;
        case SINSQUARE:
                value = java.lang.Math.sin(step);
                value *= value;
                break;
        case SINORZERO:
                value = java.lang.Math.sin(step);
                if (value < 0) {
                        value = 0;
                }
                break;
        }
        return value;
}


static void printHelp() {
        System.out.println("+-----------------------------------------");
        System.out.println("| Usage :  <steps> <order>");
        System.out.println("|    steps : number of steps");
        System.out.println("|    order : order of your waveforms");
        System.out.println("+-----------------------------------------");
}

static private DecimalFormat df;

}
