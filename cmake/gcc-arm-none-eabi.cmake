# =============================================================================
#  arm-none-eabi-gcc 交叉编译工具链（给 CMake 用）
#
#  CMakePresets.json 里的 toolchainFile 指向本文件。
#  工具链不在 PATH 上时，用 -DTOOLCHAIN_PREFIX=/完整路径/arm-none-eabi- 指定。
# =============================================================================

set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# 交叉编译时 CMake 会先试着编译+链接一个可执行文件来探测编译器，
# 裸机上这一步必然失败，所以改成"只编译成静态库"再探测。
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if(NOT DEFINED TOOLCHAIN_PREFIX)
    set(TOOLCHAIN_PREFIX "arm-none-eabi-" CACHE STRING "工具链前缀")
endif()

set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_OBJCOPY      ${TOOLCHAIN_PREFIX}objcopy CACHE FILEPATH "objcopy")
set(CMAKE_SIZE         ${TOOLCHAIN_PREFIX}size    CACHE FILEPATH "size")

# CMake 配置阶段会链接一个测试程序来探测 ABI。裸机上没有操作系统，
# newlib 的 _write/_sbrk/_exit 这些系统调用没人实现，会链接失败：
#   undefined reference to `_exit' / `_sbrk' ...
# 用 nosys.specs 提供一组空桩，探测就能过。
# （最终固件里这些桩会被 --gc-sections 全部丢弃，不占空间）
set(CMAKE_EXE_LINKER_FLAGS_INIT "-specs=nosys.specs")

# 只在工具链目录里找库和头文件，别把主机的混进来
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
