CC=icc
TOP=.
INCLUDES=-I$(TOP)
CFLAGS=
COMPILE=$(CC) $(INCLUDES) $(CFLAGS)
INCL=./jrb.h ./jval.h ./dllist.h ./fifo.h
COMMON=.
BOBJ=$(COMMON)/jval.o $(COMMON)/jrb.o $(COMMON)/dllist.o $(COMMON)/fifo.o
LIBS= -lm 
TARGET=ce_constructor

all: $(TARGET)

$(TARGET): $(TARGET).c $(INCL) ${BOBJ}
	$(COMPILE) -o $(TARGET) $(TARGET).c $(BOBJ) $(LIBS)

fifo.o: fifo.c fifo.h jrb.h jval.h dllist.h
	$(COMPILE) -c fifo.c

jrb.o: jrb.c jrb.h jval.h
	$(COMPILE) -c jrb.c

jval.o: jval.c jval.h
	$(COMPILE) -c jval.c

dllist.o: dllist.c dllist.h jval.h
	$(COMPILE) -c dllist.c

clean:
	rm -f $(TARGET) *.o