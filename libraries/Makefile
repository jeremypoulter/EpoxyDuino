tests:
	@status=0; \
	for i in */tests/Makefile; do \
		echo '==== Making:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i) || status=1; \
	done; \
	exit $$status

runtests:
	@status=0; \
	for i in */tests/Makefile; do \
		echo '==== Running:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i) runtests || status=1; \
	done; \
	exit $$status

examples:
	@status=0; \
	for i in */examples/Makefile; do \
		echo '==== Making:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i) || status=1; \
	done; \
	exit $$status

clean:
	@set -e; \
	for i in */examples/Makefile */tests/Makefile; do \
		echo '==== Cleaning:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i) clean; \
	done
