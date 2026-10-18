// Desktop-only replay of the exact detector used by the Arduino sketch.
#ifndef ARDUINO
#include "StrokeDetector.h"
#include <iostream>
#include <sstream>
#include <string>
int main() {
  StrokeDetector detector;
  std::string line;
  std::getline(std::cin,line);
  const bool raw=line.find("echo_duration_us")!=std::string::npos;
  std::cout<<"trough_ms,down_mm,bpm\n";
  while(std::getline(std::cin,line)) {
    std::stringstream stream(line);
    std::string ts,echo,gap,valid;
    if(!std::getline(stream,ts,','))continue;
    if(raw && !std::getline(stream,echo,','))continue;
    if(!std::getline(stream,gap,',')||!std::getline(stream,valid,','))continue;
    auto event=detector.add(std::stoul(ts),gap.empty()?0:std::stof(gap),std::stoi(valid)==1);
    if(event.down)std::cout<<event.troughMs<<','<<event.downMm<<','<<event.bpm<<'\n';
  }
}
#endif
