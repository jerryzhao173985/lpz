# -*- mode: makefile; -*-
# Do not edit, this is file generated by m4 from Makefile.4sim.m4 and
#  copied to the simulations while installation!
# Make your changes in Makefile.conf.
#  E.g. add there files to compile or custom libs and so on.

# File:       Makefile for lpzrobots simulations
# Author:     Georg Martius  <georg.martius at web.de>
# Date:       Oct 2009

define(`COMMENT', )
COMMENT(`There are defines for platform dependend stuff.
         DEV only produces output in the development version
         DEVORUSER  choice between development (first argument) or user (second)
         LINUXORMAC outputs the first argument on linux and the second on mac.')

ifdef(`DEVEL',
`define(`DEVORUSER', $1) define(`DEV', $1)'
,
`define(`DEVORUSER', $2) define(`DEV',)'
)
ifdef(`MAC',
`define(`LINUXORMAC', $2)'
,
`define(`LINUXORMAC', $1)'
)

EXEC = start
# add files to compile in the conf file
include Makefile.conf

CFILES = $(addsuffix .cpp, $(FILES))
OFILES = $(addsuffix .o, $(FILES))

# the CFGOPTS are set by the opt and dbg target
CFGOPTS=
INC    += -I.
# Find config scripts relative to simulation directory
SELFORGCFG := $(shell if [ -x ../../../selforg/selforg-config ]; then echo "../../../selforg/selforg-config"; else echo "selforg-config"; fi)
ODEROBOTSCFG := $(shell if [ -x ../../ode_robots-config ]; then echo "../../ode_robots-config"; else echo "ode_robots-config"; fi)

BASELIBS = $(shell $(ODEROBOTSCFG) $(CFGOPTS) --static --libs) $(shell $(SELFORGCFG) $(CFGOPTS) --static --libs)
BASELIBSSHARED := $(shell $(ODEROBOTSCFG) $(CFGOPTS) --libs) $(shell $(SELFORGCFG) $(CFGOPTS) --libs)


DEV(LIBSELFORG=$(shell $(SELFORGCFG) $(CFGOPTS) --libfile))
DEV(LIBSELFORGSHARED:=$(shell $(SELFORGCFG) $(CFGOPTS) --solibfile))
DEV(SELFORGSRCPREFIX=$(shell $(SELFORGCFG) $(CFGOPTS) --srcprefix))

DEV(LIBODEROBOTS=$(shell $(ODEROBOTSCFG) $(CFGOPTS) --libfile))
DEV(LIBODEROBOTSSHARED:=$(shell $(ODEROBOTSCFG) $(CFGOPTS) --solibfile))
DEV(ODEROBOTSSRCPREFIX=$(shell $(ODEROBOTSCFG) $(CFGOPTS) --srcprefix))




LIBS  += $(BASELIBS) $(ADDITIONAL_LIBS)

INC   += -I.
# Add system include path for ode-dbl symlinks if they exist locally
ifneq ($(wildcard ../../../include/ode-dbl),)
  INC += -isystem ../../../include/ode-dbl
endif
ifneq ($(wildcard ../../include/ode-dbl),)
  INC += -isystem ../../include/ode-dbl
endif

CXX = g++
# Base flags including external library includes with -isystem
CPPFLAGS_BASE = -Wall -pipe -Wno-deprecated $(INC) $(shell $(SELFORGCFG) $(CFGOPTS) --cflags) \
  $(shell $(ODEROBOTSCFG) $(CFGOPTS) --intern --cflags)

# Additional warnings for our own code only
EXTRA_WARNINGS = -Wextra -Wpedantic -Wfloat-equal -Wold-style-cast -Wno-unused-parameter

# Full flags for compilation
CPPFLAGS = $(CPPFLAGS_BASE) $(EXTRA_WARNINGS)

normal: DEV(libode_robots)
	$(MAKE) $(EXEC)
opt:    DEV(libode_robots_opt)
	$(MAKE) CFGOPTS=--opt EXEC=$(EXEC)_opt $(EXEC)_opt
dbg:    DEV(libode_robots_dbg)
	$(MAKE) CFGOPTS=--dbg EXEC=$(EXEC)_dbg $(EXEC)_dbg
shared:  DEV(libode_robots_shared)
	$(MAKE) BASELIBS="$(BASELIBSSHARED)" LIBSELFORG="$(LIBSELFORGSHARED)" \
		LIBODEROBOTS="$(LIBODEROBOTSSHARED)" $(EXEC)

$(EXEC): Makefile Makefile.depend $(OFILES) DEV($(LIBODEROBOTS) $(LIBSELFORG))
	$(CXX) $(CPPFLAGS) $(OFILES) $(LIBS) -o $(EXEC)

DEV(
libode_robots:
	cd $(ODEROBOTSSRCPREFIX) && $(MAKE) lib

libode_robots_dbg:
	cd $(ODEROBOTSSRCPREFIX) && $(MAKE) dbg

libode_robots_opt:
	cd $(ODEROBOTSSRCPREFIX) && $(MAKE) opt

libode_robots_shared:
	cd $(ODEROBOTSSRCPREFIX) && $(MAKE) shared
)

Makefile.depend:
	makedepend -- $(shell echo "$(CPPFLAGS_BASE)" | sed 's/-isystem/-I/g') -- $(CFILES) -f- > Makefile.depend 2>/dev/null

depend:
	makedepend -- $(shell echo "$(CPPFLAGS_BASE)" | sed 's/-isystem/-I/g') -- $(CFILES)  -f- > Makefile.depend 2>/dev/null

check-syntax:
	$(CXX) $(CPPFLAGS) -Wextra -S -fsyntax-only $(CHK_SOURCES)
	cppcheck --std=c++11 --std=posix --enable=performance,information,portability --suppress=incorrectStringBooleanError --suppress=invalidscanf --quiet --template={file}:{line}:{severity}:{message} $(CHK_SOURCES)

todo:
	find -name "*.[ch]*" -exec grep -Hni "TODO" {} \;

tags:
	etags $(find -name "*.[ch]")

DEV(
cleandist: clean-all
clean-all: clean
	cd $(ODEROBOTSSRCPREFIX) && make clean-all
	cd $(SELFORGSRCPREFIX) && make clean-all
)

clean:
	rm -f $(EXEC) $(EXEC)_dbg $(EXEC)_opt *.o Makefile.depend

-include Makefile.depend
