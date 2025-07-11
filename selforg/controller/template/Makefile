# Makefile for controller template
# Copy this when creating a new controller

# Find selforg base directory
SELFORG = ../..

# Include the generic makefile for controllers
include $(SELFORG)/Makefile.conf

# Add your controller to the library
CONTROLLER_SOURCES = mycontroller.cpp
CONTROLLER_HEADERS = mycontroller.h

# Object files
OBJS = $(CONTROLLER_SOURCES:.cpp=.o)

# Default target
all: $(OBJS)

# Compile rule
%.o: %.cpp %.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(SELFORG)/include -c $< -o $@

# Clean
clean:
	rm -f *.o *~ core

# Install headers (optional)
install: $(CONTROLLER_HEADERS)
	cp $(CONTROLLER_HEADERS) $(SELFORG)/include/selforg/controller/

.PHONY: all clean install