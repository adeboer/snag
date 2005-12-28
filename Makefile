# snag Makefile

TARGETS = snag smaster

SNAGOBJS = snag.o snagdf.o snaginfo.o thresh.o

SMASOBJS = smaster.o

all: $(TARGETS)

snag : $(SNAGOBJS)
	cc $(SNAGOBJS) -o $@

smaster: $(SMASOBJS)
	cc $(SMASOBJS) -o $@

depend :
	ex -c '/^# DEPENDENCIES/,$$d' -c x Makefile
	echo '# DEPENDENCIES' >> Makefile
	cc -MM *.c >> Makefile

clean :
	rm -f *.o core

clobber : clean
	rm -f $(TARGETS)

install: all
	install snag /usr/local/bin/snag
	install smaster /usr/local/bin/smaster

# DEPENDENCIES
smaster.o: smaster.c
snag.o: snag.c snag.h
snagdf.o: snagdf.c snag.h
snaginfo.o: snaginfo.c snag.h
thresh.o: thresh.c snag.h
