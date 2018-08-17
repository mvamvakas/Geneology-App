CC = gcc
CPPFLAG= 
CFLAGS = -Wall -std=c11 -g -Iinclude -fPIC
LDFLAGS = -L.
BIN = ./bin/
SRC = ./src/

all:
	$(CC) $(CFLAGS) $(SRC)LinkedListAPI.c $(SRC)GEDCOMparser.c $(SRC)GEDCOMutilities.c -shared -o ../myLibrary.so







#parser
	
test: TestMain.o list parser
	$(CC) $(CFLAGS) $(LDFLAGS) $(BINDIR)testMain.o -L./bin -lparser -llist -o $(BINDIR)runMe 
	
	
TestMain.o: $(SRCDIR)testMain.c 
	$(CC) $(CFLAGS) -c $(SRCDIR)testMain.c -o $(BINDIR)testMain.o
	
$(LIB): libList.o libParser.o
	$(CC) -shared -o $(LIB) $(BINDIR)LinkedListAPI.o
	$(CC) -shared -o $(LIB) $(BINDIR)GEDCOMparser.o 
	$(CC) -shared -o $(LIB) $(BINDIR)GEDCOMparser.o



libList.o: $(SRCDIR)LinkedListAPI.c
	$(CC) $(CFLAGS) -c -fpic $(SRCDIR)LinkedListAPI.c -o $(BINDIR)LinkedListAPI.o

libParser.o: $(SRCDIR)GEDCOMparser.c $(SRCDIR)GEDCOMutilities.c
	$(CC) $(CFLAGS) -c -fpic $(SRCDIR)GEDCOMutilities.c -o $(BINDIR)GEDCOMutilities.o
	$(CC) $(CFLAGS) -c -fpic $(SRCDIR)GEDCOMparser.c -o $(BINDIR)GEDCOMparser.o

list: liblist.a

liblist.a: $(SRCDIR)LinkedListAPI.c
	$(CC) $(CFLAGS) -c $(SRCDIR)LinkedListAPI.c -o $(BINDIR)LinkedListAPI.o
	#ar cr $(BINDIR)liblist.a $(BINDIR)LinkedListAPI.o 
	$(CC) $(CFLAGS) -c -fpic $(SRCDIR)LinkedListAPI.c
	$(CC) -shared -o $(LIB) $(BINDIR)LinkedListAPI.o
	#rm -rf $(BINDIR)LinkedListAPI.o

parser: libparser.a

libparser.a: $(SRCDIR)GEDCOMparser.c $(SRCDIR)GEDCOMutilities.c
	$(CC) $(CFLAGS) -c $(SRCDIR)GEDCOMutilities.c -o $(BINDIR)GEDCOMutilities.o
	$(CC) $(CFLAGS) -c $(SRCDIR)GEDCOMparser.c -o $(BINDIR)GEDCOMparser.o
	ar cr $(BINDIR)libparser.a $(BINDIR)GEDCOMparser.o $(BINDIR)GEDCOMutilities.o
	rm -rf $(BINDIR)GEDCOMparser.o -rf $(BINDIR)GEDCOMutilities.o

run:
	valgrind --leak-check=full --track-origins=yes ./$(BINDIR)runMe

clean:
	rm -rf $(BINDIR)runMe $(BINDIR)*.o $(BINDIR)*.a
