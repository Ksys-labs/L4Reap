#!/usr/bin/make -f
include ../config.make


OUT_PREFIX := libwhefs-API-
clean-doxy:
	-rm -fr $(OUT_PREFIX)*

clean: clean-doxy

doc doxy:
	@echo "Generating API docs..."
	@doxygen || exit $$?
	@test -e $(AMALGAMATION_H) || { \
		echo "Amalgamation header [$(AMALGAMATION_H)] not found. Skipping internal docs generation."; \
		exit 0; \
		}; \
		doxygen Doxyfile-internal
	@echo "Output is in: "; ls -1td $(OUT_PREFIX)*

all:
	@echo "Run 'make doc' to generate the API docs with doxygen."
