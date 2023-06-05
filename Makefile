TARGET  = gngeo
  CROSS   = /opt/miyoo/bin/arm-miyoo-linux-uclibcgnueabi-
CC      = $(CROSS)gcc
CXX     = $(CROSS)g++
LD      = $(CROSS)gcc
STRIP   = $(CROSS)strip
CFLAGS += `/opt/miyoo/arm-miyoo-linux-uclibcgnueabi/sysroot/usr/bin/sdl-config --cflags`
#CFLAGS += -ggdb
CFLAGS += -D_GNU_SOURCE=1 -D_REENTRANT -DARM
CFLAGS += -fstrength-reduce -frerun-loop-opt -funroll-loops -ffast-math -fexpensive-optimizations -fomit-frame-pointer -fno-strict-aliasing -O3
LDFLAGS+= `/opt/miyoo/arm-miyoo-linux-uclibcgnueabi/sysroot/usr/bin/sdl-config --libs` -lSDL_image
#LDFLAGS+= `/opt/miyoo/arm-miyoo-linux-uclibcgnueabi/sysroot/usr/bin/sdl-config --libs` -lSDL_image -lSDL_gfx
LDFLAGS+= -lm -lz
OBJS    = \
  src/messages.o \
  src/drv.o \
  src/conf.o \
  src/mame_layer.o \
  src/cyclone.o \
  src/neocrypt.o \
  src/drz80.o \
  src/state.o \
  src/frame_skip.o \
  src/sound.o \
  src/video.o \
  src/memory.o \
  src/ym2610.o \
  src/neoboot.o \
  src/list.o \
  src/cyclone_interf.o \
  src/pd4990a.o \
  src/ym2610_interf.o \
  src/menu.o \
  src/gnutil.o \
  src/unzip.o \
  src/drz80_interf.o \
  src/screen.o \
  src/interp.o \
  src/emu.o \
  src/event.o \
  src/main.o \
  src/video_arm.o \
  src/roms.o \
  src/timer.o \
  src/rumble.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@
	$(STRIP) $(TARGET)

%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s 
	$(CXX) $(ASFLAGS) $(CXXFLAGS) -c $< -o $@

%.o: %.S 
	$(CXX) $(ASFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) $(TARGET)
