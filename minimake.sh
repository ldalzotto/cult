mkdir ./build
gcc -march=native -DDEBUG_ASSERTIONS_ENABLED=0 -DNDEBUG -O3 -fdata-sections -ffunction-sections -Wl,--gc-sections -Isrc/libs ./tools/minimake/minimake.amalgamated.c \
    -o ./build/minimake
