## preenfm2 :musical_keyboard:

forked from Ixox/preenfm2

in this version, some more filters & fx :

###### LP2  :
- different flavour of the original LP

###### HP2  :
- same for HP

###### BP2  :
- same for BP

###### Lp3  :
- state variable filter, low pass mode

###### Hp3  :
- state variable filter, high pass mode

###### Bp3  :
- state variable filter, band pass mode

###### Peak  :
- state variable filter, peak mode

###### Notc  :
- state variable filter, notch mode

###### Bell  :
- state variable filter, boost if param amp > 0.5, else cut

###### LowS :
- state variable Low Shelf filter, boost if param amp > 0.5, else cut

###### HigS :
- state variable High Shelf filter, boost if param amp > 0.5, else cut

###### LpHp :
- distortion filter, morph from lowpass at freq=0 to highpass at freq=1

###### BpDs
- saturated bandpass filter

###### LPws :
- distorted low pass mixed with source signal, bottom presence enhancer

###### Tilt :
- emphasis on low (mod < 0.5) or high frequency (mod > 0.5), Freq = breakpoint

###### Pann :
- stereo placement tool : pos = panning, sprd = 3 x pole spread

###### Sat  :
- kind of guitar saturation ; signal over threshold is distorded

###### Sigm :
- tanh waveshaper saturation

###### Fold :
- signal is amplified by "driv", signal over 1 is folded, as many time as needed

###### Wrap :
- signal is amplified by "driv", signal over 1 is wraped ; it is mirrored at -1

###### Rot  :
- signal rotation in the stereo field

###### Txr1 :
- bitmangling filtered texture 1 

###### Txr2 :
- bitmangling filtered texture 2, different kind

###### LPx1 :
- xor low pass filter + fold 

###### LPx2 :
- xor low pass filter + fold, different kind

###### LpSn :
- saturated low pass filter, for polyphonic context 

###### HpSn :
- saturated high pass filter, for polyphonic context 

###### Not4 :
- quad notch filter, Sprd the 4 poles around the center frequency

###### Ap4 :
- all pass quad filter, Sprd the 4 poles around the center frequency

###### Ap4b :
- all pass quad filter, resonant version, Sprd the 4 poles around the center frequency

###### Ap4D :
- all pass quad filter, diffusion version, Sprd the 4 poles around the center frequency

###### Oryx :
- 2 peaks formant filter with some distortion

###### Orx2 :
- 3 peaks formant filter

###### Orx3 :
- 3 peaks formant filter + more distortion

###### 18db :
- 3 pole soft LP

###### La+d :
- ladder version 1

###### Lad+ :
- ladder version 2

###### Diod :
- ladder version 3

###### L+d+ :
- ladder version 4

###### H3O+ :
- tb303 filter wanabee, built in accent for notes over velocity 80

###### Svh3 :
- svf filter with hp in feedback loop, built in accent

###### Alkx :
- double sample rate reduction



### Some more tweaks :control_knobs:


#### Matrix sources :
###### rndK :
 (random on new Key)
 new random value with each new note

###### notD :
 (note Difference)
 difference between current note value and previous one.

###### velD :
 (velocity Difference)
 difference between current velocity and previous one.
###### actv :
 (actives voices)
 % of voices played in current timbre
###### rpt :
 (repeat)
 at each new note, value increased by by 1/16 if note = previous note, else decrease (min 0, max 1).
###### dvrg :
 (divergence)
 value increased by by 1/16 if note different from previous note, else decrease.
###### spee :
 (speed)
 decrease from 1 to 0 as the note is far from previous one
###### dura :
 (duration)
 duration of previous note

##### access of sources from next and previous timbre :

###### nD-1
###### nD+1
###### vD-1
###### vD+1
###### ac-1
###### ac+1
###### rp-1
###### rp+1
###### dv-1
###### dv+1
###### sp-1
###### sp+1
###### du-1
###### du+1

#### Glide enhancement :

* A note stack is used to allow gliding on polyphonic patch
* Monophonic patch get also benefit of this note stack (one can press lot of keys : on release, good note should be played)

#### Matrix destinations :
###### G.hd
glide hold (if >0)
###### G.rt
glide rate
###### G.sk
glide skew (different glide speed for ascending or descending intervals)
###### sq1S
start point of seq1 ( 0 to 1 to start from step 1 to 16)
###### sq2S
start point of seq2


#### Evelope loop :
on modulators envelope only, set release to level 1 and duration to zero, the env will loop A,D,S part of the envelope.


Credits : 

	many thanks to :
	- of course to Xavier Hosxe for the amazing PreenFM2 synth !
	- musicdsp.org for the good filters algo ( and more).
	- Andrew Simper for the bell, low shelf and high shelf algo.
	- Dennis Cronin for the allpass algo.
	- Karlsen, Stilson et al. for the ladder algo
	- Andy Sloane for the 303 filter algo
