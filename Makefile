ifdef ARCH

all: 
	${MAKE} -C pseint   -f Makefile ARCH=${ARCH}
	${MAKE} -C psexport -f Makefile ARCH=${ARCH}
	${MAKE} -C hoewrap  -f Makefile ARCH=${ARCH}
	${MAKE} -C wxPSeInt -f Makefile ARCH=${ARCH}
	${MAKE} -C pseval   -f Makefile ARCH=${ARCH}
	${MAKE} -C updatem  -f Makefile ARCH=${ARCH}
	${MAKE} -C psterm   -f Makefile ARCH=${ARCH}
	${MAKE} -C psdraw3  -f Makefile ARCH=${ARCH}
	${MAKE} -C psdrawE  -f Makefile ARCH=${ARCH}

clean: 
	${MAKE} -C pseint   -f Makefile ARCH=${ARCH} clean
	${MAKE} -C psexport -f Makefile ARCH=${ARCH} clean
	${MAKE} -C hoewrap  -f Makefile ARCH=${ARCH} clean
	${MAKE} -C wxPSeInt -f Makefile ARCH=${ARCH} clean
	${MAKE} -C pseval   -f Makefile ARCH=${ARCH} clean
	${MAKE} -C updatem  -f Makefile ARCH=${ARCH} clean
	${MAKE} -C psterm   -f Makefile ARCH=${ARCH} clean
	${MAKE} -C psdraw3  -f Makefile ARCH=${ARCH} clean
	${MAKE} -C psdrawE  -f Makefile ARCH=${ARCH} clean

else
all:
	@echo " "
	@echo "Para compilar directo (en un sistema para ese mismo sistema), usar:"
	@echo "   make ARCH=lnx             para compilar todo en GNU/Linux o macOS"
	@echo "   mingw32-make ARCH=w32     para compilar todo en Microsoft Windows"
	@echo " "
	@echo "Para compilar cruzado (en GNU/Linux, para otro sistema), usar:"
	@echo "   make ARCH=wine            para compilar todo para Microsoft Windows con wine"
	@echo "   make ARCH=m64             para compilar todo para macOS-ia64 con osxcross"
	@echo "   make ARCH=marm            para compilar todo para macOS-arm64 con osxcross"
	@echo " "
	@echo "Para generar los instaladores/paquetes finales, o el tgz con los fuentes:"
	@echo "   make -f Makefile.pack arq     (donde arq es src, lnx, w32, m64 o marm)" 
	@echo " "
	@echo "En caso de error, o para mas informacion, leer \"fuentes.txt\"."
	@echo " "

endif

