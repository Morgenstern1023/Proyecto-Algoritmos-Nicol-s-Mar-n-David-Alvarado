# ============================================================================
# @file Makefile
# @brief Compilación en C++17 (Arch Linux / WSL2) con -pthread, OpenSSL y ASan.
# @author Oscar David Alvarado Navarrete (DA)
# ============================================================================

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS  := -pthread -lcrypto

ifeq ($(SANITIZE), 1)
    CXXFLAGS += -fsanitize=address,undefined -g -O1 -fno-omit-frame-pointer
    LDFLAGS  += -fsanitize=address,undefined
else
    CXXFLAGS += -O2
endif

HEADERS     := $(wildcard include/*/*.h)
BIN_DIR     := bin
TARGET_APP  := $(BIN_DIR)/crai_server
TARGET_TEST := $(BIN_DIR)/tests_crai

.PHONY: all test run clean

all: $(TARGET_APP)

$(TARGET_APP): src/main.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

test: tests/tests_main.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $(TARGET_TEST) $(LDFLAGS)

run: $(TARGET_APP)
	./$(TARGET_APP)

$(BIN_DIR):
	mkdir -p $@

clean:
	rm -rf $(BIN_DIR)
