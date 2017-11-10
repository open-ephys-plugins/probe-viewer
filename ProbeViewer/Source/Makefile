LIBNAME := $(notdir $(CURDIR))
OBJDIR := $(OBJDIR)/$(LIBNAME)
TARGET := $(LIBNAME).so
OS := $(shell uname)

SRC_DIR := ${shell find ./ -type d -print}
VPATH := $(SOURCE_DIRS)

SRC := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c*))
OBJ := $(addprefix $(OBJDIR)/,$(notdir $(SRC:.cpp=.o)))
OBJ := $(OBJ:.c=.o)

#Extra macros and libraries needed by the plugin
#CXXFLAGS := $(CXXFLAGS) -D EXAMPLE_MACRO
#LDFLAGS := $(LDFLAGS) -lExampleLib



BLDCMD := $(CXX) -shared -o $(OUTDIR)/$(TARGET) $(OBJ) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)

VPATH = $(SRC_DIR)

.PHONY: objdir

$(OUTDIR)/$(TARGET): objdir $(OBJ)
	-@mkdir -p $(BINDIR)
	-@mkdir -p $(LIBDIR)
	-@mkdir -p $(OUTDIR)
	@echo "Building $(TARGET)"
	@$(BLDCMD)

$(OBJDIR)/%.o : %.c
	@echo "Compiling $<"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/%.o : %.cpp
	@echo "Compiling $<"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"


objdir:
	-@mkdir -p $(OBJDIR)

clean:
	@echo "Cleaning $(LIBNAME)"
	-@rm -rf $(OBJDIR)
	-@rm -f $(OUTDIR)/$(TARGET)

-include $(OBJ:%.o=%.d)
