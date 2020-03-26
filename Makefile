all:
	make -f Makefile.x64
	make -f Makefile.x86

clean:
	make -f Makefile.x64 clean
	make -f Makefile.x86 clean
