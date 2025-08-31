# Top-level Makefile for building with CMake
# Users can just run 'make' at the top level

.PHONY: all clean rebuild

BINARY = cozenage
BUILD_DIR = build

all:
	@echo "Creating build directory..."
	@mkdir -p $(BUILD_DIR)
	@echo "Running CMake..."
	@cd $(BUILD_DIR) && cmake ..
	@echo "Building..."
	@$(MAKE) -C $(BUILD_DIR)
	@echo "Copying binary to top-level directory..."
	@cp $(BUILD_DIR)/$(BINARY) .

clean:
	@echo "Removing build directory and top-level binary..."
	@rm -rf $(BUILD_DIR) $(BINARY)

rebuild: clean all
	@echo "Rebuild complete!"
