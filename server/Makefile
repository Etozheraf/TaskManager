# Makefile for Docker Compose

# Include environment variables (optional)
-include .env
export

# Variables
COMPOSE_FILE ?= docker-compose.yml
BUILD_DIR ?= build
BINARY ?= $(BUILD_DIR)/src/TaskManagerServer
TEST_BINARY ?= $(BUILD_DIR)/tests/unit_tests

# Default target
all: build up

# ========================================
# DOCKER COMPOSE COMMANDS
# ========================================

# Build the Docker image
build:
	docker-compose -f $(COMPOSE_FILE) build

# Start the services in detached mode
up:
	docker-compose -f $(COMPOSE_FILE) up -d

# Stop the services
down:
	docker-compose -f $(COMPOSE_FILE) down

# Restart the services
restart: down up

# View the logs
logs:
	docker-compose -f $(COMPOSE_FILE) logs -f




# ========================================
# NATIVE BUILD/RUN (without Docker)
# ========================================

.PHONY: native-config native-build native-run native-run-without-build native-clean

native-config:
	cmake -S . -B $(BUILD_DIR)

native-build: native-config
	cmake --build $(BUILD_DIR) -j

# Runs the built server with env vars loaded from .env (already exported at top)
native-run: native-build
	@echo "Starting TaskManagerServer with environment from .env (if present)"
	$(BINARY)

native-run-without-build:
	@echo "Starting TaskManagerServer with environment from .env (if present)"
	$(BINARY)

native-clean:
	rm -rf $(BUILD_DIR)

# ========================================
# TEST COMMANDS
# ========================================

.PHONY: test-config test-build test-run test-verbose test-clean

# Configure project (ensures tests are generated)
test-config: native-config

# Build tests target
test-build:
	cmake --build $(BUILD_DIR) --target unit_tests -j

# Run tests via ctest (parallel)
test-run: test-build
	ctest --test-dir $(BUILD_DIR) -j --output-on-failure | cat


test-run-without-build:
	ctest --test-dir $(BUILD_DIR) -j --output-on-failure | cat

# Verbose run of unit test binary
test-verbose: test-build
	$(TEST_BINARY) --gtest_color=yes

# Clean only test binaries (keep rest of build)
test-clean:
	rm -rf $(BUILD_DIR)/tests

# ========================================
# DATABASE COMMANDS
# ========================================

# Start only database service
db-up:
	docker-compose -f $(COMPOSE_FILE) up -d db

# Stop only database service
db-down:
	docker-compose -f $(COMPOSE_FILE) stop db

# Restart database service
db-restart: db-down db-up

# View database logs
db-logs:
	docker-compose -f $(COMPOSE_FILE) logs -f db

# Connect to database
db-connect:
	docker-compose -f $(COMPOSE_FILE) exec db psql -U $(DB_USER) -d $(DB_NAME)

# ========================================
# FLYWAY COMMANDS
# ========================================

flyway-migrate:
	docker-compose -f $(COMPOSE_FILE) run --rm flyway migrate | cat

flyway-info:
	docker-compose -f $(COMPOSE_FILE) run --rm flyway info | cat

flyway-validate:
	docker-compose -f $(COMPOSE_FILE) run --rm flyway validate | cat

flyway-clean:
	docker-compose -f $(COMPOSE_FILE) run --rm flyway clean | cat

# ========================================
# CLEANUP COMMANDS
# ========================================

# Clean up dangling images and volumes
clean:
	docker-compose -f $(COMPOSE_FILE) down -v --rmi all
	docker system prune -f

.PHONY: all build up down restart logs clean db-up db-down db-restart db-logs db-connect flyway-migrate flyway-info flyway-validate flyway-clean native-config native-build native-run native-run-without-build native-clean test-config test-build test-run test-verbose test-clean
