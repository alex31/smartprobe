# Required include directories
BOARDINC = cfg

# List of all the board related files.
BOARDSRC = ${BOARDINC}/board.c

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
