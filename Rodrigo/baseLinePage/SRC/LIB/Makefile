OPEN_LIBS=`pkg-config --cflags --libs opencv4` -lpugixml

INCLUDES=-I. -I../LIB

all: libPoints.o libXmlPAGE.o slopeClass.o

libPoints.o: libPoints.cc  libPoints.h
	g++ -c libPoints.cc  ${INCLUDES} ${OPEN_LIBS} -O3

libXmlPAGE.o: libXmlPAGE.cc libXmlPAGE.h
	g++ -c libXmlPAGE.cc ${INCLUDES} ${OPEN_LIBS} -O3

slopeClass.o: slopeClass.cc slopeClass.h
	g++ -c slopeClass.cc ${INCLUDES} ${OPEN_LIBS} -O3
clean:
	rm -f libPoints.o libXmlPAGE.o slopeClass.o


