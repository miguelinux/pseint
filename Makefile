all:
	@echo " "
	@echo usar:
	@echo "   make linux           para compilar todo en GNU/Linux con wxWidgets-2.8.x-ansi"
	@echo "   make linux3          para compilar todo en GNU/Linux o Mac OS con wxWidgets-3.1.x-unicode"
	@echo "   mingw32-make win32   para compilar todo en Microsoft Windows"
	@echo En caso de error, leer \"fuentes.txt\".
	@echo " "

linux:
	${MAKE} -f Makefile.lnx2

linux-clean:
	${MAKE} -f Makefile.lnx2 clean

linux3:
	${MAKE} -f Makefile.lnx3

linux3-clean:
	${MAKE} -f Makefile.lnx3 clean

win32:
	${MAKE} -f Makefile.w32

win32-clean:
	${MAKE} -f Makefile.w32 clean
