##=======================================================
## Makefile for AlphaDict.                                
##                                                        
##
##=======================================================

##=== funtions ===##
# tree -d -L 1  -i
FUN_LOCAL_DIRS = $(shell ls $(1) -lA | grep ^d  | awk '{print $$9}')

include config.mak

# GUI/QML also travel through config.h (CONFIG_QT5, CONFIG_QML).  Overriding
# them on the make command line ("make GUI=no") does not recompile the objects
# that were built with the other setting, so the resulting link mixes two
# configurations.  Say so instead of failing later with an undefined symbol.
ifeq ($(GUI),no)
ifneq ($(wildcard config.h),)
_CFG_GUI_WORD := $(shell grep -c '^#define CONFIG_QT5' config.h 2>/dev/null || echo 0)
ifneq ($(_CFG_GUI_WORD),0)
$(warning config.h still defines CONFIG_QT5 but GUI=no was requested; run "make clean" first to rebuild a matching tree)
endif
endif
endif
ifeq ($(QML),no)
ifneq ($(wildcard config.h),)
_CFG_QML_WORD := $(shell grep -c '^#define CONFIG_QML' config.h 2>/dev/null || echo 0)
ifneq ($(_CFG_QML_WORD),0)
$(warning config.h still defines CONFIG_QML but QML=no was requested; run "make clean" first to rebuild a matching tree)
endif
endif
endif

subdirs := tools lib dict db gui
incs := $(foreach dir, $(subdirs), $(dir)/Makefile.inc)
include $(incs)

SRCS_ALPHADICT += main.cpp    \
             Log.cpp          \
             MutexLock.cpp    \
             SpinLock.cpp     \
             ThdCond.cpp      \
             Thread.cpp       \
             Configure.cpp    \
             Util.cpp         \
             CharUtil.cpp     \
             X11Util.cpp      \
             MessageQueue.cpp \
             SysMessager.cpp  \
             TaskManager.cpp  \
             DictManager.cpp  \
             VocabularyBook.cpp \
             Application.cpp  \
             $(SRCS_DICT-yes) \
			 $(SRCS_GUI_$(GUI)) \
			 $(SRCS_QML_$(QML)) \
			 $(SRCS_DB-yes) \

OBJS_TINYXML := $(SRCS_TINYXML:%.cpp=%.o)

OBJS_ALPHADICT := $(SRCS_ALPHADICT:%.cpp=%.o)
OBJS_ALPHADICT += $(SRCS_SQLITE_$(SQLITE_BUNDLED):%.c=%.o)
OBJS_ALDCTL := $(SRCS_ALDCTL:%.cpp=%.o)
OBJS_ALPHADICT += lib/tinyxml2/libtinyxml2.a
# Static libraries must follow the objects that use them, or the linker
# drops them and reports undefined symbols from Util.o.
OBJS_ALDCTL += Util.o CharUtil.o Log.o SpinLock.o MutexLock.o \
                dict/aldict/AldictDocument.o lib/tinyxml2/libtinyxml2.a

DEPS := $(SRCS_ALPHADICT:%.cpp=%.d)
DEPS += $(SRCS_SQLITE_$(SQLITE_BUNDLED):%.c=%.d)
DEPS += $(SRCS_ALDCTL:%.cpp=%.d)

EXTRALIBS_ALPHADICT += $(EXTRALIBS)

# Without a GUI there is no Qt program to build or install; only the
# command line dictionary converter remains.
PROGS_GUI_yes = AlphaDict
INSTALL_GUI_yes = install-AlphaDict
UITR_yes = uitr-qm

INSTALL_TARGETS-yes = install-system install-aldctl $(INSTALL_GUI_$(GUI))

DIRS = . tools dict/aldict gui/qt gui/qml db

ADDSUFFIXES     = $(foreach suf,$(1),$(addsuffix $(suf),$(2)))
ADD_ALL_DIRS    = $(call ADDSUFFIXES,$(1),$(DIRS))
ADD_ALL_EXESUFS = $(1) $(call ADDSUFFIXES,$(EXESUFS_ALL),$(1))

# English is the source language, so only the Chinese catalogue is built.
# "make uitr" (gui/qt/qt.mk) also runs lupdate; the build target only compiles
# the checked-in .ts, so packaging never rewrites a tracked file.
uitr-qm:
	@if command -v lrelease >/dev/null 2>&1; then \
	    lrelease gui/qt/uitr_cn.ts -qm gui/qt/uitr_cn.qm; \
	else \
	    echo "note: lrelease not found, UI translation not built"; \
	fi

a11: aldctl $(PROGS_GUI_$(GUI)) $(UITR_$(GUI))

lib/tinyxml2/libtinyxml2.a: $(OBJS_TINYXML)
	$(AR) -r $@  $^

AlphaDict: $(QT_H_$(GUI)) $(OBJS_ALPHADICT) $(UITR_$(GUI))
	$(CC)  $(OBJS_ALPHADICT) $(EXTRALIBS_ALPHADICT) -lstdc++  -o $@

# The command line tool and the conversion scripts share tools/.  It is also
# installed to <datadir>/tools so that the scripts can find it there.
aldctl: $(OBJS_ALDCTL)
	$(CC) $^ $(EXTRALIBS) -lstdc++ -o tools/aldctl

