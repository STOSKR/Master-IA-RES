all: mdbscan
OPEN_LIBS=`pkg-config --cflags --libs opencv4` -lpugixml
INCLUDES=-I. -I../LIB


mdbscan: mdbscan.cc  ../LIB/slopeClass.cc 
	make -C ../LIB
	g++ -o mdbscan mdbscan.cc  ../LIB/slopeClass.cc ../LIB/libPoints.o ../LIB/libXmlPAGE.o ${INCLUDES} ${OPEN_LIBS} -O3
clean:
	rm -f mdbscan ../LIB/libPoints.o ../LIB/libXmlPAGE.o


