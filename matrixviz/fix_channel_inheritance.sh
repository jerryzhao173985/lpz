#!/bin/bash
# Fix channel classes that should inherit from AbstractPlotChannel

cd /Users/jerry/lpzrobot_mac/matrixviz/src/Channel

# Fix AxesPlotChannel.h
sed -i '' 's/class AxesPlotChannel{/class AxesPlotChannel : public AbstractPlotChannel {/' AxesPlotChannel.h
sed -i '' 's/virtual ~AxesPlotChannel() override;/virtual ~AxesPlotChannel();/' AxesPlotChannel.h

# Fix DefaultPlotChannel.h
sed -i '' 's/class DefaultPlotChannel{/class DefaultPlotChannel : public AbstractPlotChannel {/' DefaultPlotChannel.h
sed -i '' 's/virtual ~DefaultPlotChannel() override;/virtual ~DefaultPlotChannel();/' DefaultPlotChannel.h

# Fix IRPlotChannel.h
sed -i '' 's/class IRPlotChannel{/class IRPlotChannel : public AbstractPlotChannel {/' IRPlotChannel.h
sed -i '' 's/virtual ~IRPlotChannel() override;/virtual ~IRPlotChannel();/' IRPlotChannel.h

# Fix MotorCurrentPlotChannel.h
sed -i '' 's/class MotorCurrentPlotChannel{/class MotorCurrentPlotChannel : public AbstractPlotChannel {/' MotorCurrentPlotChannel.h
sed -i '' 's/virtual ~MotorCurrentPlotChannel() override;/virtual ~MotorCurrentPlotChannel();/' MotorCurrentPlotChannel.h

# Fix MotorSpeedPlotChannel.h
sed -i '' 's/class MotorSpeedPlotChannel{/class MotorSpeedPlotChannel : public AbstractPlotChannel {/' MotorSpeedPlotChannel.h
sed -i '' 's/virtual ~MotorSpeedPlotChannel() override;/virtual ~MotorSpeedPlotChannel();/' MotorSpeedPlotChannel.h

# Fix TiltPlotChannel.h
sed -i '' 's/class TiltPlotChannel{/class TiltPlotChannel : public AbstractPlotChannel {/' TiltPlotChannel.h
sed -i '' 's/virtual ~TiltPlotChannel() override;/virtual ~TiltPlotChannel();/' TiltPlotChannel.h

echo "Done fixing channel inheritance"