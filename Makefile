EXEC=maxflow
OLD_PWD=..
SRC=src

all: $(SRC)
	cd $(SRC) && make && cp $(EXEC) $(OLD_PWD)

clean: $(SRC)
	rm -f $(EXEC) && cd $(SRC) && make clean
