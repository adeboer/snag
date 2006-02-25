# snag Makefile

TARGETS = snag smaster

SNAGOBJS = snag.o snagdf.o snaginfo.o thresh.o snag.tab.o lex.yy.o

SMASOBJS = smaster.o

all: $(TARGETS)

snag : $(SNAGOBJS)
	cc $(SNAGOBJS) -o $@ -lfl

smaster: $(SMASOBJS)
	cc $(SMASOBJS) -o $@

depend : snag.tab.c lex.yy.c
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

snag.tab.c snag.tab.h: snag.yacc
	yacc -d -b snag snag.yacc

lex.yy.c: snag.lex
	flex -s snag.lex

# DEPENDENCIES
lex.yy.o: lex.yy.c snag.tab.h snag.h
smaster.o: smaster.c
snag.o: snag.c snag.h
snag.tab.o: snag.tab.c snag.h
snagdf.o: snagdf.c snag.h
snaginfo.o: snaginfo.c snag.h
thresh.o: thresh.c snag.h
