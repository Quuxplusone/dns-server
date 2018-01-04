SRCS = \
    src/bytes.cpp \
    src/main.cpp \
    src/message.cpp \
    src/name.cpp \
    src/question.cpp \
    src/resolver.cpp \
    src/rr.cpp \
    src/server.cpp

OBJS = $(patsubst %.cpp,.objs/cxx/%.o,$(SRCS))
DEPS = $(patsubst %.cpp,.deps/cxx/%.d,$(SRCS))

CPPFLAGS += -I src/include
CXXFLAGS += -std=c++11

all: dnsserver

ifneq ($(MAKECMDGOALS), clean)
    -include $(DEPS)
endif

.deps/cxx/%.d: %.cpp
	@mkdir -p $(shell dirname $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM -MP -MT $(patsubst %.cpp,.objs/cxx/%.o,$<) $< -MF $@

.objs/cxx/%.o: %.cpp .deps/cxx/%.d
	@mkdir -p $(shell dirname $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

dnsserver: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf .deps .objs dnsserver
