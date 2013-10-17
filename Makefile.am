SUBDIRS = . src

AUTOMAKE_OPTIONS = subdir-objects

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS} build-aux

bin_PROGRAMS = hcell
hcell_SOURCES = \
src/utils.cpp src/utils.h \
src/PModule.cpp src/PModule.h \
src/Action.cpp src/Action.h \
src/ActionLoader.cpp src/ActionLoader.h \
src/ComputeFeatures.cpp src/ComputeFeatures.h src/FeatureCalculator.cpp src/FeatureCalculator.h \
src/FileProcessing.cpp src/FileProcessing.h \
src/General.cpp src/General.h\
src/ImageProcessing.cpp src/ImageProcessing.h \
src/MAllTypes.h src/ModuleSelector.h src/MType.h \
src/PipelineGraph.cpp src/PipelineGraph.h \
src/Pipeline.h src/Pipeline.cpp  \
src/PoolManager.cpp src/PoolManager.h \
src/proctools.cpp src/proctools.h \
src/SegmentationProcessing.cpp src/SegmentationProcessing.h \
src/Slot.cpp src/Slot.h \
src/tinystr.cpp src/tinystr.h src/tinyxml.cpp src/tinyxmlerror.cpp src/tinyxml.h src/tinyxmlparser.cpp \
src/WatershedSegmenter.cpp src/WatershedSegmenter.h \
src/Writer.cpp src/Writer.h \
src/XmlManager.cpp src/XmlManager.h \
src/Hcell.cpp src/Hcell.h 

# the additional libraries needed to link 
hcell_LDADD = $(OPENCV_LIBS) $(BOOST_REGEX_LIB)
hcell_CPPFLAGS = $(OPENCV_CFLAGS) 
# hcell_CXXFLAGS = 
# hcell_LDFLAGS= $(OPENCV_LIBS)

#AM_LDFLAGS = 
AM_CXXFLAGS = -O0 -g3 -Wall -std=c++0x -w -MMD 

EXTRA_DIST = m4/NOTES examples
