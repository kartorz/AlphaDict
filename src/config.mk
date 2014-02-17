##=== config modules ===##

TOPDIR := $(abspath .)

CC := g++
INCLUDES := -I$(TOPDIR) -I$(TOPDIR)/dict  -I$(TOPDIR)/lib
CPPFLAGS := -g -MMD -O0
LDFLAGS  := 
DEFINES  := -D_LINUX

MODULE.yes := 