set(LIB_NAME ossimndf_plugin)
MESSAGE( "************** LIBRARY SETUP FOR ossimndf_plugin******************")

find_package(ossim)
find_package(OpenThreads)
find_package(TIFF)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${OPENTHREADS_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIR})

FILE(GLOB OSSIMPLUGIN_SRCS *.cpp)
FILE(GLOB OSSIMPLUGIN_HEADERS *.h)

####
# OSSIMPLUGINSMAKINGDLL controls dll linkage on windows.  
# Adding this sets OSSIM_PLUGINS_DLL #define TO "__declspec(dllexport)".
###
add_definitions("-DOSSIMPLUGINSMAKINGDLL")

OSSIM_LINK_LIBRARY(${LIB_NAME}
                   COMPONENT_NAME ossim TYPE "${OSSIM_PLUGIN_LINK_TYPE}"
		   LIBRARIES ${OSSIM_LIBRARIES} ${OPENTHREADS_LIBRARY} ${TIFF_LIBRARY}
                   HEADERS "${OSSIMPLUGIN_HEADERS}"
		   SOURCE_FILES "${OSSIMPLUGIN_SRCS}"
                   INSTALL_LIB)
