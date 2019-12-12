TARGET ?= a.out
SRC_DIRS ?= ./src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)
ROOTINC := /home/alex/DEV/STM32/

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_DIRS += $(ROOTINC)/etl/include \
          $(ROOTINC)/frozen/include \
          $(ROOTINC)/compile-time-regular-expressions/single-header

INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -std=c++2a -Wall -Wextra -O0 -g # -Wextra

$(TARGET): $(OBJS)
	g++ $(LDFLAGS) $(OBJS) -o $@ $(LOADLIBES) $(LDLIBS)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)
