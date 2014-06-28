CXX = g++
CXXFLAGS = -O2 -g -fPIC -Wall -Wextra -pg -I../CFAR/lib/include -I../CFAR/lib/src -I../CFAR/lib/ossim_plugins/ossim
COMPILEFLAGS =`pkg-config opencv --cflags`  
LINKFLAGS = `pkg-config opencv --libs`
TARGET = driver
OBJS = src/commonutils.o src/gdalprocess.o src/ossimSimpleFilter.o src/ossimGlobalFilter.o src/ossimCFARFilter.o src/ossimWaveletFilter.o src/ossimSDFilter.o driver.o

%.o: %.C
	$(CXX) $(CXXFLAGS) $(COMPILEFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LINKFLAGS) -o $(TARGET) ../lib/libossim.so ../lib/libossimgdal_plugin.so ../lib/libossim_plugin.so ../lib/libgdal.so

.PHONEY: clean

checkENV:
	printenv LD_LIBRARY_PATH

run:
	rm -rf results/cfar/*.* results/global/*.* results/*.*
	./$(TARGET) ${ARGS}

clean: 
	rm -f deps.mak $(OBJS) $(TARGET)
	
empty: 
	rm -rf results/cfar/*.* results/global/*.* results/*.*

deps.mak:
	$(CXX) -MM *.C > deps.mak

include deps.mak
