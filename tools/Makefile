.PHONY: all etc html clean tags save load stage pkg test

TOOLS ?= tools
SCONS ?= $(TOOLS)/scons.py

all:
	@$(SCONS) -f $(TOOLS)/SConstruct -j1 target-default

single:
	@$(SCONS) -f $(TOOLS)/SConstruct -j1 target-default

verbose:
	@PI_VERBOSE=1 $(SCONS) -f $(TOOLS)/SConstruct -j1 target-default

quiet:
	@$(SCONS) -f $(TOOLS)/SConstruct -j1 -s target-default

clean:
	@$(SCONS) -f $(TOOLS)/SConstruct -c
	rm -rf tmp env.sh dbg.sh tags
	find . -name \*.pyc -exec rm {} \;

tags:
	@$(SCONS) -f $(TOOLS)/SConstruct tags

stage:
	@$(SCONS) -f $(TOOLS)/SConstruct target-stage

mpkg:
	@$(SCONS) -f $(TOOLS)/SConstruct target-mpkg

pkg:
	@$(SCONS) -f $(TOOLS)/SConstruct target-pkg
