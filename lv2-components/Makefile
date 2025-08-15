#!/usr/bin/make -f
# Makefile for DPF #
# ---------------- #
# Created by falkTX
#

include DPF/Makefile.base.mk

all: dgl examples gen

# --------------------------------------------------------------

ifneq ($(CROSS_COMPILING),true)
CAN_GENERATE_TTL = true
else ifneq ($(EXE_WRAPPER),)
CAN_GENERATE_TTL = true
endif

dgl:
ifeq ($(HAVE_DGL),true)
	$(MAKE) -C dgl
endif

examples: dgl
	$(MAKE) all -C plugins/AutoLeveler

ifeq ($(CAN_GENERATE_TTL),true)
gen: examples utils/lv2_ttl_generator
	@$(CURDIR)/utils/generate-ttl.sh

utils/lv2_ttl_generator:
	$(MAKE) -C utils/lv2-ttl-generator
else
gen:
endif

tests: dgl
	$(MAKE) -C tests

# --------------------------------------------------------------

clean:
	$(MAKE) clean -C dgl
	$(MAKE) clean -C plugins/AutoLeveler
	$(MAKE) clean -C utils/lv2-ttl-generator
	rm -rf bin build

# --------------------------------------------------------------

.PHONY: dgl examples tests