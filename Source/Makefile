# Makefile - compila deus_translate.so (plugin SA:MP para Linux, 32 bits)
#
# SA:MP en Linux es de 32 bits, por eso -m32. En un host de 64 bits necesitas
# el soporte multilib de g++:
#   Debian/Ubuntu:  sudo apt-get install g++-multilib
#   CentOS/RHEL:    sudo yum install glibc-devel.i686 libstdc++-devel.i686
#
# Uso:
#   make          compila -> ./deus_translate.so
#   make clean    limpia
#
# -DLINUX es OBLIGATORIO: plugincommon.h define PLUGIN_EXPORT/PLUGIN_CALL segun
# la plataforma y sin el define lanza "#error You must define WIN32/LINUX/FREEBSD".

CXX      := g++
OUTFILE  := deus_translate.so
BUILDDIR := build

CXXFLAGS := -m32 -fPIC -O2 -w -std=c++14 -fvisibility=hidden -DLINUX -Isdk
LDFLAGS  := -m32 -shared -fPIC

SRCS := src/main.cpp src/lang.cpp src/encoding.cpp sdk/amxplugin.cpp sdk/amxplugin2.cpp
OBJS := $(patsubst %.cpp,$(BUILDDIR)/%.o,$(SRCS))

all: $(OUTFILE)

$(BUILDDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUTFILE): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)
	@echo "[OK] $(OUTFILE) generado (32-bit)"

clean:
	rm -rf $(BUILDDIR) $(OUTFILE)

.PHONY: all clean
