# RT1051 platform specific flags
if(NOT DEFINED LINKER_SCRIPTS)
    message(FATAL_ERROR "No linker script defined")
endif(NOT DEFINED LINKER_SCRIPTS)
message("Linker script: ${LINKER_SCRIPTS}")

set(OBJECT_GEN_FLAGS "-mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -fsingle-precision-constant -mno-unaligned-access")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OBJECT_GEN_FLAGS}" CACHE INTERNAL "C Compiler options")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OBJECT_GEN_FLAGS}" CACHE INTERNAL "C++ Compiler options")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${OBJECT_GEN_FLAGS}" CACHE INTERNAL "ASM Compiler options")

# Linker flags
list(TRANSFORM LINKER_SCRIPTS PREPEND -T${CMAKE_CURRENT_SOURCE_DIR}/)
string (REPLACE ";" " " LINKER_SCRIPTS "${LINKER_SCRIPTS}")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostartfiles -mfpu=fpv5-sp-d16 -mfloat-abi=hard ${LINKER_SCRIPTS}" CACHE INTERNAL "Linker options")