# snag Makefile

package = snag
version = 0.2

TARGETS = snag check_snag
INTERMEDIATE = lex.yy.c snag.tab.c snag.tab.h
SNAGOBJS = snag.o snagdf.o snaginfo.o thresh.o snag.tab.o lex.yy.o snagcmd.o
SMASOBJS = check_snag.o

pv = $(package)-$(version)

all: $(TARGETS)

snag : $(SNAGOBJS)
	cc $(SNAGOBJS) -o $@ -lfl

check_snag: $(SMASOBJS)
	cc $(SMASOBJS) -o $@

depend : snag.tab.c lex.yy.c
	ex -c '/^# DEPENDENCIES/,$$d' -c x Makefile
	echo '# DEPENDENCIES' >> Makefile
	cc -MM *.c >> Makefile

clean :
	rm -f *.o core

clobber : clean
	rm -f $(TARGETS) $(INTERMEDIATE)

install: all
	install snag /usr/local/bin/snag
	install check_snag /usr/local/bin/check_snag

snag.tab.c snag.tab.h: snag.yacc
	yacc -d -b snag snag.yacc

lex.yy.c: snag.lex
	flex -s snag.lex

dist:   
	ln -sf . $(pv)
	sed 's/^/$(pv)\//' MANIFEST | tar cvzf $(pv).tar.gz -T -
	rm $(pv)

# DEPENDENCIES
lex.yy.o: lex.yy.c snag.tab.h snag.h
check_snag.o: check_snag.c
snag.o: snag.c snag.h
snag.tab.o: snag.tab.c snag.h
snagcmd.o: snagcmd.c snag.h
snagdf.o: snagdf.c snag.h
snaginfo.o: snaginfo.c snag.h
thresh.o: thresh.c snag.h
