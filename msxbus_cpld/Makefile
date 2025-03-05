###################################################################
# Project Configuration: 
# 
# Specify the name of the design (project) and the Quartus II
# Settings File (.qsf)
###################################################################

PROJECT = $(patsubst %.qpf,%,$(wildcard *.qpf))
TOP_LEVEL_ENTITY = $(PROJECT)
ASSIGNMENT_FILES = $(PROJECT).qpf $(PROJECT).qsf

###################################################################
# Part, Family, Boardfile DE1 or DE2
FAMILY = "MAX II"
PART = EPM240T100C5
FLASH_DEVICE = "EPCS4"
BOARDFILE = RPMC 
###################################################################

###################################################################
# Setup your sources here
SRCS = $(shell find . -name '*.sv') \
  $(shell find . -name '*.v') \
  $(shell find . -name '*.vhdl') \
  $(shell find . -name '*.vhd') \
  $(shell find . -name '*.qsys')

###################################################################
# Main Targets
#
# all: build everything
# clean: remove output files and database
# program: program your device with the compiled design
###################################################################

all: smart.log $(PROJECT).asm.rpt $(PROJECT).sta.rpt pof jic

clean:
	$(RM) -rf reports db incremental_db smart.log *.chg

map: smart.log $(PROJECT).map.rpt
fit: smart.log $(PROJECT).fit.rpt
asm: smart.log $(PROJECT).asm.rpt
sta: smart.log $(PROJECT).sta.rpt
smart: smart.log

###################################################################
# Executable Configuration
#
# QUARTUS_PATH: If empty then system path is searched.
#               If set then requires trailling slash.
#               Commented out so it may be set from environment.
###################################################################

# QUARTUS_PATH = /opt/altera/13.0sp1/quartus/bin/

QUARTUS_MAP  = $(QUARTUS_PATH)quartus_map
QUARTUS_FIT  = $(QUARTUS_PATH)quartus_fit
QUARTUS_ASM  = $(QUARTUS_PATH)quartus_asm
QUARTUS_STA  = $(QUARTUS_PATH)quartus_sta
QUARTUS_SH   = $(QUARTUS_PATH)quartus_sh
QUARTUS_PGM  = $(QUARTUS_PATH)quartus_pgm
QUARTUS_CPF  = $(QUARTUS_PATH)quartus_cpf

ifeq ($(shell uname -m),x86_64)
QUARTUS_ARGS = 
endif

MAP_ARGS = $(QUARTUS_ARGS) --family=$(FAMILY) --read_settings_files=on $(addprefix --source=,$(SRCS))
FIT_ARGS = --part=$(PART) --read_settings_files=on
ASM_ARGS = $(QUARTUS_ARGS)
STA_ARGS = $(QUARTUS_ARGS)
SH_ARGS  = $(QUARTUS_ARGS)
PGM_ARGS = $(QUARTUS_ARGS) --no_banner --mode=jtag 

###################################################################
# Target implementations
###################################################################

STAMP = echo done >

$(PROJECT).map.rpt: map.chg $(SOURCE_FILES) 
	$(QUARTUS_MAP) $(MAP_ARGS) $(PROJECT)
	$(STAMP) fit.chg

$(PROJECT).fit.rpt: fit.chg $(PROJECT).map.rpt
	$(QUARTUS_FIT) $(FIT_ARGS) $(PROJECT)
	$(STAMP) asm.chg
	$(STAMP) sta.chg

$(PROJECT).asm.rpt: asm.chg $(PROJECT).fit.rpt
	$(QUARTUS_ASM) $(ASM_ARGS) $(PROJECT)

$(PROJECT).sta.rpt: sta.chg $(PROJECT).fit.rpt
	$(QUARTUS_STA) $(STA_ARGS) $(PROJECT) 

smart.log: $(ASSIGNMENT_FILES)
	$(QUARTUS_SH) $(SH_ARGS) --determine_smart_action $(PROJECT) > smart.log

###################################################################
# Project initialization
###################################################################

$(ASSIGNMENT_FILES):
	$(QUARTUS_SH) $(SH_ARGS) --prepare -f $(FAMILY) -t $(TOP_LEVEL_ENTITY) $(PROJECT)
	-cat $(BOARDFILE) >> $(PROJECT).qsf
map.chg:
	$(STAMP) map.chg
fit.chg:
	$(STAMP) fit.chg
sta.chg:
	$(STAMP) sta.chg
asm.chg:
	$(STAMP) asm.chg
	
show:
	echo $(SRCS)
	
pof: reports/$(PROJECT).sof
	$(QUARTUS_CPF) -c -d $(FLASH_DEVICE) $< $(patsubst %.sof,%.pof,$<)
	
jic: reports/$(PROJECT).sof
	$(QUARTUS_CPF) -c -s $(PART) -d $(FLASH_DEVICE) $< $(patsubst %.sof,%.jic,$<)

###################################################################
# Programming the device
###################################################################

program: reports/$(PROJECT).sof
	$(QUARTUS_PGM) $(PGM_ARGS) -o "P;reports/$(PROJECT).sof"

program-pof: reports/$(PROJECT).pof
	$(QUARTUS_PGM) $(PGM_ARGS) -o "BVP;reports/$(PROJECT).pof"
