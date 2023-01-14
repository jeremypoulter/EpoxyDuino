# Tests in this directory require different EPOXY_CORE_xxx global macros to be
# defined. When the EPOXY_CORE macro is changed, we must perform a `make clean`
# to recompile various source files. Unfortunately, that means we have to
# inserting a 'make clean' between tests.
#
# **WARNING**: If you copy this Makefile, consider removing the `make clean`
# between each iteration of the `for` loop. Most applications do NOT need to do
# that.
SUB=
CLEAN=0
MAKEOPTS=--no-print-directory
RECIPES=clean all run

tests:
	@set -e; \
  for folder in $(SUB)*Test/Makefile; do \
	  for recipe in clean ""; do \
	     echo "=== $$recipe $$(dirname $$folder)"; \
       $(MAKE) $(MAKEOPTS) -C $$(dirname $$folder) $$recipe; \
     done; \
	done

debug:
	@set -e; \
	for folder in $(SUB)*Test/Makefile; do \
	  for recipe in $(RECIPES) debug; do \
	     echo "=== $$recipe $$(dirname $$folder)"; \
       $(MAKE) $(MAKEOPTS) -C $$(dirname $$folder) $$recipe; \
     done; \
	done

runtests:
	@set -e; \
	for recipe in $(RECIPES) run; do \
    for folder in $(SUB)*Test/Makefile; do \
	     echo "=== $$recipe $$(dirname $$folder)"; \
       $(MAKE) $(MAKEOPTS) -C $$(dirname $$folder) $$recipe; \
     done; \
	done

clean:
	@set -e; \
	for i in $(SUB)*Test/Makefile; do \
		echo "=== Cleaning:' $$(dirname $$i)"; \
		$(MAKE) $(MAKEOPTS) -C $$(dirname $$i) clean; \
	done
