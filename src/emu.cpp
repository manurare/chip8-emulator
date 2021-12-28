#include "emu.h"
#include <bitset>


Chip8::Chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count())
{
    pc = START_ADDRESS;
    for (int i=0; i<FONTSET_SIZE; i++){
        mem[i + FONT_ADDRESS] = fontset[i];
    }

    randByte = std::uniform_int_distribution<uint8_t>(0, 255U);

}


void Chip8::loadRom(char const *filename){
    std::ifstream romFile(filename, std::ios::binary | std::ios::ate); // Set initial position to end of the file

    if (romFile.is_open()){
        int length = romFile.tellg();
        char romBuffer[length];

        romFile.seekg(0, std::ios::beg);

//        printf("%p\n", (void *)romBuffer);
//        printf("%p\n", (void *)&romBuffer[0]);
        romFile.read(romBuffer, length);
        romFile.close();

        for (int i=0; i<length; ++i){
            mem[START_ADDRESS+i] = romBuffer[i];
        }
    }
}

// Clean display. Set all pixels to 0
void Chip8::OP_00E0() {
    std::memset(video, 0, sizeof(video));
}

// RET. Set pc to top of the stack and decrement sp
void Chip8::OP_00EE() {
    --sp;
    pc = stack[sp];
}

// JUMP. Set pc to nnn for next instruction
void Chip8::OP_1nnn() {
    pc = opcode & 0x0FFFu;
}

// CALL. Set top of the stack to pc, increment sp and set pc to nnn
void Chip8::OP_2nnn() {
    stack[sp] = pc;
    ++sp;
    pc = opcode & 0x0FFFu;
}

// Equal to byte. If Vx==cmp then skip
void Chip8::OP_3xkk() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t cmp = opcode & 0x00FFU;
    if (registers[Vx] == cmp){
        pc += 2;
    }
}

// Not equal to byte. If Vx!=cmp then skip
void Chip8::OP_4xkk() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t cmp = opcode & 0x00FFU;
    if (registers[Vx] != cmp){
        pc += 2;
    }
}

// Equal registers. If Vx==Vy then skip
void Chip8::OP_5xy0() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t Vy = (opcode & 0x00F0U) >> 4U;
    if (registers[Vx] == registers[Vy]){
        pc += 2;
    }
}

// Assign byte to Vx
void Chip8::OP_6xkk() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t byte = opcode & 0x00FFU;
    registers[Vx] = byte;
}

// Add byte to Vx
void Chip8::OP_7xkk() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t byte = opcode & 0x00FFU;
    registers[Vx] += byte;
}

// Assign Vy to Vx
void Chip8::OP_8xy0() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t Vy = (opcode & 0x00F0U) >> 4U;
    registers[Vx] = registers[Vy];
}

// Vx OR Vy
void Chip8::OP_8xy1() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t Vy = (opcode & 0x00F0U) >> 4U;
    registers[Vx] |= registers[Vy];
}

// Vx AND Vy
void Chip8::OP_8xy2() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t Vy = (opcode & 0x00F0U) >> 4U;
    registers[Vx] &= registers[Vy];
}

// Vx XOR Vy
void Chip8::OP_8xy3() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t Vy = (opcode & 0x00F0U) >> 4U;
    registers[Vx] ^= registers[Vy];
}

// Vx = Vx + Vy. If > 255 then carry to VF
void Chip8::OP_8xy4() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t Vy = (opcode & 0x00F0U) >> 4U;
    uint16_t sum = registers[Vx] + registers[Vy];

    if (sum > 255u){
        registers[0xF] = 1;
    }
    else{
        registers[0xF] = 0;
    }
    registers[Vx] = sum & 0x00FFu;
}

void Chip8::OP_8xy5() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t Vy = (opcode & 0x00F0U) >> 4U;

    if (registers[Vx] > registers[Vy]){
        registers[0xF] = 1;
    }
    else{
        registers[0xF] = 0;
    }

    registers[Vx] -= registers[Vy];
}

void Chip8::OP_8xy6() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t Vy = (opcode & 0x00F0U) >> 4U;
    registers[0xF] = Vy & 0x1u;
    registers[Vx] = registers[Vy] >> 1u;

}

void Chip8::OP_8xy7() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t Vy = (opcode & 0x00F0U) >> 4U;
    if (registers[Vy] > registers[Vx]){
        registers[0xF] = 1;
    }else
    {
        registers[0xF] = 0;
    }

    registers[Vx] = registers[Vy] - registers[Vx];
}

void Chip8::OP_8xyE() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t Vy = (opcode & 0x00F0U) >> 4U;
    registers[0xF] = (Vy & 0x80u) >> 7U;
    registers[Vx] = registers[Vy] << 1u;

}

void Chip8::OP_9xy0() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t Vy = (opcode & 0x00F0U) >> 4U;
    if (registers[Vx] != registers[Vy]){
        pc += 2;
    }
}

void Chip8::OP_Annn() {
    index = opcode & 0x0FFFu;
}

void Chip8::OP_Bnnn() {
    pc = (opcode & 0x0FFFu) + registers[0];
}

void Chip8::OP_Cxkk() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t byte = opcode & 0x00FFU;
    registers[Vx] = randByte(randGen) & byte;
}

void Chip8::OP_Dxyn() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t Vy = (opcode & 0x00F0U) >> 4U;
    uint8_t N = opcode & 0x000FU;

    // Wrap if going beyond screen boundaries
    uint8_t xCoord = registers[Vx] % VIDEO_WIDTH;
    uint8_t yCoord = registers[Vy] % VIDEO_HEIGHT;

    registers[0xF] = 0;

    for (unsigned int n_byte=0; n_byte<N; ++n_byte){
        uint8_t byte_sprite = mem[index + n_byte];
        for (unsigned int col=0; col<8; ++col){
            uint8_t new_pix = byte_sprite & (0x80u >> col);
            uint32_t* curr_pix = &video[(yCoord + byte_sprite) * VIDEO_WIDTH + xCoord + col];

            if(new_pix){
                if (*curr_pix == 0xFFFFFFFF)
                {
                    registers[0xF] = 1;
                }
            }
            *curr_pix ^= 0xFFFFFFFF;
        }
    }
}


int main(int argc, char** argv){
    Chip8 ey;
    ey.loadRom("../invaders.h");
}