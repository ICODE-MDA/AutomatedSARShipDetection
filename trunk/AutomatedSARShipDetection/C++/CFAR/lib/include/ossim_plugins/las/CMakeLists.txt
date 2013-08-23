set(LIB_NAME ossimlas_plugin)
MESSAGE( "************** LIBRARY SETUP FOR ossimlas_plugin******************")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

FILE(GLOB_RECURSE OSSIMPLUGIN_SRCS *.cpp)
FILE(GLOB_RECURSE OSSIMPLUGIN_HEADERS *.h)

####
# OSSIMPLUGINSMAKINGDLL controls dll linkage on windows.  
# Adding this sets OSSIM_PLUGINS_DLL #define TO "__declspec(dllexport)".
###
add_definitions("-DOSSIMPLUGINSMAKINGDLL")

OSSIM_LINK_LIBRARY(${LIB_NAME}
                   COMPONENT_NAME ossim TYPE "${OSSIM_PLUGIN_LINK_TYPE}"
		   LIBRARIES ${OSSIM_LIBRARY}
                   HEADERS "${OSSIMPLUGIN_HEADERS}"
		   SOURCE_FILES "${OSSIMPLUGIN_SRCS}"
                   INSTALL_LIB)
