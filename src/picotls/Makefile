#
# OMNeT++/OMNEST Makefile for picotls
#

# Name of target to be created (-o option)
TARGET_DIR = .
TARGET_NAME = $(LIB_PREFIX)picotls$(D)
TARGET = $(TARGET_NAME)$(A_LIB_SUFFIX)
TARGET_FILES = $(TARGET_DIR)/$(TARGET)

# C++ include paths (with -I)
INCLUDE_PATH = -Iinclude

# Output directory
PROJECT_OUTPUT_DIR = ../../out
PROJECTRELATIVE_PATH = src/picotls
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .c files
OBJS = $O/lib/hpke.o $O/lib/openssl.o $O/lib/picotls.o

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
CONFIGFILE = $(shell opp_configfilepath)
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

COPTS = $(CFLAGS) $(IMPORT_DEFINES)  $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE (if COPTS has changed since last build)
# and make the object files depend on it
COPTS_FILE = $O/.last-copts
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
  $(shell $(MKPATH) "$O")
  $(file >$(COPTS_FILE),$(COPTS))
endif

# Main target
all: $(TARGET_FILES)

$(TARGET_DIR)/% :: $O/%
	@mkdir -p $(TARGET_DIR)
	$(Q)$(LN) $< $@
ifeq ($(TOOLCHAIN_NAME),clang-msabi)
	-$(Q)-$(LN) $(<:%.dll=%.lib) $(@:%.dll=%.lib) 2>/dev/null

$O/$(TARGET_NAME).pdb: $O/$(TARGET)
endif

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile $(CONFIGFILE)
	@$(MKPATH) $O
	@echo Creating static library: $@
	$(Q)$(AR_CR) $O/$(TARGET) $(OBJS) $(EXTRA_OBJS)

.PHONY: all clean depend

# disabling all implicit rules
.SUFFIXES :

$O/%.o: %.c $(COPTS_FILE)
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CC) -c $(CFLAGS) $(COPTS) -o $@ $<

clean:
	$(qecho) Cleaning $(TARGET)
	$(Q)-rm -rf $O
	$(Q)-rm -f $(TARGET_FILES)

# include all dependencies
-include $(OBJS:%=%.d)
