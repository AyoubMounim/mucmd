
CC=gcc
EXAMPLES_SRC_DIR=examples
EXAMPLES_BUILD_DIR="examples/build"


examples: $(EXAMPLES_BUILD_DIR)/main.o examples_build_dir
	$(CC) -o $(EXAMPLES_BUILD_DIR)/mucmd-example $(EXAMPLES_BUILD_DIR)/main.o

$(EXAMPLES_BUILD_DIR)/main.o: $(EXAMPLES_SRC_DIR)/main.c mucmd.h examples_build_dir
	$(CC) -c -I./ -o $(EXAMPLES_BUILD_DIR)/main.o $(EXAMPLES_SRC_DIR)/main.c

examples_build_dir:
	mkdir -p $(EXAMPLES_BUILD_DIR)
