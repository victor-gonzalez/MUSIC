# ===========================================================================
#  Makefile for MUSIC                                 Chun Shen Dec. 12, 2014
# ===========================================================================
##
##  Environments :	MAIN	= 	main sourcefile	
##
##  Usage : 	(g)make	[all]		compile the whole project		
##			install	make all and copy binary to $INSTPATH
##			clean		remove objectfiles in obj_$TYPE 
##			distclean	remove all objectsfiles and binaries
##  

#CXX := g++-mp-6
#CFLAGS= -Wall -g -O2 -qopenmp $(shell gsl-config --cflags)
CFLAGS= -g -Wall -std=c++11 -O3 -flto -malign-data=cacheline -finline-functions -march=native -fopenmp $(shell gsl-config --cflags) -DDOCTEST_CONFIG_DISABLE

RM		=	rm -f
O               =       .o
LDFLAGS         =       $(CFLAGS) $(shell gsl-config --libs)
SYSTEMFILES     =       $(SRCGNU)

# --------------- Files involved ------------------

ifeq "$(MAIN)" ""
MAIN		=	mpihydro
endif

SRC		=	main.cpp music.cpp cell.cpp eos.cpp init.cpp \
            util.cpp evolve.cpp advance.cpp u_derivative.cpp \
			reconst.cpp dissipative.cpp minmod.cpp grid_info.cpp \
			cornelius.cpp read_in_parameters.cpp hydro_source.cpp \
			pretty_ostream.cpp freeze.cpp freeze_pseudo.cpp reso_decay.cpp grid.cpp \
			emoji.cpp

INC		= 	music.h cell.h eos.h init.h util.h data.h \
			evolve.h advance.h u_derivative.h reconst.h dissipative.h \
			minmod.h grid_info.h cornelius.h read_in_parameters.h emoji.h \
			hydro_source.h pretty_ostream.h freeze.h int.h grid.h

# -------------------------------------------------

OBJDIR		=	obj
SRCFILES 	= 	$(SRC) $(INC) GNUmakefile
OBJECTS		=	$(addprefix $(OBJDIR)/, $(addsuffix $O, \
			$(basename $(SRC))))
TARGET		=	$(MAIN)
INSTPATH	=	../

# --------------- Pattern rules -------------------

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

%.cpp:
	if [ -f $@ ] ; then touch $@ ; else false ; fi

# -------------------------------------------------

.PHONY:		all mkobjdir clean distclean install

all:		mkobjdir $(TARGET)

help:
		@grep '^##' GNUmakefile

mkobjdir:	
		-@mkdir -p $(OBJDIR)

$(TARGET):	$(OBJECTS)	
		$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS) 
#		strip $(TARGET)

clean:		
	rm -f obj/*
	rm -f mpihydro

distclean:	
		-rm $(TARGET)
		-rm -r obj

install:	$(TARGET)
		cp $(TARGET) $(INSTPATH)

# --------------- Dependencies -------------------
