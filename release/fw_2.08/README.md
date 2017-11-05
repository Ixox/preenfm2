### Do not right click !
Click on the firmware's file, then press Download on the following page.


# What's new in 2.08 ?

# Carrier vs Modulator
#### Direct editing : 
There's a new combo button+encoder:
Modifying some value while pressing the left middle button (button 3 from this picture) will modify  other values at the same time.
If you modify any of the 8 enveloppe parameters while pressing this button, it will modify the other enveloppes of the same operator type in the current algo : all carriers or all modulators.
Exemple with ALGO 15 
If you press button 3 while modifying enveloppe of operator 1 (or 3) it will modify 1 & 3 at the  same time.
If you press button 3 while modifying enveloppe of operator 2 (or 4 or 5 or 6) it will modify 2,4,5 & 6 at the same time.
An other example :  Modifying attk of op1 (which is always a carrier operator) while pressing 3 will always modify the attk of the sound because all carriers attk will be modified :)
It also does the following multi-edit :
IM : modifying any modulation index (or v) will edit all IM (or all v) at the same time.
Mix : modifying any mix will change all mix.
Pan : modifying any pan will change all pans.

#### Matric modulation :
The legacy att*, dec*, rel* matrix targets are now envC, decC, relC and modify attack, decay and release of all carrier operator envelopes.
After each one you'll find envM, decM, relM that modify attack, decay and release of all modulator operator envelopes.
Controlling all modulator envelopes with the modwheel (matric source ModW) is very intersting.

#### Midi control:
. Legacy midi CC 80 now modifies enveloppe attacks of all carrier operators.
. Legacy midi CC 81 now modifies enveloppe releases  of all carrier operators.
. New midi CC 62 modifies enveloppe attacks of all modulator operators.
. New midi CC 63 modifies enveloppe releases  of all modulator operators.
Midi page updated : http://ixox.fr/preenfm2/manual/midi/

# LFO Frequency
LFO1-3 frequency can now go up to 99.9Hz.
!!! Your frequencies synced to midi clock (MC/16,  MC/8, MC/4 etc....) will be 24.x Hz after this modification.
Which means you'll have to turn their values up after 99.9 to set back your Midi clock synced values !!!!

# Lokki's modification 
http://ixox.fr/forum/index.php?topic=69326.0
1. hertz mode: in the Operator Oscillator page, chose Ftyp = kehz. That will make "fine tune" (FTun) value behaves in hertz. So that beating stays the same over the whole range of the midi notes.
2. global midi channel: if enabled in the menu (MENU>Tools>Set>Global Ch),  midi coming in to this channel will be sent to all four instruments. 

# Step Sequencer and NRPN
NRPN used to always modify the step sequencer of the curren instrument, not the one of the specified midi channel. It's now fixed.
