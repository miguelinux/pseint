all:
	@echo " "
	@echo usar:
	@echo "   make linux           para compilar todo en GNU/Linux o Mac OS"
	@echo "   mingw32-make win32   para compilar todo en Microsoft Windows"
	@echo En caso de error, leer \"fuentes.txt\".
	@echo " "

linux:
	${MAKE} -f Makefile.lnx

linux-clean:
	${MAKE} -f Makefile.lnx clean

win32:
	${MAKE} -f Makefile.w32

win32-clean:
	${MAKE} -f Makefile.w32 clean
