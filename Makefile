###
# Boruvka
# --------
# Copyright (c)2010-2012 Daniel Fiser <danfis@danfis.cz>
#
#  This file is part of boruvka.
#
#  Distributed under the OSI-approved BSD License (the "License");
#  see accompanying file BDS-LICENSE for details or see
#  <http://www.opensource.org/licenses/bsd-license.php>.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even the
#  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#  See the License for more information.
##

-include Makefile.local
-include Makefile.include

SO_VERSION = 0

CFLAGS += -I.
CXXFLAGS += -I.
LDFLAGS += -L. -lboruvka -lm -lrt

TARGETS  = libboruvka.a

OBJS  = alloc
OBJS += cfg cfg-lexer
OBJS += opts
OBJS += varr
OBJS += quat vec4 vec3 vec2 vec
OBJS += mat4 mat3
OBJS += predicates
OBJS += sort
OBJS += pc pc-internal
OBJS += gug
OBJS += nearest-linear
OBJS += vptree
OBJS += vptree-hamming
OBJS += nn-linear
OBJS += mesh3 net qhull chull3
OBJS += fibo pairheap dij
OBJS += bucketheap
OBJS += tasks task-pool
OBJS += hfunc
OBJS += htable
OBJS += google-city-hash
OBJS += barrier
OBJS += rand-mt rand-mt-parallel
OBJS += timer
OBJS += parse
OBJS += image
OBJS += segmarr
OBJS += rbtree

ifeq '$(USE_OPENCL)' 'yes'
  OBJS += opencl
  CFLAGS  += $(OPENCL_CFLAGS)
  LDFLAGS += $(OPENCL_LDFLAGS)
endif

ifeq '$(USE_HDF5)' 'yes'
  OBJS += hdf5
  CFLAGS  += $(HDF5_CFLAGS)
  LDFLAGS += $(HDF5_LDFLAGS)
endif

ifeq '$(USE_GSL)' 'yes'
  CFLAGS  += $(GSL_CFLAGS)
  LDFLAGS += $(GSL_LDFLAGS)
endif


BIN_TARGETS = bor-qdelaunay


OBJS_PIC        := $(foreach obj,$(OBJS),.objs/$(obj).pic.o)
OBJS 		    := $(foreach obj,$(OBJS),.objs/$(obj).o)
BIN_TARGETS     := $(foreach target,$(BIN_TARGETS),bin/$(target))


ifeq '$(BINS)' 'yes'
  TARGETS += $(BIN_TARGETS)
endif

ifeq '$(DYNAMIC)' 'yes'
  TARGETS += libboruvka.so
endif

all: $(TARGETS)

libboruvka.a: $(OBJS)
	ar cr $@ $(OBJS)
	ranlib $@

libboruvka.so: libboruvka.so.$(SO_VERSION)
	ln -s $< $@

libboruvka.so.$(SO_VERSION): $(OBJS_PIC)
	$(CC) $(CFLAGS) -shared -Wl,-soname,$@ -o $@ $(OBJS_PIC)

boruvka/config.h: boruvka/config.h.m4
	$(M4) $(CONFIG_FLAGS) $< >$@

bin/bor-%: bin/%-main.c libboruvka.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
bin/%.o: bin/%.c bin/%.h
	$(CC) $(CFLAGS) -c -o $@ $<

examples/%: examples/%.c libboruvka.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

src/cfg-lexer.c: src/cfg-lexer.l src/cfg-lexer.h
	$(FLEX) -f -t $< >$@

.objs/cfg.pic.o: src/cfg.c boruvka/cfg.h boruvka/config.h src/cfg-lexer.c
	$(CC) -fPIC $(CFLAGS) -c -o $@ $<
.objs/%.pic.o: src/%.c boruvka/%.h boruvka/config.h
	$(CC) -fPIC $(CFLAGS) -c -o $@ $<
.objs/%.pic.o: src/%.c boruvka/config.h
	$(CC) -fPIC $(CFLAGS) -c -o $@ $<

.objs/cfg.o: src/cfg.c boruvka/cfg.h boruvka/config.h src/cfg-lexer.c
	$(CC) $(CFLAGS) -c -o $@ $<
.objs/%.o: src/%.c boruvka/%.h boruvka/config.h
	$(CC) $(CFLAGS) -c -o $@ $<
.objs/%.o: src/%.c boruvka/config.h
	$(CC) $(CFLAGS) -c -o $@ $<


%.h: boruvka/config.h
%.c: boruvka/config.h

%-cl.c: %.cl
	$(PYTHON) ./scripts/cl-to-c.py opencl_program <$< >$@
src/cd-sap-gpu-cl.c: src/cd-sap-gpu.cl
	$(PYTHON) ./scripts/cl-to-c.py opencl_program <$< >$@


