include ../configs/config.${ARCH}
OBJS_DIR=../${TEMP}/updatem
ifdef WINDOWS
LDFLAGS+=-mwindows -lwsock32 
endif

all: ${OBJS_DIR} ../${BIN}/updatem${EXT}

include Makefile.common

clean:
	rm -rf ${OBJS} ../${BIN}/updatem${EXT}

../${BIN}/updatem${EXT}: ${OBJS}
	${GPP} ${OBJS} ${LDFLAGS} -o $@

${OBJS_DIR}:
	mkdir -p ${OBJS_DIR}
