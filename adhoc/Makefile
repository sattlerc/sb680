OUT = libadhoc.a
SRCS = adhoc-internal.c adhoc-sb680.c adhoc-sb880.c

DEPS = adhoc.h
DEPS_INTERNAL = $(DEPS) adhoc-internal.h

OBJS = $(SRCS:.c=.o)

INCLUDES += -I/usr/include/libevdev-1.0
LIBRARIES += -levdev

CFLAGS += -fPIC -Wall -Wextra -g $(INCLUDES) #-DFIX_DEVICE=SB680 #-DDEBUG
CXXFLAGS += -std=c++11
LDFLAGS += $(LIBRARIES)

$(OUT): $(OBJS)
	ar rcs $@ $^
#	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c $(DEPS_INTERNAL)
	$(CC) $(CFLAGS) -c -o $@ $<

test: test.c $(OUT) $(DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(OUT)

collect-clicks: collect-clicks.cpp $(OUT) $(DEPS)
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $< $(OUT)

clean:
	rm -f $(OUT) $(OBJS) test collect-clicks