# ---- dictionaries --------------------------------------------------------
# Dictionaries are data, not build products.  Every *.aldict dropped into
# ../system/dicts/ is installed to <datadir>/dicts/ by 'make install', and
# aldctl is copied next to the program data as well.  The directory also
# holds the user's own dictionaries, so it is never touched by 'make clean'.
dict:
	@echo "note: dictionaries are data in ../system/dicts/*.aldict; nothing to build"

#%.d : %.cpp
#	$(CC) -MM $< -o $@

# dpkg-buildflags puts -Werror=implicit-function-declaration in CFLAGS, but it
# is a C-only diagnostic: GCC 14 warns that the option is invalid for C++ when
# the same CFLAGS reach cc1plus.  Drop it for C++ sources only; every other
# flag (include paths, Qt defines, hardening) still applies.
CXX_ALLFLAGS = $(filter-out -Werror=implicit-function-declaration,$(CFLAGS)) $(CXXFLAGS)

%.o : %.cpp
	$(CXX) $(CXX_ALLFLAGS) -c $< -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

# The bundled SQLite amalgamation is third-party code; its diagnostics are not
# actionable here and would otherwise drown out warnings from AlphaDict itself.
lib/sqlite3/sqlite3.o : lib/sqlite3/sqlite3.c
	$(CC) $(CFLAGS) -w -c $< -o $@

-include $(DEPS)

-include gui/qt/qt.mk

###### installation / clean / generic rules #######

install: $(INSTALL_TARGETS-yes)

install-dirs:
	$(INSTALL) -d $(BINDIR) $(DATADIR) $(DATADIR)/dicts $(DATADIR)/doc $(DATADIR)/tools

install-%: % install-dirs
	$(INSTALL) -m 755 $(INSTALLSTRIP) $< $(BINDIR)

install-aldctl: aldctl install-dirs
	$(INSTALL) -m 755 $(INSTALLSTRIP) tools/aldctl $(BINDIR)

# Copying the tool next to the scripts keeps <datadir>/tools usable on its own.
install-tools: aldctl install-dirs
	$(INSTALL) -m 755 $(INSTALLSTRIP) tools/aldctl "$(DATADIR)/tools/aldctl"
	@for FILE in tools/*.py; do \
	    install -m 0755 "$$FILE" "$(DATADIR)/tools/$$(basename $$FILE)"; done

install-system: install-dirs install-tools
	install -m 0666 "../system/configure.xml.in"   "$(DATADIR)/configure.xml.in"
	install -m 0666 "../system/language.txt"       "$(DATADIR)/language.txt"
	install -m 0666  ../system/help.*              "$(DATADIR)/"
	@if [ "$(GUI)" = yes ]; then \
	    if [ -f "gui/qt/uitr_cn.qm" ]; then \
	        install -m 0666 "gui/qt/uitr_cn.qm"    "$(DATADIR)/uitr_cn.qm"; \
	    else \
	        echo "note: gui/qt/uitr_cn.qm not built, run 'make uitr' to ship translations"; fi; \
	    install -m 0666 "gui/qt/res/appicon.png"   "$(DATADIR)/AlphaDict.png"; fi
	install -m 0666 "../doc/dictionary_inner.pdf"  "$(DATADIR)/doc/dictionary_inner.pdf"
	install -m 0666 "../doc/front-end_format.txt"  "$(DATADIR)/doc/front-end_format.txt"
	install -m 0666 "../doc/make_dictionary.txt"  "$(DATADIR)/doc/make_dictionary.txt"
	@for f in ../system/dicts/*.aldict; do \
	    [ -e "$$f" ] || continue; \
	    install -m 0666 "$$f" "$(DATADIR)/dicts/$$(basename $$f)"; done

uninstall:
	rm -f $(BINDIR)/AlphaDict $(BINDIR)/aldctl
	rm -f $(DATADIR)/configure.xml.in $(DATADIR)/language.txt
	rm -f $(DATADIR)/help.* $(DATADIR)/uitr_cn.qm $(DATADIR)/AlphaDict.png
	rm -f $(DATADIR)/dicts/*.aldict $(DATADIR)/aldctl
	rm -rf $(DATADIR)/tools $(DATADIR)/doc

clean:
	@find lib -name "*.[oadi]" | xargs -I{} rm {}
	@find gui/qt -name "moc_*" | xargs -I{} rm {}
	@find gui/qt -name "qrc_*" | xargs -I{} rm {}
	@find gui/qt -name "ui_*.h" | xargs -I{} rm {}
	-rm -f gui/qt/uitr_cn.qm
	@find gui/qml -name "qrc_*" | xargs -I{} rm {}
	-rm -f $(call ADD_ALL_DIRS,/*.o /*.d /*.a /*.i /*~)
	-rm -f $(call ADD_ALL_EXESUFS, AlphaDict aldctl)
	-rm -f tools/aldctl

distclean: clean
	-@rm -rf config.*  gui/build-qt-Desktop-Debug  gui/build-qt-Desktop-Release

.PHONY : install uninstall clean distclean dict uitr-qm
