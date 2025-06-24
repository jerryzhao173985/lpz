
#ifndef __TIME_STAMP_PLOT_CHANNEL_H__
#define __TIME_STAMP_PLOT_CHANNEL_H__

#include "AbstractPlotChannel.h"

class TimeStampPlotChannel : public AbstractPlotChannel
{

public:
  explicit TimeStampPlotChannel ( const std::string& name );
  virtual ~TimeStampPlotChannel();
protected:

private:



};

#endif
