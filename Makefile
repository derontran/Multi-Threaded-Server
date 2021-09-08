CC = gcc
CFLAGS = -g -Wall -lpthread

# TODO: rename the target to something meaningful!
TARGET = client server

OBJDIR = object
HDRS = $(wildcard *.h)
SRCS = $(wildcard *.c)
OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.c=.o))

.PHONY: all
all: $(TARGET)

# WARNING: *must* have a tab before each definition
$(TARGET): $(OBJDIR) $(OBJS)
	$(CC) $(CFLAGS) $(OBJDIR)/protocol.o $(OBJDIR)/Sema.o $(OBJDIR)/thread_pool.o $(OBJDIR)/$@.o  -o $@

$(OBJDIR)/%.o: %.c $(HDRS) $(OBJDIR)
	$(CC) $(CFLAGS) -c $*.c -o object/$*.o

$(OBJDIR):
	mkdir $(OBJDIR)

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)
