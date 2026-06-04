OPENCV_LIBS=`pkg-config --cflags --libs opencv4`
PROGS=rand_trees
OPTIM=-O3
all:${PROGS}

rand_trees: rand_trees.cc
	g++ -o rand_trees rand_trees.cc ${OPENCV_LIBS} ${OPTIM}

clean:
	rm -f rand_trees
