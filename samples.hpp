#pragma once
#include <fstream>
#include <filesystem>
#include "native-tools/json.hpp"
struct SampleAsset{std::vector<float>data;double root=60;int rate=48000;std::string name;};
inline std::vector<SampleAsset>readLibrary(const std::filesystem::path&directory){std::vector<SampleAsset>library;std::ifstream catalog(directory/L"catalog.json");if(!catalog)return library;auto json=nlohmann::json::parse(catalog);for(auto&item:json){SampleAsset s;s.root=item.value("detectedMidi",60.0);s.name=item.value("name","");std::ifstream f(directory/item["file"].get<std::string>(),std::ios::binary);char header[44];if(!f.read(header,44)||memcmp(header,"RIFF",4)||memcmp(header+36,"data",4)){library.push_back(std::move(s));continue;}uint32_t bytes;memcpy(&bytes,header+40,4);if(bytes>10000000){library.push_back(std::move(s));continue;}s.data.resize(bytes/2);for(auto&v:s.data){int16_t x=0;f.read((char*)&x,2);v=x/32768.f;}library.push_back(std::move(s));}return library;}
