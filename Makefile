VERSION=0.9
CFLAGS=-O2 -Wall -Wextra -DVERSION=\"$(VERSION)\"
OBJS=truncate.o error.o

all: truncate

truncate: $(OBJS)
	$(CC) -Wall -W $(OBJS) -o truncate

install: all
	cp truncate $(DESTDIR)/usr/local/bin
	cp truncate.1 $(DESTDIR)/usr/local/share/man/man1
	cp truncate.1.it $(DESTDIR)/usr/local/share/man/it/man1/

clean:
	rm -f $(OBJS) truncate

package: clean
	# source package
	rm -rf truncate-$(VERSION)*
	mkdir truncate-$(VERSION)
	cp *.c truncate.1* Makefile license.txt truncate-$(VERSION)
	tar czf truncate-$(VERSION).tgz truncate-$(VERSION)
	rm -rf truncate-$(VERSION)
