//
// Created by manuel on 28/12/2021.
//
#include "src/Chip8.h"

int main(int argc, char** argv){
    if (argc < 2){
        printf("Indicate ROM file as command line argument");
        return 1;
    }
    char c_exit;

    Chip8 emu;
    emu.loadRom(argv[1]);
    while(true){
        c_exit = std::getchar();
        if (c_exit==27)
            break;
        printf("%02X\n", emu.mem[emu.pc]);
    }
    return 0;
}