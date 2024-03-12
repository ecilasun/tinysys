.DEFAULT_GOAL := all

TOPTARGETS := all clean

IGNORELIST := Makefile/. $(wildcard *.bat/.) $(wildcard *.sh/.)

SUBDIRS := $(filter-out $(IGNORELIST), $(wildcard */.))

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -s -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
