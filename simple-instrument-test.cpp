#define main bankRegressionMain
#include "bank-test-v2.cpp"
#undef main
int main(){std::ifstream f("ui/factory-bank.json");auto bank=nlohmann::json::parse(f);if(bank.size()!=350)return 1;Engine e;if(!e.recordData.empty())return 2;e.beginRecord();e.loadParams();e.noteOn(60,.8);for(int i=0;i<512;i++){float l,r;e.sample(l,r);}if(e.recordFrames!=512)return 3;e.recording=false;applyPatch(e,bank[0]);e.parameters[P_arp]=normalized(P_arp,3);e.loadParams();e.noteOn(48,.8);float l,r;e.sample(l,r);if(e.meterVoices<1)return 4;e.panic();e.config.sequence=true;for(int i=0;i<48000;i++)e.sample(l,r);if(e.meterVoices!=0)return 5;printf("PASS:350 sounds retained; recording allocates only on demand; legacy arp state plays direct MIDI; sequencer cannot generate notes.\n");}
