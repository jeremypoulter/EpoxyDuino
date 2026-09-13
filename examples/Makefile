all:
	@status=0; \
	for i in */Makefile; do \
		echo '==== Making:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i) || status=1; \
	done; \
	exit $$status

# Run each compiled example for a few seconds and check that it does not
# crash. Most examples run an infinite loop() and never exit on their own, so
# a clean timeout (exit code 124) is treated as success, same as an example
# that calls exit(0) itself. Any other exit code (segfault, abort, etc.) is a
# validation failure.
validate:
	@status=0; \
	$(MAKE) all || status=1; \
	for i in */Makefile; do \
		d=$$(dirname $$i); \
		name=$$(basename $$d); \
		bin=$$d/$$name.out; \
		if [ ! -x $$bin ]; then \
			echo "==== SKIP (no binary): $$d"; \
			continue; \
		fi; \
		echo "==== Validating: $$d"; \
		( cd $$d && timeout -k 1 3 ./$$name.out < /dev/null > validate.log 2>&1 ); \
		rc=$$?; \
		if [ $$rc -ne 0 ] && [ $$rc -ne 124 ]; then \
			echo "FAIL: $$d exited with code $$rc"; \
			cat $$d/validate.log; \
			status=1; \
		else \
			echo "OK: $$d (exit $$rc)"; \
		fi; \
	done; \
	exit $$status

clean:
	@status=0; \
	for i in */Makefile; do \
		echo '==== Cleaning:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i) clean || status=1; \
		rm -f $$(dirname $$i)/validate.log; \
	done; \
	exit $$status
