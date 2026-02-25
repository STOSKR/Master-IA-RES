all: lineExtractor
INCLUDE=-I. -I../LIB
OPEN_LIBS=`pkg-config --cflags --libs opencv4` -lpugixml

PROG_LIBS=../LIB/libXmlPAGE.o ../LIB/libPoints.o ../LIB/slopeClass.o


lineExtractor: lineExtractor.cc  Graf.o  ${PROG_LIBS}
	make -C ../LIB
	g++ -o lineExtractor lineExtractor.cc Graf.o  ${PROG_LIBS} ${INCLUDE}  ${OPEN_LIBS}  -O3

Graf.o: Graf.cc Graf.h
	g++ -c Graf.cc ${INCLUDE}  ${OPEN_LIBS}  -O3

slopeClass.o: slopeClass.cc slopeClass.h
	g++ -c slopeClass.cc  ${INCLUDE}  ${OPEN_LIBS}  -O3

clean:
	rm -f lineExtractor *.o


