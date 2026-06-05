BINARY	:=      WordGraph2Index
MOD	:=	main WG WGFile WGN-Best WGParsingString WGConfMeasure WGErrCorr WGPrunning

export PREFIX := 
export CC  :=  g++
#export CFLAGS  :=  -m32

#STATIC    =  -static #-L/usr/lib/x86_64-linux-gnu/
#GPROF     =  -pg
#DEBUG     =  -g $(GPROF)
#TIMES     =  -DTAKETIME
#LIBS      =  -L. -lm
LIBS      =  -Lutils
INC	  =  -Iutils
OPTIONS   =  -Wall -Wno-misleading-indentation -O4 -pedantic -std=gnu++0x -fopenmp $(TIMES)
DOPTIONS  =  -Wall -Wno-misleading-indentation -std=gnu++0x
LDFLAGS   =  -lz $(STATIC)

ifndef DEBUG
export CXXFLAGS = $(OPTIONS) $(CFLAGS) $(INC)
else
export CXXFLAGS = $(DOPTIONS) $(DEBUG) $(CFLAGS) $(INC)
endif

#OBJ     = $(addsuffix .o, $(MAIN)) $(addsuffix .o, $(MOD))
OBJ     = $(addsuffix .o, $(MOD))
UTILS   = utils

all: Utils $(BINARY)
.PHONY: all

Utils: 
	@$(MAKE) -C $(UTILS) --no-print-directory libUtils.a

$(BINARY): $(OBJ) $(UTILS)/libUtils.a
	$(CC) $(CXXFLAGS) -o $(BINARY) $^ $(LIBS) -lUtils $(LDFLAGS)
ifndef DEBUG
	strip $(BINARY)
endif

# pull in dependency info for *existing* .o files
-include $(OBJ:.o=.d)

%.o: %.cc
	$(CC) $(CXXFLAGS) -c $< -o $@
	@$(CC) $(CXXFLAGS) -MM $< > $*.d

clean:
	@-rm -f *.o *.d *~ gmon.out $(BINARY)

distclean: clean
	@$(MAKE) -C $(UTILS) --no-print-directory clean

pack:	$(addsuffix .cc, $(MOD)) *.h Makefile README utils/*.cc utils/*.h utils/Makefile WIN/Makefile WIN/README
	@cd ../; tar --atime-preserve --exclude-vcs -cvjhf $(BINARY).tar.bz2 $(addprefix $(notdir $(PWD))/, $^)

### Para Optimizar tiempos de ejecución
########################################################
# gprof --flat-profile wordGraphTool gmon.out | less   #
########################################################

# Determinacion de dependencias
# g++ -Wall -O3 -MM -c GrammarTools.cc
