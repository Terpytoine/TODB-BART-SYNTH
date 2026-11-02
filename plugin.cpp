#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstring>
#include <cstdio>
#include <string>
#include <mutex>
#include <functional>
#include <filesystem>
#include <fstream>
#include "native-tools/json.hpp"
#include "engine.hpp"
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivstunits.h"
#include "preset-support.hpp"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"
#include "pluginterfaces/vst/vstspeaker.h"
#include "pluginterfaces/gui/iplugview.h"
using namespace Steinberg;using namespace Steinberg::Vst;
DECLARE_UID(componentCID,0x724F4442,0xBA475938,0xAD573891,0x11DB2136)
inline bool uid(const TUID a,const TUID b){return !std::memcmp(a,b,16);}inline void wide(TChar*dst,const char*s,int max=127){int i=0;for(;s[i]&&i<max;i++)dst[i]=static_cast<unsigned char>(s[i]);dst[i]=0;}
class Plugin;
IPlugView* makeView(Plugin*);
extern HMODULE moduleHandle;
std::filesystem::path resourceDir();
class Plugin:public IComponent,public IAudioProcessor,public IEditController,public IMidiMapping,public IProcessContextRequirements,public IUnitInfo{
 std::atomic<uint32>refs{1};
public: bool standalone=false;std::function<nlohmann::json()>deviceInfo;std::function<nlohmann::json(const nlohmann::json&)>deviceAction;Engine engine;Config savedConfig;IComponentHandler*handler=nullptr;std::mutex stateMutex;std::string extra="{}";std::atomic<uint64_t>revision{1};
 static constexpr ParamID programParamID=0x42525450;static constexpr ProgramListID programListID=0;std::vector<FactoryProgram>programs;std::atomic<int>currentProgram{0},pendingProgram{-1},programMetadata{-1},programSteps{0};std::array<std::atomic<int>,16>midiBanks{};
 Plugin():savedConfig(engine.config){try{engine.samples=readLibrary(resourceDir()/L"samples");}catch(...){}try{programs=readPrograms(resourceDir()/L"ui"/L"factory-bank.json");for(int i=0;i<PARAM_COUNT;i++)engine.parameters[i]=programs[0].values[i];savedConfig.matrixCount=0;savedConfig.route=0;savedConfig.loopEnd=routeDefs[0].count-1;engine.config=savedConfig;extra=programs[0].metadata;}catch(...){}}
 ~Plugin(){if(handler)handler->release();}
 tresult PLUGIN_API queryInterface(const TUID id,void**obj)override{if(!obj)return kInvalidArgument;*obj=nullptr;if(uid(id,FUnknown_iid)||uid(id,IPluginBase_iid)||uid(id,IComponent_iid))*obj=static_cast<IComponent*>(this);else if(uid(id,IAudioProcessor_iid))*obj=static_cast<IAudioProcessor*>(this);else if(uid(id,IEditController_iid))*obj=static_cast<IEditController*>(this);else if(uid(id,IMidiMapping_iid))*obj=static_cast<IMidiMapping*>(this);else if(uid(id,IProcessContextRequirements_iid))*obj=static_cast<IProcessContextRequirements*>(this);else if(uid(id,IUnitInfo_iid))*obj=static_cast<IUnitInfo*>(this);if(!*obj)return kNoInterface;addRef();return kResultOk;}
 uint32 PLUGIN_API addRef()override{return ++refs;}uint32 PLUGIN_API release()override{auto r=--refs;if(!r)delete this;return r;}
 tresult PLUGIN_API initialize(FUnknown*)override{return kResultOk;}tresult PLUGIN_API terminate()override{return kResultOk;}
 tresult PLUGIN_API getControllerClassId(TUID)override{return kResultFalse;}tresult PLUGIN_API setIoMode(IoMode)override{return kResultOk;}
 int32 PLUGIN_API getBusCount(MediaType t,BusDirection d)override{return d==kInput?(t==kEvent?1:0):(t==kAudio?1:0);}
 tresult PLUGIN_API getBusInfo(MediaType t,BusDirection d,int32 i,BusInfo&b)override{if(i!=0||!getBusCount(t,d))return kInvalidArgument;b={};b.mediaType=t;b.direction=d;b.channelCount=t==kAudio?2:16;b.busType=kMain;b.flags=BusInfo::kDefaultActive;wide(b.name,t==kAudio?"Stereo output":"MIDI / Piano Roll");return kResultOk;}
 tresult PLUGIN_API getRoutingInfo(RoutingInfo&,RoutingInfo&)override{return kNotImplemented;}
 tresult PLUGIN_API activateBus(MediaType t,BusDirection d,int32 i,TBool)override{return i==0&&getBusCount(t,d)?kResultOk:kInvalidArgument;}
 tresult PLUGIN_API setActive(TBool b)override{engine.active=b;if(!b)engine.panic();return kResultOk;}
 tresult PLUGIN_API setBusArrangements(SpeakerArrangement*,int32 ni,SpeakerArrangement*out,int32 no)override{return ni==0&&no==1&&out&&out[0]==SpeakerArr::kStereo?kResultOk:kResultFalse;}
 tresult PLUGIN_API getBusArrangement(BusDirection d,int32 i,SpeakerArrangement&a)override{if(d!=kOutput||i!=0)return kInvalidArgument;a=SpeakerArr::kStereo;return kResultOk;}
 tresult PLUGIN_API canProcessSampleSize(int32 s)override{return s==kSample32||s==kSample64?kResultOk:kResultFalse;}
 uint32 PLUGIN_API getLatencySamples()override{return 0;}
 tresult PLUGIN_API setupProcessing(ProcessSetup&s)override{if(s.sampleRate<8000||s.sampleRate>192000)return kInvalidArgument;engine.prepare(s.sampleRate);return kResultOk;}
 tresult PLUGIN_API setProcessing(TBool b)override{if(!b)engine.panic();return kResultOk;}
 uint32 PLUGIN_API getTailSamples()override{return uint32(engine.sr*8);}
 uint32 PLUGIN_API getProcessContextRequirements()override{return kNeedTempo|kNeedTransportState|kNeedProjectTimeMusic;}
 tresult PLUGIN_API process(ProcessData&d)override{
  if(d.processContext&&(d.processContext->state&ProcessContext::kTempoValid))engine.hostTempo=d.processContext->tempo;else engine.hostTempo=plain(P_tempo,engine.parameters[P_tempo]);
  engine.loadParams();engine.drain();applyPendingProgramAudio();int ec=d.inputEvents?d.inputEvents->getEventCount():0,ei=0;Event event{};bool eventReady=ec>0&&d.inputEvents->getEvent(0,event)==kResultOk;
  auto changes=d.inputParameterChanges;int pc=changes?changes->getParameterCount():0;
  // Hosts may flush automation in a zero-sample process call.
  if(d.numSamples==0){for(int q=0;q<pc;q++){auto queue=changes->getParameterData(q);if(!queue||(queue->getParameterId()>=PARAM_COUNT&&queue->getParameterId()!=programParamID))continue;int32 offset;ParamValue value;if(queue->getPointCount()&&queue->getPoint(queue->getPointCount()-1,offset,value)==kResultOk){if(queue->getParameterId()==programParamID)applyProgramAudio(programIndex(value));else engine.parameters[queue->getParameterId()]=limit(value,0,1);}}while(eventReady){if(event.type==Event::kNoteOnEvent)engine.noteOn(event.noteOn.pitch,event.noteOn.velocity,event.noteOn.noteId,event.noteOn.channel);else if(event.type==Event::kNoteOffEvent)engine.noteOff(event.noteOff.pitch,event.noteOff.noteId,event.noteOff.channel);else if(event.type==Event::kLegacyMIDICCOutEvent&&event.midiCCOut.controlNumber==kCtrlProgramChange)applyProgramAudio(event.midiCCOut.value);ei++;eventReady=ei<ec&&d.inputEvents->getEvent(ei,event)==kResultOk;}return kResultOk;}
  for(int s=0;s<d.numSamples;s++){
   bool changed=false;for(int q=0;q<pc;q++){auto queue=changes->getParameterData(q);if(!queue||(queue->getParameterId()>=PARAM_COUNT&&queue->getParameterId()!=programParamID))continue;for(int point=0;point<queue->getPointCount();point++){int32 offset;ParamValue value;if(queue->getPoint(point,offset,value)==kResultOk&&offset==s){if(queue->getParameterId()==programParamID)applyProgramAudio(programIndex(value));else engine.parameters[queue->getParameterId()]=limit(value,0,1);changed=true;}}}if(changed)engine.loadParams();
   while(eventReady&&event.sampleOffset<=s){if(event.type==Event::kNoteOnEvent)engine.noteOn(event.noteOn.pitch,event.noteOn.velocity,event.noteOn.noteId,event.noteOn.channel);else if(event.type==Event::kNoteOffEvent)engine.noteOff(event.noteOff.pitch,event.noteOff.noteId,event.noteOff.channel);else if(event.type==Event::kLegacyMIDICCOutEvent&&event.midiCCOut.controlNumber==kCtrlProgramChange)applyProgramAudio(event.midiCCOut.value);ei++;eventReady=ei<ec&&d.inputEvents->getEvent(ei,event)==kResultOk;}
   float l,r;engine.sample(l,r);if(d.numOutputs>0&&d.outputs&&d.outputs[0].numChannels>=2){if(d.symbolicSampleSize==kSample32){d.outputs[0].channelBuffers32[0][s]=l;d.outputs[0].channelBuffers32[1][s]=r;}else {d.outputs[0].channelBuffers64[0][s]=l;d.outputs[0].channelBuffers64[1][s]=r;}d.outputs[0].silenceFlags=0;}
  }return kResultOk;
 }
 tresult PLUGIN_API getState(IBStream*stream)override{if(!stream)return kInvalidArgument;synchronizeProgramState();std::lock_guard<std::mutex>lock(stateMutex);int32 written;nlohmann::json metadata;try{metadata=nlohmann::json::parse(extra);}catch(...){metadata=nlohmann::json::object();}if(!metadata.is_object())metadata=nlohmann::json::object();metadata["sampleCatalogSize"]=specs[P_sample].count;std::string stateExtra=metadata.dump();uint32 magic=0x42525432,count=PARAM_COUNT,size=uint32(stateExtra.size());if(stream->write(&magic,4,&written)!=kResultOk)return kResultFalse;if(stream->write(&count,4,&written)!=kResultOk)return kResultFalse;for(auto&v:engine.parameters){double x=v.load();if(stream->write(&x,8,&written)!=kResultOk)return kResultFalse;}stream->write(&savedConfig,sizeof(Config),&written);stream->write(&size,4,&written);if(size)stream->write(stateExtra.data(),size,&written);return kResultOk;}
 tresult PLUGIN_API setState(IBStream*stream)override{if(!stream)return kInvalidArgument;uint32 magic,size;int32 n;if(stream->read(&magic,4,&n)!=kResultOk||n!=4||(magic!=0x42525431&&magic!=0x42525432))return kResultFalse;uint32 count=133;if(magic==0x42525432&&(stream->read(&count,4,&n)!=kResultOk||n!=4||count>PARAM_COUNT))return kResultFalse;std::array<double,PARAM_COUNT>values;for(int i=0;i<PARAM_COUNT;i++)values[i]=normalized(i,specs[i].def);Config conf;for(uint32 i=0;i<count;i++)if(stream->read(&values[i],8,&n)!=kResultOk||n!=8||!std::isfinite(values[i]))return kResultFalse;if(stream->read(&conf,sizeof(Config),&n)!=kResultOk||n!=sizeof(Config))return kResultFalse;if(stream->read(&size,4,&n)!=kResultOk||n!=4||size>1000000)return kResultFalse;std::string text(size,'\0');if(size&&(stream->read(text.data(),size,&n)!=kResultOk||n!=size))return kResultFalse;if(count>uint32(P_sample)){int oldSize=113;try{auto metadata=nlohmann::json::parse(text);oldSize=metadata.value("sampleCatalogSize",113);}catch(...){}if(oldSize<2||oldSize>10000)oldSize=113;int oldIndex=int(std::lround(limit(values[P_sample],0,1)*(oldSize-1)));values[P_sample]=normalized(P_sample,std::clamp(oldIndex,0,specs[P_sample].count-1));}conf.matrixCount=std::clamp(conf.matrixCount,0,12);conf.route=std::clamp(conf.route,0,5);for(auto&m:conf.matrix){m.dest=std::clamp(m.dest,0,PARAM_COUNT-1);m.source=std::clamp(m.source,0,7);m.amount=std::isfinite(m.amount)?limit(m.amount,-1,1):0;}conf.sequence=false;conf.autoTrain=false;conf.trainPosition=std::isfinite(conf.trainPosition)?limit(conf.trainPosition,0,1):.24;conf.loopStart=std::clamp(conf.loopStart,0,routeDefs[conf.route].count-1);conf.loopEnd=std::clamp(conf.loopEnd,conf.loopStart,routeDefs[conf.route].count-1);{std::lock_guard<std::mutex>lock(stateMutex);extra=text;savedConfig=conf;for(int i=0;i<PARAM_COUNT;i++)engine.parameters[i]=limit(values[i],0,1);}Command c;c.type=4;c.config=conf;engine.commands.push(c);recognizeProgramState(text);revision++;return kResultOk;}
 tresult PLUGIN_API setComponentState(IBStream*s)override{return setState(s);}
 int32 PLUGIN_API getParameterCount()override{return PARAM_COUNT+1;}
 tresult PLUGIN_API getParameterInfo(int32 id,ParameterInfo&i)override{if(id==PARAM_COUNT){i={};i.id=programParamID;wide(i.title,"Factory preset");wide(i.shortTitle,"Preset");i.stepCount=std::max(0,int(programs.size())-1);i.defaultNormalizedValue=0;i.unitId=kRootUnitId;i.flags=ParameterInfo::kIsList|ParameterInfo::kIsProgramChange;return kResultOk;}if(id<0||id>=PARAM_COUNT)return kInvalidArgument;i={};i.id=id;wide(i.title,specs[id].label);wide(i.shortTitle,specs[id].label);wide(i.units,specs[id].unit);i.stepCount=specs[id].count?specs[id].count-1:0;i.defaultNormalizedValue=normalized(id,specs[id].def);i.flags=ParameterInfo::kCanAutomate|(specs[id].count?ParameterInfo::kIsList:0);return kResultOk;}
 std::string display(int id,double val){if(specs[id].count){std::string all=specs[id].choices;int index=int(plain(id,val));size_t pos=0;while(index-->0)pos=all.find('|',pos)+1;return all.substr(pos,all.find('|',pos)-pos);}char text[64];std::snprintf(text,64,"%.3f",plain(id,val));return text;}
 tresult PLUGIN_API getParamStringByValue(ParamID id,ParamValue val,String128 text)override{if(id==programParamID){int index=programIndex(val);if(index<0)return kResultFalse;wide(text,programs[index].name.c_str());return kResultOk;}if(id>=PARAM_COUNT)return kInvalidArgument;wide(text,display(id,val).c_str());return kResultOk;}
 tresult PLUGIN_API getParamValueByString(ParamID id,TChar*text,ParamValue&val)override{if(id==programParamID&&text){std::string name;for(int i=0;text[i]&&i<127;i++)name+=char(text[i]);for(size_t i=0;i<programs.size();i++)if(programs[i].name==name){val=programValue(int(i));return kResultOk;}return kResultFalse;}if(id>=PARAM_COUNT||!text)return kInvalidArgument;std::string t;for(int i=0;text[i]&&i<127;i++)t+=char(text[i]);if(specs[id].count){for(int i=0;i<specs[id].count;i++)if(display(id,double(i)/(specs[id].count-1))==t){val=normalized(id,i);return kResultOk;}return kResultFalse;}char*end;double v=std::strtod(t.c_str(),&end);if(end==t.c_str()||!std::isfinite(v))return kResultFalse;val=normalized(id,v);return kResultOk;}
 ParamValue PLUGIN_API normalizedParamToPlain(ParamID id,ParamValue val)override{return id==programParamID?programIndex(val):id<PARAM_COUNT?plain(id,val):0;}
 ParamValue PLUGIN_API plainParamToNormalized(ParamID id,ParamValue val)override{return id==programParamID?programValue(int(val)):id<PARAM_COUNT?normalized(id,val):0;}
 ParamValue PLUGIN_API getParamNormalized(ParamID id)override{return id==programParamID?programValue(currentProgram.load()):id<PARAM_COUNT?engine.parameters[id].load():0;}
 tresult PLUGIN_API setParamNormalized(ParamID id,ParamValue val)override{if(id==programParamID){if(!std::isfinite(val)||programs.empty())return kInvalidArgument;int index=programIndex(val);for(int i=0;i<PARAM_COUNT;i++)engine.parameters[i]=programs[index].values[i];currentProgram=index;programMetadata=index;pendingProgram=index;return kResultOk;}if(id>=PARAM_COUNT||!std::isfinite(val))return kInvalidArgument;engine.parameters[id]=limit(val,0,1);return kResultOk;}
 tresult PLUGIN_API setComponentHandler(IComponentHandler*h)override{if(h)h->addRef();if(handler)handler->release();handler=h;return kResultOk;}
 IPlugView* PLUGIN_API createView(FIDString name)override{return name&&!std::strcmp(name,ViewType::kEditor)?makeView(this):nullptr;}
 tresult PLUGIN_API getMidiControllerAssignment(int32 bus,int16,CtrlNumber cc,ParamID&id)override{if(bus!=0)return kResultFalse;if(cc==kPitchBend)id=P_bend;else if(cc==kCtrlModWheel)id=P_vibrato;else if(cc==kCtrlSustainOnOff)id=P_sustain;else if(cc==kCtrlProgramChange)id=programParamID;else return kResultFalse;return kResultOk;}
 int programIndex(double value)const{return programs.empty()?-1:std::clamp(int(std::lround(limit(value,0,1)*(programs.size()-1))),0,int(programs.size()-1));}
 double programValue(int index)const{return programs.size()>1?std::clamp(index,0,int(programs.size()-1))/double(programs.size()-1):0;}
 bool applyProgramAudio(int index){if(index<0||index>=int(programs.size()))return false;auto&values=programs[index].values;engine.panic();engine.resetEffects();for(int i=0;i<PARAM_COUNT;i++)engine.parameters[i]=values[i];engine.paramsWarm=false;engine.loadParams();currentProgram.store(index,std::memory_order_release);programMetadata.store(index,std::memory_order_release);return true;}
 void applyPendingProgramAudio(){int index=pendingProgram.exchange(-1,std::memory_order_acq_rel),steps=programSteps.exchange(0,std::memory_order_acq_rel);if(steps&&!programs.empty()){int base=index>=0?index:std::max(0,currentProgram.load());index=(base+steps)%int(programs.size());if(index<0)index+=int(programs.size());}if(index>=0)applyProgramAudio(index);}
 void midiProgram(int program,int channel){if(channel<0||channel>=16||program<0||program>127)return;int index=midiBanks[channel].load(std::memory_order_relaxed)*128+program;if(index<int(programs.size()))pendingProgram.store(index,std::memory_order_release);}
 void midiBank(int controller,int value,int channel){if(channel<0||channel>=16)return;int prior=midiBanks[channel].load(std::memory_order_relaxed);midiBanks[channel].store(controller==0?((value&127)<<7)|(prior&127):(prior&0x3f80)|(value&127),std::memory_order_relaxed);}
 void recognizeProgramState(const std::string&text){try{auto name=nlohmann::json::parse(text).value("name","");int index=-1;for(size_t i=0;i<programs.size();i++)if(programs[i].name==name){index=int(i);break;}currentProgram=index;pendingProgram=-1;programMetadata=-1;}catch(...){currentProgram=-1;}}
 void synchronizeProgramState(){int index=programMetadata.exchange(-1,std::memory_order_acq_rel);if(index<0||index>=int(programs.size()))return;{std::lock_guard<std::mutex>lock(stateMutex);nlohmann::json metadata;try{metadata=nlohmann::json::parse(extra);}catch(...){metadata=nlohmann::json::object();}if(!metadata.is_object())metadata=nlohmann::json::object();metadata["name"]=programs[index].name;extra=metadata.dump();}revision++;if(handler)handler->restartComponent(kParamValuesChanged);}
 int32 PLUGIN_API getUnitCount()override{return 1;}
 tresult PLUGIN_API getUnitInfo(int32 index,UnitInfo&info)override{if(index)return kInvalidArgument;info={};info.id=kRootUnitId;info.parentUnitId=kNoParentUnitId;info.programListId=programListID;wide(info.name,"TODB BART SYNTH");return kResultOk;}
 int32 PLUGIN_API getProgramListCount()override{return programs.empty()?0:1;}
 tresult PLUGIN_API getProgramListInfo(int32 index,ProgramListInfo&info)override{if(index||programs.empty())return kInvalidArgument;info={};info.id=programListID;info.programCount=int32(programs.size());wide(info.name,"Bay Area factory sounds");return kResultOk;}
 tresult PLUGIN_API getProgramName(ProgramListID list,int32 index,String128 name)override{if(list!=programListID||index<0||index>=int(programs.size()))return kInvalidArgument;wide(name,programs[index].name.c_str());return kResultOk;}
 tresult PLUGIN_API getProgramInfo(ProgramListID list,int32 index,Steinberg::Vst::CString attribute,String128 value)override{if(list!=programListID||index<0||index>=int(programs.size())||!attribute)return kInvalidArgument;if(!std::strcmp(attribute,"PlugInCategory")||!std::strcmp(attribute,"MusicalCategory")){wide(value,programs[index].category.c_str());return kResultOk;}return kResultFalse;}
 tresult PLUGIN_API hasProgramPitchNames(ProgramListID,int32)override{return kResultFalse;}
 tresult PLUGIN_API getProgramPitchName(ProgramListID,int32,int16,String128)override{return kResultFalse;}
 UnitID PLUGIN_API getSelectedUnit()override{return kRootUnitId;}
 tresult PLUGIN_API selectUnit(UnitID unit)override{return unit==kRootUnitId?kResultOk:kInvalidArgument;}
 tresult PLUGIN_API getUnitByBus(MediaType type,BusDirection direction,int32 bus,int32 channel,UnitID&unit)override{if(type!=kEvent||direction!=kInput||bus||channel<0||channel>=16)return kResultFalse;unit=kRootUnitId;return kResultOk;}
 tresult PLUGIN_API setUnitProgramData(int32,int32,IBStream*)override{return kNotImplemented;}

