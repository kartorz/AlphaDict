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

subdirs := convert lib dict db gui
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
OBJS_ALCONVERT := $(SRCS_ALCONVERT:%.cpp=%.o)
OBJS_ALPHADICT += lib/tinyxml2/libtinyxml2.a
OBJS_ALCONVERT += lib/tinyxml2/libtinyxml2.a \
                  Util.o CharUtil.o

DEPS := $(SRCS_ALPHADICT:%.cpp=%.d)
DEPS += $(SRCS_SQLITE_$(SQLITE_BUNDLED):%.c=%.d)
DEPS += $(SRCS_ALCONVERT:%.cpp=%.d)

EXTRALIBS_ALPHADICT += $(EXTRALIBS)

# Without a GUI there is no Qt program to build or install; only the
# command line dictionary converter remains.
PROGS_GUI_yes = AlphaDict
INSTALL_GUI_yes = install-AlphaDict

INSTALL_TARGETS-yes = install-system install-AldConvert $(INSTALL_GUI_$(GUI))

DIRS = . convert dict/aldict gui/qt gui/qml db

ADDSUFFIXES     = $(foreach suf,$(1),$(addsuffix $(suf),$(2)))
ADD_ALL_DIRS    = $(call ADDSUFFIXES,$(1),$(DIRS))
ADD_ALL_EXESUFS = $(1) $(call ADDSUFFIXES,$(EXESUFS_ALL),$(1))

a11: AldConvert $(PROGS_GUI_$(GUI)) dict

lib/tinyxml2/libtinyxml2.a: $(OBJS_TINYXML)
	$(AR) -r $@  $^

AlphaDict: $(QT_H_$(GUI)) $(OBJS_ALPHADICT)
	$(CC)  $(OBJS_ALPHADICT) $(EXTRALIBS_ALPHADICT) -lstdc++  -o $@

AldConvert: $(OBJS_ALCONVERT)
	$(CC) $^ $(EXTRALIBS) -lstdc++ -o $@

# ---- sample dictionary ---------------------------------------------------
# A fresh checkout has no dictionary, so the GUI would always show an empty
# index. Build the small sample that ships in system/dicts/ with AldConvert;
# install-system puts it next to the program data. A real dictionary can
# replace it at any time: the program scans every file in <datadir>/dicts/.
DICT_SRC  = system/dicts/en-ch.xml
DICT_OUT  = system/dicts/en-ch.aldict
DICT_BASE = system/dicts/en-ch

dict: $(DICT_OUT)

$(DICT_OUT): $(DICT_SRC) AldConvert
	@mkdir -p `dirname $(DICT_OUT)`
	./AldConvert -o $(DICT_BASE) $(DICT_SRC)

#%.d : %.cpp
#	$(CC) -MM $< -o $@

%.o : %.cpp
	$(CC) $(CFLAGS) $(CXXFLAGS)  -c $< -o $@

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
	$(INSTALL) -d $(BINDIR) $(DATADIR) $(DATADIR)/dicts $(DATADIR)/xmls $(DATADIR)/doc

install-%: % install-dirs
	$(INSTALL) -m 755 $(INSTALLSTRIP) $< $(BINDIR)

install-system: install-dirs
	install -m 0666 "system/configure.xml.in"      "$(DATADIR)/configure.xml.in"
	install -m 0666 "system/language.txt"          "$(DATADIR)/language.txt"
	install -m 0666  system/help.*                 "$(DATADIR)/"
	@if [ "$(GUI)" = yes ]; then \
	    if [ -f "gui/qt/uitr_cn.qm" ]; then \
	        install -m 0666 "gui/qt/uitr_cn.qm"    "$(DATADIR)/uitr_cn.qm"; \
	    else \
	        echo "note: gui/qt/uitr_cn.qm not built, run 'make uitr' to ship translations"; fi; \
	    install -m 0666 "gui/qt/res/appicon.png"   "$(DATADIR)/AlphaDict.png"; fi
	install -m 0666 "../doc/dictionary_inner.pdf"  "$(DATADIR)/doc/dictionary_inner.pdf"
	install -m 0666 "../doc/front-end_format.xml"  "$(DATADIR)/doc/front-end_format.xml"
	install -m 0666 "../doc/alphadict_en-ch.xml"   "$(DATADIR)/doc/alphadict_en-ch.xml"
	@if [ -f "$(DICT_OUT)" ]; then \
	    install -m 0666 "$(DICT_OUT)" "$(DATADIR)/dicts/en-ch.aldict"; fi

	$(INSTALL) -d  $(DATADIR)/tools
	@for FILE in `ls tools`; do \
	    install -m 0755 "tools/$$FILE" "$(DATADIR)/tools/$$FILE"; done

uninstall:
	rm -f $(BINDIR)/AlphaDict $(BINDIR)/AldConvert
	rm -f $(DATADIR)/configure.xml.in $(DATADIR)/language.txt
	rm -f $(DATADIR)/help.* $(DATADIR)/uitr_cn.qm $(DATADIR)/AlphaDict.png
	rm -f $(DATADIR)/dicts/en-ch.aldict
	rm -rf $(DATADIR)/tools $(DATADIR)/doc

clean:
	@find lib -name "*.[oadi]" | xargs -I{} rm {}
	@find gui/qt -name "moc_*" | xargs -I{} rm {}
	@find gui/qt -name "qrc_*" | xargs -I{} rm {}
	@find gui/qt -name "ui_*.h" | xargs -I{} rm {}
	-rm -f gui/qt/uitr_cn.qm
	@find gui/qml -name "qrc_*" | xargs -I{} rm {}
	-rm -f $(call ADD_ALL_DIRS,/*.o /*.d /*.a /*.i /*~)
	-rm -f $(call ADD_ALL_EXESUFS, AlphaDict AldConvert)
	-rm -f system/dicts/*.aldict

distclean: clean
	-@rm -rf config.*  gui/build-qt-Desktop-Debug  gui/build-qt-Desktop-Release

.PHONY : install uninstall clean distclean dict 
