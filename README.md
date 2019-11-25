## preenfm2

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

###### Xor  :
- signal over threshold is xor-ed with an allpass version of itself

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


Credits : 

	many thanks to :
	- musicdsp.org for the good filters algo ( and more).
	- Andrew Simper for the bell, low shelf and high shelf algo.
	- Dennis Cronin for the allpass algo.
	- Karlsen, Stilson et al. for the ladder algo
