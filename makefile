# ----------------------------
# Makefile Options
# ----------------------------

NAME = CTILES
ICON = icon.png
DESCRIPTION = "Color matching game"
COMPRESSED = NO

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)
