file(GLOB RDC_SOURCE *.cpp *.h *.hpp ../include/rgc/*.h ../include/rgc/*.hpp)

add_library(rgc STATIC ${RDC_SOURCE})
