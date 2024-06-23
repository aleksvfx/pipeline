# MUST TODO
#
# 1. Easy variable to indicate nuke version
# 2. Optimize vs. Debug build (variables)
# 3. Output location that uses nuke version  (AND update NUKE_PATH)
# 4. When we fix a bug in a plugin we need an easy rebuild-for-each-nuke-ver
#    (Eithere here, or as a wrapper build script that will inovke make multiple
#    times)
#
## planned platform names are:
## _linux_64
## _osx_64
## _win_64
## _win_32

#NUKE_VERSION ?= nuke_5.1v5_linux_64
#NUKE_VERSION ?= nuke_5.2v3_linux_64
NUKE_VERSION ?= nuke_6.1v1_linux_64

# To actually install the plugin you need to copy the plugin to
# your svn and then 
# svn update /tools/release/nuke/$NUKE_VERSION
# svn update /tools/release/nuke/nuke_5.2v2_linux_64

OUTPUT_DIR = ../${NUKE_VERSION}

NDKDIR ?= /tools/apps/nuke/${NUKE_VERSION}
CXX ?= g++
LINK ?= g++

# First is path to the boost library.  Second path is to the build of the VXL vision library
CXXFLAGS ?= -fPIC -g -c -DFN_VXL_MATRICES -DUSE_GLEW -I/home/eofferman/Documents/nukePlugins/boost_1_44_0 -I$(NDKDIR)/include  -I/home/eofferman/Documents/nukePlugins/bin/core

#CXXFLAGS ?= -fPIC -g -c -DFN_VXL_MATRICES -DUSE_GLEW -I/home/bleair/Downloads/boost_1_39_0 -I$(NDKDIR)/include  -I/home/bleair/Downloads/bin/core
#CXXFLAGS ?= -fPIC -g -c -DFN_VXL_MATRICES -DUSE_GLEW -I/home/bleair/Downloads/boost_1_39_0 -I$(NDKDIR)/include  -I/home/bleair/Downloads/vxl-1.12.0/core -I/home/bleair/Downloads/vxl-1.12.0/vcl
#-I/home/bleair/Downloads/vxl-1.12.0/core
#CXXFLAGS ?= -fPIC -g -c -DUSE_GLEW -I/home/bleair/Downloads/boost_1_39_0 -I$(NDKDIR)/include  
#CXXFLAGS ?= -fPIC -g -c -DUSE_GLEW -I/home/bleair/Downloads/boost_1_34_1 -I$(NDKDIR)/include  

LINKFLAGS ?= -L$(NDKDIR) 
LIBS ?= -lDDImage
LINKFLAGS += -shared

#all: GridWarp2.so SplineWarp2.so
#all: GridWarp2.so Sphere.so Handle.so dpxReader.so
all: GridWarp2.so

GridWarp2.so:	TransformUtils.o GridWarp2.o
	$(LINK) $(LINKFLAGS) $(LIBS) -o ${OUTPUT_DIR}/$(@) $^

dpxReader.so:	dpxReader.o
	$(LINK) $(LINKFLAGS) $(LIBS) -o ${OUTPUT_DIR}/$(@) $^

#SplineWarp2.so:	TransformUtils.o SplineWarp2.o
#	$(LINK) $(LINKFLAGS) $(LIBS) -o ${OUTPUT_DIR}/$(@) $^

Sphere.so: Sphere.o
	$(LINK) $(LINKFLAGS) $(LIBS) -o ${OUTPUT_DIR}/$(@) $^

Handle.so: Handle.o
	$(LINK) $(LINKFLAGS) $(LIBS) -o ${OUTPUT_DIR}/$(@) $^

#all: Add.so AddChannels.so AppendClip.so Assert.so Blocky.so CheckerBoard2.so ColorLookup.so Constant.so Convolve.so CornerPin2D.so Difference.so Dilate.so IDistort.so Grade.so Keymix.so Mirror.so Noise.so Position.so Rectangle.so Remove.so Saturation.so Socket.so Sphere.so TemporalMedian.so crwReader.so dpxReader.so dpxWriter.so movReader.so movWriter.so iffReader.so psdReader.so yuvReader.so yuvWriter.so 
.PRECIOUS : %.os

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $(@) $<

clean:
	rm -rf *.o *.so *.os GridWarp2.so Add.so AddChannels.so AppendClip.so Assert.so Blocky.so CheckerBoard2.so ColorLookup.so Constant.so Convolve.so CornerPin2D.so Difference.so Dilate.so IDistort.so Grade.so Keymix.so Mirror.so Noise.so Position.so Rectangle.so Remove.so Saturation.so Socket.so Sphere.so TemporalMedian.so crwReader.so dpxReader.so dpxWriter.so movReader.so movWriter.so iffReader.so psdReader.so yuvReader.so yuvWriter.so 
