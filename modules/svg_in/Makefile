include ../../config.mak

vpath %.c $(SRC_PATH)/modules/svg_in

CFLAGS= $(OPTFLAGS) -I"$(SRC_PATH)/include"

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif

ifeq ($(GPROFBUILD), yes)
CFLAGS+=-pg
LDFLAGS+=-pg
endif

ifeq ($(CONFIG_ZLIB), local)
CFLAGS+= -I"$(LOCAL_INC_PATH)/zlib"
EXTRALIBS+=-L../../extra_lib/lib/gcc
endif

#common obj
OBJS= svg_in.o

SRCS := $(OBJS:.o=.c) 

LIB=gm_svg_in.$(DYN_LIB_SUFFIX)
ifeq ($(CONFIG_WIN32),yes)
#LDFLAGS+=-export-symbols svg_in.def 
endif


all: $(LIB)


$(LIB): $(OBJS)
	$(CC) $(SHFLAGS) $(LDFLAGS) -o ../../bin/gcc/$@ $(OBJS) $(EXTRALIBS) -L../../bin/gcc -lgpac -lz
ifeq ($(STATICBUILD),yes)
	$(CC) $(SHFLAGS) $(LDFLAGS) -o ../../bin/gcc/gm_svg_in-static.so $(OBJS) $(EXTRALIBS) -L../../bin/gcc -lgpac_static -lz
endif


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< 


clean: 
	rm -f $(OBJS) ../../bin/gcc/$(LIB)

dep: depend

depend:
	rm -f .depend	
	$(CC) -MM $(CFLAGS) $(SRCS) 1>.depend

distclean: clean
	rm -f Makefile.bak .depend



# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif
