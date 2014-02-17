##=======================================================
## Makefile for AlphaDict.                                
##                                                        
## Every subdirctory should hava a Makefile.inc defining 
## module.
##=======================================================

##=== funtions ===##
# tree -d -L 1  -i
FUN_LOCAL_DIRS = $(shell ls $(1) -lA | grep ^d  | awk '{print $$9}')

include config.mk

SOURCES_ALPHADICT :=
subdirs := convert lib dict

incs := $(foreach dir, $(subdirs), $(dir)/Makefile.inc)

include $(incs)

SOURCES_ALPHADICT += $(foreach module, $(MODULE.yes), SOURCES_$(module))

SOURCES_ALPHADICT += main.cpp \
		     Log.cpp

OBJS_TINYXML := $(SOURCS_TINYXML:%.cpp=%.o)

OBJS_ALPHADICT := $(SOURCES_ALPHADICT:%.cpp=%.o)
OBJS_ALCONVERT := $(SOURCES_ALCONVERT:%.cpp=%.o)
OBJS_ALCONVERT += lib/tinyxml2/libtinyxml2.a

DEPS := $(SOURCES_ALPHADICT:%.cpp=%.d)
DEPS += $(SOURCES_ALCONVERT:%.cpp=%.d)

a11: AlphaDict AlConvert

lib/tinyxml2/libtinyxml2.a: $(OBJS_TINYXML)
	$(AR) -r $@  $<

AlphaDict: $(OBJS_ALPHADICT)
	$(CC) $^ -o $@

AlConvert: $(OBJS_ALCONVERT)
	$(CC) $^ -o $@ 

#%.d : %.cpp
#	$(CC) -MM $< -o $@

%.o : %.cpp
	$(CC) $(CPPFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

-include $(DEPS)

clean:
	@find . -name "*.[oadi]" | xargs -I{} rm {}
	-@rm -f AlphaDict

echo:
	@echo $(DEPS)


.PHONY : clean  echo
