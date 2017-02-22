#
# Generic Makefile for TvGuideFetch
#

# Customisable settings
CONFTOOL =	perl
CONFSCRIPT =	make-config-from-file.pl
INSTALL =	install -m 755 -s
CFLAGS =	-Os -Wall

# Build specification
TARGET =	tv_grab_au_tvguide
GENS =		configEmpty.cpp configFreeview.cpp \
			configAnalogue.cpp configFoxtel.cpp
OBJS =		TvGuideFetch.o XmlDom.o Config.o HttpFetch.o \
			CmdOptions.o Channel.o EasyDate.o $(GENS:.cpp=.o)
DEPS =		$(OBJS:.o=.d)

# Flags needed for libexpat
LIBS +=		-lexpat

# Flags needed for libcurl
LIBS +=		-lcurl -lssl -lcrypto

# Flags needed for zlib
LIBS +=		-lz

# Standard flags
CFLAGS +=	-MMD -MP -D_GNU_SOURCE
CXXFLAGS =	$(CFLAGS)

.PHONY:		all install clean clobber

# Targets
all:	$(TARGET)

$(TARGET):	$(OBJS)
	$(CXX) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

install:	$(TARGET)
	$(INSTALL) $(TARGET) /usr/bin/tv_grab_au_tvguide

clean:
	$(RM) $(OBJS) $(DEPS) $(GENS)

clobber:	clean
	$(RM) $(TARGET)

# pull in dependency info for *existing* .o files
-include $(DEPS)

# Generate default configuration
configEmpty.cpp: configEmpty.xml $(CONFSCRIPT) Makefile
	$(CONFTOOL) $(CONFSCRIPT) Empty configEmpty.xml >$@

configFreeview.cpp: configFreeview.xml $(CONFSCRIPT) Makefile
	$(CONFTOOL) $(CONFSCRIPT) Freeview configFreeview.xml >$@

configAnalogue.cpp: configAnalogue.xml $(CONFSCRIPT) Makefile
	$(CONFTOOL) $(CONFSCRIPT) Analogue configAnalogue.xml >$@

configFoxtel.cpp: configFoxtel.xml $(CONFSCRIPT) Makefile
	$(CONFTOOL) $(CONFSCRIPT) Foxtel configFoxtel.xml >$@

