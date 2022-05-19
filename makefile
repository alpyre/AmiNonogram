#******************************************************************************
#  This file is part of AmiNonogram.
#  Copyright (C) 2022 Ibrahim Alper Sönmez
#******************************************************************************
#  - for m68k OS3 binaries use: make
#  - for ppc OS4 binaries use: make OS=os4
#  - for ppc MorphOS binaries use: make OS=mos

EXE = AmiNonogram
################################################################################
# Target OS
ifndef (OS)
  OS = os3
endif

ifeq ($(OS), os3)
  CPU = -m68020
  CC = m68k-amigaos-gcc
  OPTIONS = -DNO_INLINE_STDARG
  LFLAGS = -s -noixemul -lamiga -lmui
else
ifeq ($(OS), os4)
  CPU = -mcpu=powerpc
  CC = ppc-amigaos-gcc
  OPTIONS = -DNO_INLINE_STDARG -D__USE_INLINE__
  LFLAGS = -lauto
else
ifeq ($(OS), mos)
  CPU = -mcpu=powerpc
  CC = ppc-morphos-gcc-5.5.0
  OPTIONS = -DNO_PPCINLINE_STDARG -I/opt/amiga/compilers/ppc-morphos/os-include
  LFLAGS = -s -noixemul
endif
endif
endif
################################################################################
# Common options
WARNINGS = -Wall
OPTIMIZE = -Os
DEBUG =
IDIRS = -Iincludes

CFLAGS = $(WARNINGS) $(OPTIMIZE) $(DEBUG) $(CPU) $(OPTIONS) $(IDIRS)

OBJS = utility.o bitfield.o pack.o profile.o integerdisplay.o sizeselector.o livesdisplay.o square.o nonogram.o hintdisplay.o dosupernew.o game.o main.o
################################################################################

# target 'all' (default target)
all : $(EXE)

# $@ matches the target; $< matches the first dependent
main.o : main.c
	$(CC) -c $< $(CFLAGS)

dusupernew.o : dusupernew.c
		$(CC) -c $< $(CFLAGS)

hintdisplay.o : hintdisplay.c
	$(CC) -c $< $(CFLAGS)

nonogram.o : nonogram.c
	$(CC) -c $< $(CFLAGS)

square.o : square.c
	$(CC) -c $< $(CFLAGS)

livesdisplay.o : livesdisplay.c
	$(CC) -c $< $(CFLAGS)

integerdisplay.o : integerdisplay.c
	$(CC) -c $< $(CFLAGS)

$(EXE) : $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(LFLAGS)

# target 'clean'
clean:
	rm -f $(EXE)
	rm -f $(OBJS)
