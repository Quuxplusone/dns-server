DNS_AUTH_SERVER_SRCS = \
    src/authoritative-resolver.cpp \
    src/bytes.cpp \
    src/ipaddressv4.cpp \
    src/main-auth-server.cpp \
    src/message.cpp \
    src/name.cpp \
    src/question.cpp \
    src/rr.cpp \
    src/rrtype.cpp \
    src/server.cpp \
    src/symboltable.cpp

DNS_DIG_SRCS = \
    src/bytes.cpp \
    src/ipaddressv4.cpp \
    src/main-dig.cpp \
    src/message.cpp \
    src/name.cpp \
    src/question.cpp \
    src/rr.cpp \
    src/rrtype.cpp \
    src/stub-resolver.cpp \
    src/symboltable.cpp \
    src/upstream.cpp

DNS_AUTH_SERVER_OBJS = $(patsubst %.cpp,.objs/cxx/%.o,$(DNS_AUTH_SERVER_SRCS))
DNS_DIG_OBJS = $(patsubst %.cpp,.objs/cxx/%.o,$(DNS_DIG_SRCS))
DEPS = $(patsubst %.cpp,.deps/cxx/%.d,$(DNS_AUTH_SERVER_SRCS) $(DNS_DIG_SRCS))

CPPFLAGS += -I src/include
CXXFLAGS += -std=c++11 -O2 -W -Wall -Wextra -pedantic -Werror -Wno-sign-compare

all: dns-auth-server dns-dig

ifneq ($(MAKECMDGOALS), clean)
    -include $(DEPS)
endif

.deps/cxx/%.d: %.cpp
	@mkdir -p $(shell dirname $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM -MP -MT $(patsubst %.cpp,.objs/cxx/%.o,$<) $< -MF $@

.objs/cxx/%.o: %.cpp .deps/cxx/%.d
	@mkdir -p $(shell dirname $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

dns-auth-server: $(DNS_AUTH_SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

dns-dig: $(DNS_DIG_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf .deps .objs dns-auth-server dns-dig
