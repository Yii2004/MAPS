cmake_minimum_required(VERSION 3.16)

project(maps_os LANGUAGES NONE)

find_program(RISCV_GCC riscv64-unknown-elf-gcc REQUIRED)
find_program(PYTHON_EXE python REQUIRED)

set(MAPS_OS_ELF ${CMAKE_CURRENT_BINARY_DIR}/maps_os_demo.elf)
set(MAPS_OS_TENSORC_C ${CMAKE_CURRENT_BINARY_DIR}/generated/tensorc_gemm.c)
set(MAPS_OS_TENSORC_ELF ${CMAKE_CURRENT_BINARY_DIR}/maps_os_tensorc_example.elf)

set(MAPS_OS_COMMON_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime/crt0.S
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/riscv/trap.S
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/kernel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/syscall.c
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/riscv/machine_syscall.c
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/console.c
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/npu.c
    ${CMAKE_CURRENT_SOURCE_DIR}/user/syscall.c
    ${CMAKE_CURRENT_SOURCE_DIR}/user/runtime.c
)

set(MAPS_OS_COMMON_DEPS
    runtime/crt0.S
    arch/riscv/trap.S
    linker/kernel.ld
    kernel/kernel.c
    kernel/syscall.c
    arch/riscv/machine_syscall.c
    drivers/console.c
    drivers/npu.c
    user/syscall.c
    user/runtime.c
    include/maps_os/console.h
    include/maps_os/kernel.h
    include/maps_os/machine_syscall.h
    include/maps_os/npu.h
    include/maps_os/syscall.h
    include/maps_os/types.h
    include/maps_os/user_runtime.h
)

add_custom_command(
    OUTPUT ${MAPS_OS_ELF}
    COMMAND ${RISCV_GCC}
        -march=rv32i_zicsr
        -mabi=ilp32
        -ffreestanding
        -fno-builtin
        -fno-pic
        -msmall-data-limit=0
        -nostdlib
        -nostartfiles
        -Wl,--no-relax
        -T ${CMAKE_CURRENT_SOURCE_DIR}/linker/kernel.ld
        -I ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${MAPS_OS_COMMON_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/user/demo.c
        -o ${MAPS_OS_ELF}
    DEPENDS
        ${MAPS_OS_COMMON_DEPS}
        user/demo.c
    VERBATIM
)

add_custom_target(maps_os_demo ALL DEPENDS ${MAPS_OS_ELF})

add_custom_command(
    OUTPUT ${MAPS_OS_TENSORC_C}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/generated
    COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR}/../maps-compiler
        ${PYTHON_EXE}
        -m tensorc.driver
        ${CMAKE_CURRENT_SOURCE_DIR}/../maps-compiler/examples/gemm.tc
        -o ${MAPS_OS_TENSORC_C}
    DEPENDS
        ../maps-compiler/examples/gemm.tc
        ../maps-compiler/tensorc/driver.py
        ../maps-compiler/tensorc/lexer.py
        ../maps-compiler/tensorc/parser.py
        ../maps-compiler/tensorc/sema.py
        ../maps-compiler/tensorc/codegen_c.py
    VERBATIM
)

add_custom_command(
    OUTPUT ${MAPS_OS_TENSORC_ELF}
    COMMAND ${RISCV_GCC}
        -march=rv32i_zicsr
        -mabi=ilp32
        -ffreestanding
        -fno-builtin
        -fno-pic
        -msmall-data-limit=0
        -nostdlib
        -nostartfiles
        -Wl,--no-relax
        -T ${CMAKE_CURRENT_SOURCE_DIR}/linker/kernel.ld
        -I ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${MAPS_OS_COMMON_SOURCES}
        ${MAPS_OS_TENSORC_C}
        -o ${MAPS_OS_TENSORC_ELF}
    DEPENDS
        ${MAPS_OS_COMMON_DEPS}
        ${MAPS_OS_TENSORC_C}
    VERBATIM
)

add_custom_target(maps_os_tensorc_example ALL DEPENDS ${MAPS_OS_TENSORC_ELF})
