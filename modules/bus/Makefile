#----------------------------------------------------------------------------
# COMPILE macros
#----------------------------------------------------------------------------

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))

all:$(OBJS)

$(OBJS): %.o:%.c
	$(CC) -c $(CFLAGS) $(INCS) $< -o $(OBJDIR)/$@
.PHNOY:clean
clean:
	rm -f $(OBJS)