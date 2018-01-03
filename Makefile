SOURCEDIR = ./src
CXXSOURCES = main.cpp application.cpp logger.cpp message.cpp query.cpp resolver.cpp response.cpp server.cpp

CXXOBJECTS = $(CXXSOURCES:.cpp=.o)
CXXFLAGS = -g $(INCLUDEDIRS)
LDFLAGS = $(LIBDIRS) $(LIBS)
		
build: $(CXXOBJECTS)
	$(CXX) -o dnsserver $(CXXOBJECTS) $(LDFLAGS)

%.o: $(SOURCEDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(CXXOBJECTS) dnsserver dnsserver.log
