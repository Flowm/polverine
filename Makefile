# Makefile for POLVERINE_MULTI PlatformIO project
# Default target: build, upload, and monitor

.PHONY: all build clean upload monitor help

# Default target - build, upload, and monitor in sequence
all: build upload monitor

# Build the project
build:
	pio run

# Clean build artifacts
clean:
	pio run --target clean

# Upload firmware to device
upload:
	pio run --target upload

# Start serial monitor
monitor:
	pio run --target monitor

# Build and upload (useful for quick deployment)
deploy: build upload

# Full clean rebuild
rebuild: clean build

# Show available targets
help:
	@echo "Available targets:"
	@echo "  all      - Build, upload, and monitor (default)"
	@echo "  build    - Build the project"
	@echo "  clean    - Clean build artifacts"
	@echo "  upload   - Upload firmware to device"
	@echo "  monitor  - Start serial monitor"
	@echo "  deploy   - Build and upload"
	@echo "  rebuild  - Clean and build"
	@echo "  help     - Show this help message"
