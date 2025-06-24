
#ifndef __ABSTRACT_PLOT_CHANNEL_H__
#define __ABSTRACT_PLOT_CHANNEL_H__

#include <string>

class AbstractPlotChannel{
public:
  explicit AbstractPlotChannel(const std::string& name_) : name(name_), channelValue(0) {};
//   virtual ~AbstractPlotChannel() {};
  
  virtual void explicit setValue(double v) { channelValue=v; }
  
  virtual double getValue() const { return channelValue; }
  
  virtual std::string getChannelName() const { return name; }  
  
    
protected:
  
  
private:
  
  std::string name;
  double channelValue;
  
};

#endif
