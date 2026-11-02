#define main bankRegressionMain
#include "bank-test-v2.cpp"
#undef main
int main(){std::ifstream f("ui/factory-bank.json");auto bank=nlohmann::json::parse(f);for(int i=338;i<341;i++){Engine e;e.samples=readLibrary("samples");applyPatch(e,bank[i]);e.noteOn(60,.85);double peak=0;for(int n=0;n<6000;n++){float l,r;e.sample(l,r);peak=std::max(peak,double(std::max(std::abs(l),std::abs(r))));}e.noteOff(60);double tail=0;for(int n=0;n<4800;n++){float l,r;e.sample(l,r);if(n>2400)tail=std::max(tail,double(std::max(std::abs(l),std::abs(r))));}printf("%s heldPeak %.6f releaseTail %.9f\n",bank[i]["name"].get<std::string>().c_str(),peak,tail);if(peak<.001||tail>.0001)return 1;}return 0;}
