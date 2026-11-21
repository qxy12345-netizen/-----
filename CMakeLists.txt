# =============================================================================
#  64x64 HUB75 点阵显示数字 "3" —— STM32F407VET6（天猛星）
#
#  CMake + Ninja + arm-none-eabi-gcc，零第三方依赖（不用 HAL/CMSIS）。
#
#  命令行用法：
#      cmake --preset debug          # 配置（等价于一键生成 build/）
#      cmake --build --preset debug  # 编译
#      cmake --build --preset debug --target flash   # 编译 + ST-Link 烧录
#
#  产物都在 build/ 下：matrix3.elf / .hex / .bin / .map
# =============================================================================

cmake_minimum_required(VERSION 3.21)

# ---------------------------------------------------------------- 路径检查
# CMake 在 Windows 上处理不了非 ASCII 路径：它会把中文按 ANSI 代码页传给子进程，
# 变成乱码后直接崩（退出码 0xC0000409 或 1，日志里路径显示成"鏂板瀷瑁呯敳鏉"）。
# 放在 project() 之前，这样报错是可控的中文提示，而不是莫名退出。
string(REGEX MATCH "[^ -~]" _non_ascii "${CMAKE_CURRENT_SOURCE_DIR}")
if(_non_ascii)
    # 注意：这里刻意用纯英文。Windows 控制台默认 GBK，中文 message 会变乱码。
    message(FATAL_ERROR
        "\n"
        "========================================================\n"
        " ERROR: project path contains non-ASCII characters.\n"
        " Path : ${CMAKE_CURRENT_SOURCE_DIR}\n"
        "\n"
        " CMake on Windows mangles such paths when calling the\n"
        " compiler, then crashes. It is NOT a code problem.\n"
        "\n"
        " Fix (choose one):\n"
        "   1) Open  E:/VScode/m3  in VSCode instead -- a junction\n"
        "      to this folder with an ASCII path. Already created.\n"
        "   2) Rename this folder to ASCII, e.g. matrix3\n"
        "\n"
        " Details: README section 4.5 (Chinese).\n"
        "========================================================\n")
endif()

project(matrix3 C ASM)

# ---------------------------------------------------------------- 编译参数
# Cortex-M4F，带硬件浮点
set(MCU_FLAGS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 没指定构建类型时默认 Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Debug / Release" FORCE)
endif()

# ---------------------------------------------------------------- 源文件
# 新增 .c 文件记得加到这里（没写 GLOB，是为了让依赖关系显式可见）
set(APP_SOURCES
    src/main.c
    src/board.c
    src/hub75.c
    src/glyph3.c
    src/app.c
    startup/startup_stm32f407xx.s
)

add_executable(${PROJECT_NAME} ${APP_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE inc)

target_compile_options(${PROJECT_NAME} PRIVATE
    ${MCU_FLAGS}
    -Wall
    -Wextra
    -ffunction-sections
    -fdata-sections
)

# 按构建类型给优化等级
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -Og -g3)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -O2 -g3)
endif()

# ---------------------------------------------------------------- 链接
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/STM32F407VET6.ld)

target_link_options(${PROJECT_NAME} PRIVATE
    ${MCU_FLAGS}
    -T${LINKER_SCRIPT}
    -nostartfiles
    -g3
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.map
    -Wl,--print-memory-usage
)

# 改链接脚本也要重新链接，并让产物固定叫 matrix3.elf（Windows 默认不带后缀）
set_target_properties(${PROJECT_NAME} PROPERTIES
    LINK_DEPENDS ${LINKER_SCRIPT}
    SUFFIX ".elf"
)

# ---------------------------------------------------------------- 后处理
# 每次链接完顺手生成 hex / bin，并打印 Flash、RAM 占用
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex
            $<TARGET_FILE:${PROJECT_NAME}>
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary -S
            $<TARGET_FILE:${PROJECT_NAME}>
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
    COMMENT "objcopy -> hex / bin"
)

# ---------------------------------------------------------------- 烧录
# cmake --build --preset debug --target flash
find_program(STM32_CLI NAMES STM32_Programmer_CLI STM32_Programmer_CLI.exe)

if(STM32_CLI)
    add_custom_target(flash
        COMMAND ${STM32_CLI} -c port=SWD -w $<TARGET_FILE:${PROJECT_NAME}> -rst
        DEPENDS ${PROJECT_NAME}
        USES_TERMINAL
        COMMENT "flash via ST-Link"
    )
else()
    add_custom_target(flash
        COMMAND ${CMAKE_COMMAND} -E echo
                "找不到 STM32_Programmer_CLI：请安装 STM32CubeProgrammer，或改用 ST-Link Utility 手动烧 build/matrix3.hex"
        COMMENT "STM32_Programmer_CLI not found"
    )
endif()

# 让 IDE 能直接看到链接脚本
target_sources(${PROJECT_NAME} PRIVATE ${LINKER_SCRIPT})
set_source_files_properties(${LINKER_SCRIPT} PROPERTIES HEADER_FILE_ONLY TRUE)
