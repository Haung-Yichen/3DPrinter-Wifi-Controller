# CMakeLists.txt for STM32 Cryptographic Library (CMOX)
# This file integrates the CMOX library as a subdirectory for the 3DP_Wifi_Controller project.
# It includes necessary headers and links the precompiled library for Cortex-M3 (STM32F103VE).

cmake_minimum_required(VERSION 3.22)

# Define the CMOX library as a static library
add_library(STM32Cryptographic STATIC
    src/cmox_low_level.c
        include/cmox_common.h
        include/cmox_crypto.h
        include/cmox_small_config.h
        include/cmox_cta.h
        include/cmox_fast_config.h
)

# Specify include directories for CMOX
target_include_directories(STM32Cryptographic PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/cipher
    ${CMAKE_CURRENT_SOURCE_DIR}/include/drbg
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ecc
    ${CMAKE_CURRENT_SOURCE_DIR}/include/hash
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mac
    ${CMAKE_CURRENT_SOURCE_DIR}/include/rsa
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utils
)

# Link the precompiled CMOX library for Cortex-M3
target_link_libraries(STM32Cryptographic PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libSTM32Cryptographic_CM3.a
)

# Define necessary compile definitions to enable SHA256
target_compile_definitions(STM32Cryptographic PUBLIC
    CMOX_HASH_SHA256_ENABLE
)

# Ensure CRC module is enabled (required by CMOX)
target_compile_definitions(STM32Cryptographic PUBLIC
    HAL_CRC_MODULE_ENABLED
)

# Specify compiler options for ARM Cortex-M3
target_compile_options(STM32Cryptographic PRIVATE
    -mcpu=cortex-m3
    -mthumb
)