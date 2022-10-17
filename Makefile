TARGET = funkin
TYPE = ps-exe

SRCS = src/main.c \
	   \
	   \
       src/characters/bf/bf.c \
	   src/characters/bfe/bfe.c \
       src/characters/starvedtails/stails.c \
	   src/characters/bfn/bfn.c \
	   \
	   \
	   src/characters/gf/gf.c \
       src/characters/gf/speaker.c \
	   src/characters/gfn/gfn.c \
	   \
	   \
       src/characters/sonicexe/sonicexe.c \
       src/characters/ycn/ycn.c \
       src/characters/lordx/lordx.c \
       src/characters/nmouse/nmouse.c \
       src/characters/sanic/sanic.c \
	   \
	   \
	   \
       src/stages/gh/gh.c \
       src/stages/ycr/ycr.c \
       src/stages/trichael/trichael.c \
       src/stages/starved/starved.c \
	   \
	   \
       \
       src/menu/menu.c \
	   \
	   \
       src/stage/stage.c \
	   src/stage/animation.c \
       src/stage/character.c \
       src/stage/object.c \
       src/stage/object/combo.c \
       src/stage/object/splash.c \
       src/events/event.c \
	   \
	   \
	   \
       src/fonts/font.c \
	   \
       src/psx/mutil.c \
       src/psx/random.c \
       src/psx/archive.c \
       src/psx/trans.c \
       src/psx/loadscr.c \
       src/psx/psx.c \
       src/psx/io.c \
       src/psx/gfx.c \
       src/psx/audio.c \
       src/psx/pad.c \
       src/psx/timer.c \
       src/psx/movie.c \
       src/psx/save.c \
       mips/common/crt0/crt0.s

CPPFLAGS += -Wall -Wextra -pedantic -mno-check-zero-division
LDFLAGS += -Wl,--start-group
# TODO: remove unused libraries
LDFLAGS += -lapi
#LDFLAGS += -lc
LDFLAGS += -lc2
LDFLAGS += -lcard
LDFLAGS += -lcd
#LDFLAGS += -lcomb
LDFLAGS += -lds
LDFLAGS += -letc
LDFLAGS += -lgpu
#LDFLAGS += -lgs
#LDFLAGS += -lgte
#LDFLAGS += -lgun
#LDFLAGS += -lhmd
#LDFLAGS += -lmath
LDFLAGS += -lmcrd
#LDFLAGS += -lmcx
LDFLAGS += -lpad
LDFLAGS += -lpress
#LDFLAGS += -lsio
LDFLAGS += -lsnd
LDFLAGS += -lspu
#LDFLAGS += -ltap
LDFLAGS += -flto -Wl,--end-group

include mips/common.mk
