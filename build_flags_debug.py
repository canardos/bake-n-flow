Import("env")

#
# Most of these are PIO defaults, but repeat here for reference
#

# C/C++
env.Append(CCFLAGS=[
     "-ffunction-sections",
    "-fdata-sections",
    "-mthumb",
    "-mcpu=cortex-m3",
    "-mfloat-abi=soft",
    "-march=armv7-m",
    "-Wall",
    "-Og",
    "-DSTM32F103xD",
    "-g2",
    "-ggdb2",
    "-v",
])

# C
#env.Append(CFLAGS=[])

# C++
env.Append(CXXFLAGS=[
    "-std=c++20",
    "-fno-rtti",
    "-fno-exceptions",
    "-fno-use-cxa-atexit",
])

# Linker
env.Append(
    LINKFLAGS=[
        "-Wl,--verbose",
        "-Wl,--no-warn-rwx-segments",
    ]
)
