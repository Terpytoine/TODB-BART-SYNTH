#include "engine.hpp"
#include <cstdio>
void frames(Engine&e,int n){e.loadParams();for(int i=0;i<n;i++){float l,r;e.sample(l,r);}}
void setup(Engine&e,int mode){e.prepare(48000);e.config.matrixCount=0;e.config.sequence=false;e.parameters[P_voiceMode]=normalized(P_voiceMode,mode);e.parameters[P_ampSustain]=.8;e.parameters[P_ampRelease]=normalized(P_ampRelease,.03);e.parameters[P_arp]=0;e.parameters[P_sustain]=0;e.loadParams();}
int main(){Engine e;setup(e,0);e.noteOn(60,.8,100);e.noteOn(64,.8,101);e.noteOn(67,.8,102);frames(e,1000);e.noteOff(64,101);e.noteOff(67,102);e.noteOff(60,100);frames(e,48000);if(e.meterVoices||e.heldCount){puts("FAIL: three-note mono chord sticks after fallback and final identified note-off");return 1;}
setup(e,1);for(int pass=0;pass<80;pass++){for(int i=0;i<6;i++)e.noteOn(48+i*3,.8,pass*10+i);frames(e,256);for(int i: {2,0,5,1,4,3})e.noteOff(48+i*3,pass*10+i);frames(e,2400);if(e.meterVoices||e.heldCount){puts("FAIL: poly chord release");return 2;}}
setup(e,1);e.noteOn(60,.8,200);e.noteOn(60,.7,201);frames(e,1000);e.noteOff(60,200);frames(e,4800);if(e.meterVoices!=1||e.heldCount!=1){puts("FAIL: overlapping unison note identity");return 3;}e.noteOff(60,201);frames(e,4800);if(e.meterVoices||e.heldCount)return 4;
setup(e,1);e.noteOn(60,.8,-1,0);e.noteOn(60,.7,-1,1);frames(e,1000);e.noteOff(60,-1,0);frames(e,4800);if(e.meterVoices!=1){puts("FAIL: MIDI channel isolation");return 5;}e.noteOff(60,-1,1);frames(e,4800);if(e.meterVoices||e.heldCount)return 6;
setup(e,1);e.parameters[P_sustain]=1;frames(e,8000);for(int i=0;i<4;i++)e.noteOn(60+i*3,.8,300+i);frames(e,1000);for(int i=0;i<4;i++)e.noteOff(60+i*3,300+i);frames(e,4800);if(e.meterVoices!=4||e.heldCount){puts("FAIL: sustain chord");return 7;}e.parameters[P_sustain]=0;frames(e,10000);if(e.meterVoices){puts("FAIL: sustain release");return 8;}
setup(e,1);for(int i=0;i<40;i++)e.noteOn(36+i,.8,400+i);frames(e,1000);for(int i=0;i<40;i++)e.noteOff(36+i,400+i);frames(e,10000);if(e.meterVoices||e.heldCount){puts("FAIL: stolen voice release");return 9;}
for(auto&lamp:e.noteLamps)if(lamp.load()){puts("FAIL: released MIDI lamps remain lit");return 10;}
puts("Chords: mono fallback and final release, 80 six-note poly chords, overlapping unison, MIDI channels, sustain pedal, and 40-note voice stealing passed.");return 0;}

