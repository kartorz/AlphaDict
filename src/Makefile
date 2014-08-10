##=======================================================
## Makefile for AlphaDict.                                
##                                                        
##
##=======================================================

##=== funtions ===##
# tree -d -L 1  -i
FUN_LOCAL_DIRS = $(shell ls $(1) -lA | grep ^d  | awk '{print $$9}')

include config.mak

subdirs := convert lib dict gui
incs := $(foreach dir, $(subdirs), $(dir)/Makefile.inc)
include $(incs)

SRCS_ALPHADICT += main.cpp    \
             Log.cpp          \
             MutexLock.cpp    \
             SpinLock.cpp     \
             ThdCond.cpp      \
             Thread.cpp       \
             Util.cpp         \
             CharUtil.cpp     \
             X11Util.cpp      \
             MessageQueue.cpp \
             SysMessager.cpp  \
             TaskManager.cpp  \
             DictManager.cpp  \
             Configure.cpp    \
             VocabularyBook.cpp \
             Application.cpp  \
             $(SRCS_DICT-yes) \
	         $(SRCS_GUI-yes)  \


OBJS_TINYXML := $(SRCS_TINYXML:%.cpp=%.o)

OBJS_ALPHADICT := $(SRCS_ALPHADICT:%.cpp=%.o)
OBJS_ALCONVERT := $(SRCS_ALCONVERT:%.cpp=%.o)
OBJS_ALPHADICT += lib/tinyxml2/libtinyxml2.a
OBJS_ALCONVERT += lib/tinyxml2/libtinyxml2.a \
                  Util.o CharUtil.o

DEPS := $(SRCS_ALPHADICT:%.cpp=%.d)
DEPS += $(SRCS_ALCONVERT:%.cpp=%.d)

EXTRALIBS_ALPHADICT += $(EXTRALIBS)

INSTALL_TARGETS-yes = install-system install-AlphaDict install-AlConvert

DIRS = . convert lib \
         dict dict/aldict \
         gui  gui/qt 

ADDSUFFIXES     = $(foreach suf,$(1),$(addsuffix $(suf),$(2)))
ADD_ALL_DIRS    = $(call ADDSUFFIXES,$(1),$(DIRS))
ADD_ALL_EXESUFS = $(1) $(call ADDSUFFIXES,$(EXESUFS_ALL),$(1))

a11: AlphaDict AlConvert

lib/tinyxml2/libtinyxml2.a: $(OBJS_TINYXML)
	$(AR) -r $@  $<

AlphaDict: $(OBJS_ALPHADICT)
	$(CC) $^ $(EXTRALIBS_ALPHADICT) -lstdc++  -o $@

AlConvert: $(OBJS_ALCONVERT)
	$(CC) $^ $(EXTRALIBS) -lstdc++ -o $@ 

#%.d : %.cpp
#	$(CC) -MM $< -o $@

%.o : %.cpp
	$(CC) $(CFLAGS) $(CXXFLAGS)  -c $< -o $@

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
	install -m 0666 "gui/qt/uitr_cn.qm"            "$(DATADIR)/uitr_cn.qm"
	install -m 0666 "../doc/dictionary_inner.pdf"  "$(DATADIR)/doc/dictionary_inner.pdf"
	install -m 0666 "../doc/front-end_format.xml"  "$(DATADIR)/doc/front-end_format.xml"
	install -m 0666 "../doc/alphadict_en-ch.xml"   "$(DATADIR)/doc/alphadict_en-ch.xml"

	install -m 0666 ../doc/help.*                  "$(DATADIR)/doc/"

	$(INSTALL) -d  $(DATADIR)/tools
	@for FILE in `ls tools`; do \
	    install -m 0755 "tools/$$FILE" "$(DATADIR)/tools/$$FILE"; done

uninstall:
	rm -f $(BINDIR)/AlphaDict $(BINDIR)/AlConvert
	rm -f $(DATADIR)/configure.xml.in $(DATADIR)/language.txt
	rm -f $(DATADIR)/tools -r

clean:
#	@find . -name "*.[oadi]" | xargs -I{} rm {}
	-rm -f $(call ADD_ALL_DIRS,/*.o /*.d /*.a /*.i /*~)
	-rm -f $(call ADD_ALL_EXESUFS, AlphaDict AlConvert)

distclean: clean
	-@rm -rf config.*  gui/build-qt-Desktop-Debug  gui/build-qt-Desktop-Release

.PHONY : install uninstall clean distclean 
