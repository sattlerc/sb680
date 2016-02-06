OUT = libadhoc.a
SRCS = adhoc-internal.c adhoc-sb680.c adhoc-sb885.c

DEPS = adhoc.h adhoc-internal.h

OBJS = $(SRCS:.c=.o)

INCLUDES += -I/usr/include/libevdev-1.0
LIBRARIES += -levdev

CFLAGS += -fPIC -Wall -Wextra -g $(INCLUDES) #-DFIX_DEVICE=SB680 #-DDEBUG
CXXFLAGS += -std=c++11
LDFLAGS += $(LIBRARIES)

$(OUT): $(OBJS)
	ar rcs $@ $^

%.o: %.c $(DEPS_INTERNAL)
	$(CC) $(CFLAGS) -c -o $@ $<

print: print.c $(OUT) $(DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(OUT)

test: test.c $(OUT) $(DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(OUT)

collect-clicks: collect-clicks.cpp $(OUT) $(DEPS)
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $< $(OUT)

setup: print
	./setup.sh

points: collect-clicks
	./collect-clicks | tee clicks
	../calibrate/dist/build/calibrate-compute/calibrate-compute <clicks | tee transform

clean:
	rm -f $(OUT) $(OBJS) print test collect-clicks
