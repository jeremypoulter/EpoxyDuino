# vim: noexpandtab
# make help for more infos

folder?=
compiler?=gcc
jobs?=4
clearn?=0
# MAKEOPTS=--no-print-directory compiler=$(compiler)
ifeq (clean, 1)
  RECIPES=clean all
else
  RECIPES=all
endif

$(info MAKEOPTS : $(MAKEOPTS))

runtests:
	@set -e; \
	for recipe in $(RECIPES) run; do \
    for makefile in $(folder)*Test/Makefile; do \
	     echo "=== $$recipe $$(dirname $$makefile)"; \
       $(MAKE) -C $$(dirname $$makefile) $(MAKEOPTS) -j $(jobs) $$recipe; \
     done; \
	done

tests:
	@set -e; \
  for makefile in $(folder)*Test/Makefile; do \
	  for recipe in clean ""; do \
	     echo "=== $$recipe in $$(dirname $$makefile)"; \
       $(MAKE) -C $$(dirname $$makefile) $(MAKEOPTS) $$recipe; \
     done; \
	done

help:
	@echo
	@echo "  Cmdline              Description                               "
	@echo "  ---------------------------------------------------------------"
	@echo "  make sanitize        use clang and sanitizer"
	@echo "  make clang           use clang and runtests"
	@echo "  ---------------------------------------------------------------"
	@echo
	@echo "  Flags"
	@echo "  ---------------------------------------------------------------"
	@echo "  folder=Inj           only make tests for folders starting by Inj"
	@echo "  compiler=clang       use clang as compiler"
	@echo "  sanitize=1           use clang and sanitize"
	@echo "  ---------------------------------------------------------------"
	@echo

clean:
	@set -e; \
	for i in $(folder)*Test/Makefile; do \
		echo "=== Cleaning:' $$(dirname $$i)"; \
		$(MAKE) $(MAKEOPTS) -C $$(dirname $$i) clean; \
	done

clang:
	$(MAKE) compiler=clang $(filter-out clang,$(MAKECMDGOALS)) $(MAKEFLAGS)

sanitize:
	$(MAKE) compiler=clang sanitize=1 $(filter-out sanitize,$(MAKECMDGOALS)) $(MAKEFLAGS)
