
# This is a GNU Makefile.

# It can be used to compile an OpenCL program with
# the Altera SDK for OpenCL.
# See README.txt for more information.


# You must configure ALTERAOCLSDKROOT to point the root directory of the Altera SDK for OpenCL
# software installation.
# See doc/getting_started.txt for more information on installing and
# configuring the Altera SDK for OpenCL.


# Creating a static library
TARGET = vectorAdd.exe

# Where is the Altera SDK for OpenCL software?
ifeq ($(wildcard $(ALTERAOCLSDKROOT)),)
$(error Set ALTERAOCLSDKROOT to the root directory of the Altera SDK for OpenCL software installation)
endif
ifeq ($(wildcard $(ALTERAOCLSDKROOT)/host/include/CL/opencl.h),)
$(error Set ALTERAOCLSDKROOT to the root directory of the Altera SDK for OpenCL software installation.)
endif

# Libraries to use, objects to compile
OBJ_DIR = obj
OBJ_EXT = o
SRCS = main.cpp

# Compiler/Linker flags
INCS = -I../inc
LINKFLAGS = /nologo /libpath:../lib
CPPFLAGS = /nologo /MD /Zi /EHsc


# OpenCL API header files and linker line.
AOCL_COMPILE_CONFIG=$(shell aocl compile-config)
AOCL_LINK_CONFIG=$(shell aocl link-config)

LINKFLAGS += /libpath:../shrUtils \
	/libpath:../oclUtils

CPPFLAGS += -DALTERA_CL

# Keep it tidy with an obj directory
OBJS = $(foreach OBJ,$(patsubst %.cpp,%.$(OBJ_EXT),$(patsubst %.c,%.$(OBJ_EXT),$(SRCS))),$(OBJ_DIR)/$(OBJ))

# Make it all!
all : Makefile $(TARGET)

# Standard make targets
clean :
	@rm -f $(OBJS) $(TARGET) *.pdb *~ *.manifest *.ptx
	@rm -rf $(OBJ_DIR)

$(OBJ_DIR)/_dir :
	@mkdir -p $(OBJ_DIR)
	@touch $(OBJ_DIR)/_dir

$(OBJ_DIR)/%.$(OBJ_EXT) : ./%.cpp $(OBJ_DIR)/_dir
	@cl /Fo$@ -c $(CPPFLAGS) -DWINDOWS $(INCS) $(AOCL_COMPILE_CONFIG) $<

$(TARGET) : $(OBJS)
	link -nologo /OUT:$@ $(LINKFLAGS) $(OBJS) $(AOCL_LINK_CONFIG)
