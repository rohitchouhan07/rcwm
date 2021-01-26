CFLAGS+= -Wall
LDADD+= -lX11 
LDFLAGS=
EXEC=rcwm

PREFIX?= /usr
BINDIR?= $(PREFIX)/bin

CC=gcc

all: $(EXEC)

rcwm: rcwm.o
	$(CC) $(LDFLAGS) -Os -o $@ $+ $(LDADD)

install: all
	install -Dm 755 rcwm $(DESTDIR)$(BINDIR)/rcwm

clean:
	rm -f rcwm *.o
