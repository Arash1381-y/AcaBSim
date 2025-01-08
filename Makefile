SRCDIR = src
OBJDIR = obj
BINDIR = bin
INCLUDEDIR = include

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))
INCLUDES = -I$(INCLUDEDIR)
CFLAGS = -Wall -g -ggdb

.PHONY: clean


$(OBJDIR)/%.o: $(SRCDIR)/%.c
	gcc $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BINDIR)/main: $(OBJS)
	gcc $(CFLAGS)  $(OBJS) $(INCLUDES) -o $@ -lm


clean: $(OBJDIR) $(BINDIR)
	@if [ -n "$(ls -A $(OBJDIR))" ]; then \
		rm $(OBJDIR)/*; \
	fi
	@if [ -n "$(ls -A $(BINDIR))" ]; then \
		rm $(BINDIR)/*; \
	fi