install:
	mkdir -p $(PREFIX)/$(INCLUDEDIR)/boruvka
	mkdir -p $(PREFIX)/$(LIBDIR)
	mkdir -p $(PREFIX)/$(BINDIR)
	cp boruvka/*.h $(PREFIX)/$(INCLUDEDIR)/boruvka/
	cp -f libboruvka.a $(PREFIX)/$(LIBDIR)/
	cp -f libboruvka.so.$(SO_VERSION) $(PREFIX)/$(LIBDIR)/
	ln -s $(PREFIX)/$(LIBDIR)/libboruvka.so.$(SO_VERSION) $(PREFIX)/$(LIBDIR)/libboruvka.so
	cp -f $(BIN_TARGETS) $(PREFIX)/$(BINDIR)/

clean:
	rm -f $(OBJS)
	rm -f src/cfg-lexer.c src/cfg-lexer-gen.h
	rm -f .objs/*.o
	rm -f $(TARGETS)
	rm -f $(BIN_TARGETS)
	rm -f libboruvka.so
	rm -f libboruvka.so.*
	rm -f boruvka/config.h
	rm -f src/*-cl.c
	if [ -d testsuites ]; then $(MAKE) -C testsuites clean; fi;
	if [ -d doc ]; then $(MAKE) -C doc clean; fi;
	
check:
	$(MAKE) -C testsuites check
check-valgrind:
	$(MAKE) -C testsuites check-valgrind

doc:
	$(MAKE) -C doc

analyze: clean
	$(SCAN_BUILD) $(MAKE)

help:
	@echo "Targets:"
	@echo "    all            - Build library"
	@echo "    doc            - Build documentation"
	@echo "    check          - Build & Run automated tests"
	@echo "    check-valgrind - Build & Run automated tests in valgrind(1)"
	@echo "    clean          - Remove all generated files"
	@echo "    install        - Install library into system"
	@echo "    analyze        - Performs static analysis using Clang Static Analyzer"
	@echo ""
	@echo "Options:"
	@echo "    CC         - Path to C compiler          (=$(CC))"
	@echo "    CXX        - Path to C++ compiler        (=$(CXX))"
	@echo "    M4         - Path to m4 macro processor  (=$(M4))"
	@echo "    SED        - Path to sed(1)              (=$(SED))"
	@echo "    PYTHON     - Path to python interpret    (=$(PYTHON))"
	@echo "    PYTHON2    - Path to python interpret v2 (=$(PYTHON2))"
	@echo "    PYTHON3    - Path to python interpret v3 (=$(PYTHON3))"
	@echo "    SCAN_BUILD - Path to scan-build          (=$(SCAN_BUILD))"
	@echo ""
	@echo "    DYNAMIC   'yes'/'no' - Set to 'yes' if also .so library should be built (=$(DYNAMIC))"
	@echo ""
	@echo "    BINS      'yes'/'no' - Set to 'yes' if binaries should be built (=$(BINS))"
	@echo ""
	@echo "    CC_NOT_GCC 'yes'/'no' - If set to 'yes' no gcc specific options will be used (=$(CC_NOT_GCC))"
	@echo ""
	@echo "    DEBUG      'yes'/'no' - Turn on/off debugging          (=$(DEBUG))"
	@echo "    PROFIL     'yes'/'no' - Compiles profiling info        (=$(PROFIL))"
	@echo "    NOWALL     'yes'/'no' - Turns off -Wall gcc option     (=$(NOWALL))"
	@echo "    NOPEDANTIC 'yes'/'no' - Turns off -pedantic gcc option (=$(NOPEDANTIC))"
	@echo ""
	@echo "    USE_SINGLE   'yes'       - Use single precision  (=$(USE_SINGLE))"
	@echo "    USE_DOUBLE   'yes'       - Use double precision  (=$(USE_DOUBLE))"
	@echo "    USE_MEMCHECK 'yes'/'no'  - Use memory checking during allocation (=$(USE_MEMCHECK))"
	@echo "    USE_SSE      'yes'/'no'  - Use SSE instructions  (=$(USE_SSE))"
	@echo "    USE_OPENCL   'yes'/'no'  - Use OpenCL library    (=$(USE_OPENCL))"
	@echo "                               By default, auto detection is used."
	@echo "                               This option depends on USE_SINGLE set to 'yes'"
	@echo "    USE_HDF5     'yes'/'no'  - Use HDF5 data format  (=$(USE_HDF5))"
	@echo "    USE_GSL      'yes'/'no'  - Use GNU Scientific Library (=$(USE_GSL))"
	@echo ""
	@echo "    PREFIX     - Prefix where library will be installed                             (=$(PREFIX))"
	@echo "    INCLUDEDIR - Directory where header files will be installed (PREFIX/INCLUDEDIR) (=$(INCLUDEDIR))"
	@echo "    LIBDIR     - Directory where library will be installed (PREFIX/LIBDIR)          (=$(LIBDIR))"
	@echo "    BINDIR     - Directory where binaries will be installed (PREFIX/BINDIR)         (=$(BINDIR))"
	@echo ""
	@echo "Variables:"
	@echo "  Note that most of can be preset or changed by user"
	@echo "    SYSTEM            = $(SYSTEM)"
	@echo "    CFLAGS            = $(CFLAGS)"
	@echo "    CXXFLAGS          = $(CXXFLAGS)"
	@echo "    LDFLAGS           = $(LDFLAGS)"
	@echo "    CONFIG_FLAGS      = $(CONFIG_FLAGS)"
	@echo "    PYTHON_CFLAGS     = $(PYTHON_CFLAGS)"
	@echo "    PYTHON_LDFLAGS    = $(PYTHON_LDFLAGS)"
	@echo "    OPENCL_CFLAGS     = $(OPENCL_CFLAGS)"
	@echo "    OPENCL_LDFLAGS    = $(OPENCL_LDFLAGS)"
	@echo "    HDF5_CFLAGS       = $(HDF5_CFLAGS)"
	@echo "    HDF5_LDFLAGS      = $(HDF5_LDFLAGS)"
	@echo "    GSL_CFLAGS       = $(GSL_CFLAGS)"
	@echo "    GSL_LDFLAGS      = $(GSL_LDFLAGS)"

.PHONY: all clean check check-valgrind help doc install analyze examples
