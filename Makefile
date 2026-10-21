CMAKE ?= cmake

BUILD_DIR := build
APP := $(BUILD_DIR)/deepred

.PHONY: all configure run check clean FORCE
.NOTPARALLEL: clean check

all: $(APP)

configure: $(BUILD_DIR)/CMakeCache.txt

$(BUILD_DIR)/CMakeCache.txt: CMakeLists.txt
	$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release

$(APP): FORCE $(BUILD_DIR)/CMakeCache.txt
	$(CMAKE) --build $(BUILD_DIR) --config Release

run: $(APP)
	./$(APP)

# CI uses this target: compile the GUI without attempting to open a display.
check: $(APP)

clean:
	$(CMAKE) -E rm -rf $(BUILD_DIR)

FORCE:
