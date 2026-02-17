set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

set(CMAKE_C_FLAGS "-mcpu=cortex-a9 -mfpu=neon-fp16 -mfloat-abi=hard -Os -g -mthumb")
set(CMAKE_CXX_FLAGS "-mcpu=cortex-a9 -mfpu=neon-fp16 -mfloat-abi=hard -Os -g -mthumb")
