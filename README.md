## preenfm2

forked from Ixox/preenfm2

in this version, some more filters & fx :

######LP2  :
- enhanced version of the original LP (oversampled)

######HP2  :
- same as LP2, but for HP

######BP2  :
- same as LP2, for BP

######LpHp :
- experimental filter, morph from lowpaqq at freq=0 to highpass at freq=1

######Bp3  :
- bandpass filter, different algo from original BP

######Tilt :
- emphasis on low (mod < 0.5) or high frequency (mod > 0.5), Freq = breakpoint

######Pann :
- experimental stereo enhancer : at wide = 0, the signal is mono, at wide = 1, the signal is stereo, larger than original (maybe, depend on the material), pos is the pseudo pan position (bandpass trick) 

######Sat  :
- kind of guitar saturation ; signal over threshold is distorded

######Sigm :
- tanh waveshaper saturation

######Fold :
- signal is amplified by "driv ", signal over 1 is folded, as many time as needed

######Wrap :
- signal is amplified by "driv ", signal over 1 is wraped ; it is mirrored at -1

######Xor  :
- signal over threshold is xor-ed with an allpass version of itself

######Txr1 :
- bitmangling texture 1, thrs : xor-ed intensity

######Txr2 :
- bitmangling texture 2, thrs : xor-ed intensity, different kind

######LPx1 :
- low pass filter with xor after filter, this one can self oscillate on the xor part

######LPx2 :
- low pass filter with xor before filter

######LPws :
- distorted low pass mixed with source signal, bottom presence enhancer

