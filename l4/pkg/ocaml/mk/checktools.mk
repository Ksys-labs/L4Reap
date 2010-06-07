
ifeq ($(shell sh -c "command -v $(CAMLOPT)"),)
TARGET :=
text := $(shell echo -e "\033[32mHost tool '$(CAMLOPT)' missing, skipping build.\033[0m")
$(info $(text))
endif

ifeq ($(shell sh -c "command -v $(CAMLC)"),)
TARGET :=
text := $(shell echo -e "\033[32mHost tool '$(CAMLC)' missing, skipping build.\033[0m")
$(info $(text))
endif
