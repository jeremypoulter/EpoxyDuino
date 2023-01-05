# Tests in this directory require different EPOXY_CORE_xxx global macros to be
# defined. When the EPOXY_CORE macro is changed, we must perform a `make clean`
# to recompile various source files. Unfortunately, that means we have to
# inserting a 'make clean' between tests.
#
# **WARNING**: If you copy this Makefile, consider removing the `make clean`
# between each iteration of the `for` loop. Most applications do NOT need to do
# that.
SUB=

tests:
	@set -e; \
	for i in $(SUB)*Test/Makefile; do \
		echo "==== Making:' $$(dirname $$ia)"; \
		echo "==== Cleaning $$(dirname $$i)"; \
		$(MAKE) -C $$(dirname $$i) clean; \
		echo "==== Buidling $$(dirname $$i)"; \
		$(MAKE) -C $$(dirname $$i); \
	done

runtests:
	@set -e; \
	for i in $(SUB)*Test/Makefile; do \
		echo "==== Cleaning $$(dirname $$i)"; \
		$(MAKE) -C $$(dirname $$i) clean; \
		echo "==== Buidling $$(dirname $$i)"; \
		$(MAKE) -C $$(dirname $$i) all; \
		echo "==== Running $$(dirname $$i)"; \
		$(MAKE) -C $$(dirname $$i) run; \
	done

clean:
	@set -e; \
	for i in $(SUB)*Test/Makefile; do \
		echo "==== Cleaning:' $$(dirname $$i)"; \
		$(MAKE) -C $$(dirname $$i) clean; \
	done
