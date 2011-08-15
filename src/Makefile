CXXFLAGS=-Wall
CXX=g++
CFLAGS=-pipe \
			 -O \

# List of partially-compiled files to create.
OBJS= Arc.o Node.o DGraph.o main.o

all: $(OBJS)
	$(CXX) $(CXXFLAGS) $(CFLAGS) -o maxflow $(OBJS) 

clean:
	rm -f *.o maxflow