 void edit(int id,double v){if(id<0||id>=PARAM_COUNT)return;v=normalized(id,v);if(std::abs(engine.parameters[id]-v)<1e-9)return;if(handler)handler->beginEdit(id);engine.parameters[id]=v;if(handler){handler->performEdit(id,v);handler->endEdit(id);}}
};
class Factory:public IPluginFactory2{std::atomic<uint32>refs{1};public:tresult PLUGIN_API queryInterface(const TUID id,void**o)override{if(!o)return kInvalidArgument;*o=nullptr;if(uid(id,FUnknown_iid)||uid(id,IPluginFactory_iid)||uid(id,IPluginFactory2_iid)){*o=this;addRef();return kResultOk;}return kNoInterface;}uint32 PLUGIN_API addRef()override{return ++refs;}uint32 PLUGIN_API release()override{return --refs;}tresult PLUGIN_API getFactoryInfo(PFactoryInfo*i)override{if(!i)return kInvalidArgument;*i=PFactoryInfo("TODB / S.O.N Productions","","",PFactoryInfo::kUnicode);return kResultOk;}int32 PLUGIN_API countClasses()override{return 1;}tresult PLUGIN_API getClassInfo(int32 x,PClassInfo*i)override{if(x||!i)return kInvalidArgument;*i=PClassInfo(componentCID,PClassInfo::kManyInstances,kVstAudioEffectClass,"TODB BART SYNTH");return kResultOk;}tresult PLUGIN_API getClassInfo2(int32 x,PClassInfo2*i)override{if(x||!i)return kInvalidArgument;*i=PClassInfo2(componentCID,PClassInfo::kManyInstances,kVstAudioEffectClass,"TODB BART SYNTH",0,"Instrument|Synth","TODB / S.O.N Productions","1.3.1","VST 3.8");return kResultOk;}tresult PLUGIN_API createInstance(FIDString cid,FIDString iid,void**obj)override{if(!obj||!cid||!iid)return kInvalidArgument;*obj=nullptr;if(!uid(cid,componentCID))return kResultFalse;auto p=new Plugin;auto r=p->queryInterface(iid,obj);p->release();return r;}};
static Factory factory;HMODULE moduleHandle=nullptr;
extern "C" __declspec(dllexport) IPluginFactory* GetPluginFactory(){factory.addRef();return &factory;}
extern "C" __declspec(dllexport) bool InitDll(){return true;}extern "C" __declspec(dllexport) bool ExitDll(){return true;}
BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,LPVOID){if(reason==DLL_PROCESS_ATTACH){moduleHandle=instance;DisableThreadLibraryCalls(instance);}return TRUE;}
#include "editor.hpp"
