CC := g++
CFLAGS := -Wall -g -Og
INCLUDES := include
LDLIBS := -lstdc++
TARGETS := test_example
BUILD := bin/

all: $(BUILD) $(addprefix $(BUILD), $(TARGETS))

$(BUILD):
	mkdir -p $@

$(BUILD)test_example: test_example.c c_testcase.cpp
	$(CC) $^ $(CFLAGS) $(LDLIBS) -o $@

.PHONY: test
test: all
	@python unittest.py

.PHONY: clean
clean:
	rm -rf $(BUILD)