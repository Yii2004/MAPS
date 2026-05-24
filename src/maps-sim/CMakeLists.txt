cmake_minimum_required(VERSION 3.16)

project(maps_sim LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(maps_sim
    src/common/bus.cpp
    src/common/memory.cpp
    src/cpu/cpu.cpp
    src/npu/array.cpp
    src/npu/buffer.cpp
    src/npu/controller.cpp
    src/npu/descriptor.cpp
    src/npu/device.cpp
    src/npu/dma.cpp
    src/npu/pe.cpp
    src/npu/schedule.cpp
    src/system/loader.cpp
    src/system/machine.cpp
)

add_library(maps_runtime
    src/runtime/npu_driver.c
    src/runtime/syscall.c
)

target_include_directories(maps_sim PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(maps_runtime PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

enable_testing()

set(MAPS_SIM_TESTS
    test_buffer
    test_dma
    test_scheduler
    test_npu_basic
    test_controller_gemm
    test_controller_readback_dma
    test_end2end_npu
    test_bus_mmio_npu
    test_cpu_basic
    test_cpu_npu_mmio
    test_npu_device_mmio
    test_runtime_npu_driver
    test_machine_npu_demo
    test_loader_raw_binary
    test_loader_elf32
)

foreach(test_name IN LISTS MAPS_SIM_TESTS)
    add_executable(${test_name} tests/${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE maps_sim)
    if(test_name STREQUAL test_runtime_npu_driver)
        target_link_libraries(${test_name} PRIVATE maps_runtime)
    endif()
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

find_program(RISCV_GCC riscv64-unknown-elf-gcc)

if(RISCV_GCC)
    set(MAPS_BAREMETAL_ELF ${CMAKE_CURRENT_BINARY_DIR}/maps_baremetal_npu.elf)

    add_custom_command(
        OUTPUT ${MAPS_BAREMETAL_ELF}
        COMMAND ${RISCV_GCC}
            -march=rv32i
            -mabi=ilp32
            -ffreestanding
            -fno-builtin
            -fno-pic
            -msmall-data-limit=0
            -nostdlib
            -nostartfiles
            -Wl,--no-relax
            -T ${CMAKE_CURRENT_SOURCE_DIR}/examples/baremetal/linker.ld
            -I ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src/runtime/crt0.S
            ${CMAKE_CURRENT_SOURCE_DIR}/examples/baremetal/main.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/runtime/npu_driver.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/runtime/syscall.c
            -o ${MAPS_BAREMETAL_ELF}
        DEPENDS
            src/runtime/crt0.S
            examples/baremetal/main.c
            examples/baremetal/linker.ld
            src/runtime/npu_driver.c
            src/runtime/syscall.c
            include/runtime/npu_driver.h
            include/runtime/syscall.h
        VERBATIM
    )

    add_custom_target(maps_baremetal_npu_elf DEPENDS ${MAPS_BAREMETAL_ELF})

    add_executable(test_baremetal_npu_elf tests/test_baremetal_npu_elf.cpp)
    target_link_libraries(test_baremetal_npu_elf PRIVATE maps_sim)
    target_compile_definitions(test_baremetal_npu_elf PRIVATE
        MAPS_BAREMETAL_ELF="${MAPS_BAREMETAL_ELF}"
    )
    add_dependencies(test_baremetal_npu_elf maps_baremetal_npu_elf)
    add_test(NAME test_baremetal_npu_elf COMMAND test_baremetal_npu_elf)
else()
    message(STATUS "riscv64-unknown-elf-gcc not found; skipping baremetal NPU ELF test")
endif()

set(MAPS_OS_DEMO_ELF ${CMAKE_CURRENT_SOURCE_DIR}/../maps-os/build/maps_os_demo.elf)
if(EXISTS ${MAPS_OS_DEMO_ELF})
    add_executable(test_maps_os_boot tests/test_maps_os_boot.cpp)
    target_link_libraries(test_maps_os_boot PRIVATE maps_sim)
    target_compile_definitions(test_maps_os_boot PRIVATE
        MAPS_OS_DEMO_ELF="${MAPS_OS_DEMO_ELF}"
    )
    add_test(NAME test_maps_os_boot COMMAND test_maps_os_boot)
else()
    message(STATUS "maps_os_demo.elf not found; skipping Mini OS boot test")
endif()

set(MAPS_TENSORC_EXAMPLE_ELF ${CMAKE_CURRENT_SOURCE_DIR}/../maps-os/build/maps_os_tensorc_example.elf)
if(EXISTS ${MAPS_TENSORC_EXAMPLE_ELF})
    add_executable(test_tensorc_os_example tests/test_tensorc_os_example.cpp)
    target_link_libraries(test_tensorc_os_example PRIVATE maps_sim)
    target_compile_definitions(test_tensorc_os_example PRIVATE
        MAPS_TENSORC_EXAMPLE_ELF="${MAPS_TENSORC_EXAMPLE_ELF}"
    )
    add_test(NAME test_tensorc_os_example COMMAND test_tensorc_os_example)
else()
    message(STATUS "maps_os_tensorc_example.elf not found; skipping TensorC OS example test")
endif()
