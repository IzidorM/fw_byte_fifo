TOOLS_DIR = ../../tools/
SRC_DIR = ../src/

ifndef $(TARGET)
TARGET=byte_fifo
endif

PROJECT_BASE_DIR = .

BUILD_DIR=/tmp/

FILES_TO_TEST_SRC= \
	$(SRC_DIR)/byte_fifo.c

TESTS_SRC= Test_$(TARGET).c


C_FLAGS+=-g -ggdb3 -Wall -std=c11 -pedantic -fprofile-arcs -ftest-coverage
DEFINES+=-D UNIT_TESTS


UNITY_INC_FILES = $(TOOLS_DIR)/Unity/src/
UNITY_SRC_FILES = $(TOOLS_DIR)/Unity/src/unity.c

TARGET_SRC_TEST_RUNNER_FILES= $(patsubst %.c, $(BUILD_DIR)/%_Runner.c, $(TESTS_SRC))

TARGET_SRC_ALL=$(FILES_TO_TEST_SRC) \
	$(TESTS_SRC) \
	$(TARGET_SRC_TEST_RUNNER_FILES) \
	$(UNITY_SRC_FILES)

INC_DIRS= \
	$(SRC_DIR) \
	$(UNITY_INC_FILES) \
	. 

COVERAGE_FILES=$(FILES_TO_TEST_SRC:.c=.gcda)
COVERAGE_FILES_NO_PATH=$(notdir $(COVERAGE_FILES))

C_COMPILER=gcc

all: run

INC_DIRS_GCC = $(patsubst %,-I%, $(INC_DIRS))

run: $(BUILD_DIR)/$(TARGET)
	@echo "Running tests"
	@/usr/bin/time --format="\nTest run time: %E, Max memory: %M Kb" $(BUILD_DIR)/$(TARGET)
	@printf "\nGCOV:\n"
	@gcov -n $(BUILD_DIR)/$(COVERAGE_FILES_NO_PATH) |\
		sed 's/^\s*$$/Sum/' |\
		grep -E "Lines executed|File|Sum"


$(BUILD_DIR)/%_Runner.c: $(TESTS_SRC)
	@echo "generating $<"
	@ruby $(TOOLS_DIR)/Unity/auto/generate_test_runner.rb $< $@


$(BUILD_DIR)/$(TARGET): $(TARGET_SRC_ALL)
	@$(C_COMPILER) -dumpbase '' $(C_FLAGS) $(DEFINES) $(INC_DIRS_GCC) $(TARGET_SRC_ALL) -o $(BUILD_DIR)/$(TARGET)
	@echo ""
	@size $(BUILD_DIR)/$(TARGET)
	@echo ""

clean:
	@rm -f $(BUILD_DIR)/$(TARGET) \
	$(BUILD_DIR)/*_Runner.c \
	$(BUILD_DIR)/*.gcno \
	*.gcda *.info *.gcov


ifeq ($(MAKECMDGOALS),rtags)
include $(TOOLS_DIR)/rtags/rtags.mk
endif

