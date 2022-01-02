#include "Chip8.h"
#include <bitset>


Chip8::Chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count())
{
    pc = START_ADDRESS;
    for (unsigned int i=0; i<FONTSET_SIZE; ++i){
        mem[i + FONT_ADDRESS] = fontset[i];
    }

    randByte = std::uniform_int_distribution<uint8_t>(0, 255U);
    drawFlag = false;
}


void Chip8::loadRom(char const *filename){
    std::ifstream romFile(filename, std::ios::binary | std::ios::ate); // Set initial position to end of the file

    if (romFile.is_open()){
        int length = romFile.tellg();
        char romBuffer[length];

        romFile.seekg(0, std::ios::beg);

        romFile.read(romBuffer, length);
        romFile.close();

        for (int i=0; i<length; ++i){
            mem[START_ADDRESS+i] = (unsigned int)romBuffer[i];
        }
    }
}

// Clean display. Set all pixels to 0
void Chip8::OP_00E0() {
    std::memset(video, 0, sizeof(video));
    pc += 2;
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
    pc += 2;
    stack[sp] = pc;
    ++sp;
    pc = opcode & 0x0FFFu;
}

// Equal to byte. If Vx==cmp then skip
void Chip8::OP_3xkk() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t cmp = opcode & 0x00FFU;
    if (registers[Vx] == cmp){
        pc += 4;
    }else
    {
        pc += 2;
    }
}

// Not equal to byte. If Vx!=cmp then skip
void Chip8::OP_4xkk() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t cmp = opcode & 0x00FFU;
    if (registers[Vx] != cmp){
        pc += 4;
    }else
    {
        pc += 2;
    }
}

// Equal registers. If Vx==Vy then skip
void Chip8::OP_5xy0() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t Vy = (opcode & 0x00F0U) >> 4U;
    if (registers[Vx] == registers[Vy]){
        pc += 4;
    }else
    {
        pc += 2;
    }
}

// Assign byte to Vx
void Chip8::OP_6xkk() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t byte = opcode & 0x00FFU;
    registers[Vx] = byte;
    pc += 2;
}

// Add byte to Vx
void Chip8::OP_7xkk() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t byte = opcode & 0x00FFU;
    registers[Vx] += byte;
    pc += 2;
}

// Assign Vy to Vx
void Chip8::OP_8xy0() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t Vy = (opcode & 0x00F0U) >> 4U;
    registers[Vx] = registers[Vy];
    pc += 2;
}

// Vx OR Vy
void Chip8::OP_8xy1() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t Vy = (opcode & 0x00F0U) >> 4U;
    registers[Vx] |= registers[Vy];
    pc += 2;
}

// Vx AND Vy
void Chip8::OP_8xy2() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t Vy = (opcode & 0x00F0U) >> 4U;
    registers[Vx] &= registers[Vy];
    pc += 2;
}

// Vx XOR Vy
void Chip8::OP_8xy3() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t Vy = (opcode & 0x00F0U) >> 4U;
    registers[Vx] ^= registers[Vy];
    pc += 2;
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
    pc += 2;
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
    pc += 2;
}

//void Chip8::OP_8xy6() {
//    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
//    uint8_t Vy = (opcode & 0x00F0U) >> 4U;
//    registers[0xF] = Vy & 0x1u;
//    registers[Vx] = registers[Vy] >> 1u;
//    pc += 2;
//}

void Chip8::OP_8xy6()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    // Save LSB in VF
    registers[0xF] = (registers[Vx] & 0x1u);

    registers[Vx] >>= 1u;
    pc += 2;
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
    pc += 2;
}

//void Chip8::OP_8xyE() {
//    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
//    uint8_t Vy = (opcode & 0x00F0U) >> 4U;
//    registers[0xF] = (Vy & 0x80u) >> 7U;
//    registers[Vx] = registers[Vy] << 1u;
//    pc += 2;
//
//}

void Chip8::OP_8xyE()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    // Save MSB in VF
    registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

    registers[Vx] <<= 1u;
    pc += 2;
}


void Chip8::OP_9xy0() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t Vy = (opcode & 0x00F0U) >> 4U;
    if (registers[Vx] != registers[Vy]){
        pc += 4;
    }
    else{
        pc += 2;
    }
}

void Chip8::OP_Annn() {
    index = opcode & 0x0FFFu;
    pc += 2;
}

void Chip8::OP_Bnnn() {
    pc = (opcode & 0x0FFFu) + registers[0];
}

void Chip8::OP_Cxkk() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t byte = opcode & 0x00FFU;
    registers[Vx] = randByte(randGen) & byte;
    pc += 2;
}

void Chip8::OP_Dxyn() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t Vy = (opcode & 0x00F0u) >> 4U;
    uint8_t N = opcode & 0x000FU;

    // Wrap if going beyond screen boundaries
    uint8_t xCoord = registers[Vx] % VIDEO_WIDTH;
    uint8_t yCoord = registers[Vy] % VIDEO_HEIGHT;

    registers[0xF] = 0;

    for (unsigned int n_byte=0; n_byte<N; ++n_byte){
        uint8_t byte_sprite = mem[index + n_byte];
        for (unsigned int col=0; col<8; ++col){
            uint8_t new_pix = byte_sprite & (0x80u >> col);
            uint32_t* curr_pix = &video[(yCoord + n_byte) * VIDEO_WIDTH + xCoord + col];

            if(new_pix){
                if (*curr_pix == 0xFFFFFFFF)
                {
                    registers[0xF] = 1;
                }
                *curr_pix ^= 0xFFFFFFFF;
            }
        }
    }
    drawFlag = true;
    pc += 2;
}

void Chip8::OP_Ex9E() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t key = registers[Vx];

    if (keypad[key]) pc += 4;
    else pc += 2;

}

void Chip8::OP_ExA1() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t key = registers[Vx];

    if (!keypad[key]) pc += 4;
    else pc += 2;
}

void Chip8::OP_Fx07() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;

    registers[Vx] = delayTimer;
    pc += 2;

}

void Chip8::OP_Fx0A() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;

    if (keypad[0])
    {
        registers[Vx] = 0;
        pc += 2;
    }
    else if (keypad[1])
    {
        registers[Vx] = 1;
        pc += 2;
    }
    else if (keypad[2])
    {
        registers[Vx] = 2;
        pc += 2;
    }
    else if (keypad[3])
    {
        registers[Vx] = 3;
        pc += 2;
    }
    else if (keypad[4])
    {
        registers[Vx] = 4;
        pc += 2;
    }
    else if (keypad[5])
    {
        registers[Vx] = 5;
        pc += 2;
    }
    else if (keypad[6])
    {
        registers[Vx] = 6;
        pc += 2;
    }
    else if (keypad[7])
    {
        registers[Vx] = 7;
        pc += 2;
    }
    else if (keypad[8])
    {
        registers[Vx] = 8;
        pc += 2;
    }
    else if (keypad[9])
    {
        registers[Vx] = 9;
        pc += 2;
    }
    else if (keypad[10])
    {
        registers[Vx] = 10;
        pc += 2;
    }
    else if (keypad[11])
    {
        registers[Vx] = 11;
        pc += 2;
    }
    else if (keypad[12])
    {
        registers[Vx] = 12;
        pc += 2;
    }
    else if (keypad[13])
    {
        registers[Vx] = 13;
        pc += 2;
    }
    else if (keypad[14])
    {
        registers[Vx] = 14;
        pc += 2;
    }
    else if (keypad[15])
    {
        registers[Vx] = 15;
        pc += 2;
    }
    else
    {
        pc -= 2;
    }
}

void Chip8::OP_Fx15() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;

    delayTimer = registers[Vx];
    pc += 2;
}

void Chip8::OP_Fx18() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;

    soundTimer = registers[Vx];
    pc += 2;
}

void Chip8::OP_Fx1E() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;

    index += registers[Vx];
    pc += 2;

}

void Chip8::OP_Fx29() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    uint8_t sprite_data = registers[Vx];

    index += FONT_ADDRESS + sprite_data*5;
    pc += 2;

}

void Chip8::OP_Fx33() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    mem[index] =   (registers[Vx] / 100) % 10;
    mem[index+1] = (registers[Vx] / 10)  % 10;
    mem[index+2] =  registers[Vx] % 10;
    pc += 2;

}

void Chip8::OP_Fx55() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    for (unsigned int i=0; i<=Vx; ++i) mem[index + i] = registers[i];
//    index += Vx + 1;
    pc += 2;
}

void Chip8::OP_Fx65() {
    uint8_t Vx = (opcode & 0x0F00U) >> 8U;
    for (unsigned int i=0; i<=Vx; ++i) registers[i] = mem[index + i];
//    index += Vx + 1;
    pc += 2;
}

void Chip8::run() {
    opcode = uint16_t(mem[pc] << 8) | uint16_t(mem[pc+1]);
//    printf("0x%04X\n", mem[pc + 1] << 8);
//    printf("0x%04X\n", mem[pc]);
    printf("opcode = 0x%04X\n", opcode);
//    printf("pc = 0x%X\n", pc);

    switch(opcode & 0xF000u){
        case 0x1000:
            OP_1nnn();
            break;
        case 0x2000:
            OP_2nnn();
            break;
        case 0x3000:
            OP_3xkk();
            break;
        case 0x4000:
            OP_4xkk();
            break;
        case 0x5000:
            OP_5xy0();
            break;
        case 0x6000:
            OP_6xkk();
            break;
        case 0x7000:
            OP_7xkk();
            break;
        case 0x9000:
            OP_9xy0();
            break;
        case 0xA000:
            OP_Annn();
            break;
        case 0xB000:
            OP_Bnnn();
            break;
        case 0xC000:
            OP_Cxkk();
            break;
        case 0xD000:
            OP_Dxyn();
            break;
        case 0x8000:
            switch (opcode & 0x000Fu) {
                case 0x0000:
                    OP_8xy0();
                    break;
                case 0x0001:
                    OP_8xy1();
                    break;
                case 0x0002:
                    OP_8xy2();
                    break;
                case 0x0003:
                    OP_8xy3();
                    break;
                case 0x0004:
                    OP_8xy4();
                    break;
                case 0x0005:
                    OP_8xy5();
                    break;
                case 0x0006:
                    OP_8xy6();
                    break;
                case 0x0007:
                    OP_8xy7();
                    break;
                case 0x000E:
                    OP_8xyE();
                    break;
                default:
                    pc += 2;
                    break;
            }
            break;
        case 0x0000:
            switch (opcode & 0x00FFu) {
                case 0x00E0:
                    OP_00E0();
                    break;
                case 0x00EE:
                    OP_00EE();
                    break;
                default:
                    pc += 2;
                    break;
            }
            break;
        case 0xE000:
            switch (opcode & 0x00FFu) {
                case 0x00A1:
                    OP_ExA1();
                    break;
                case 0x009E:
                    OP_Ex9E();
                    break;
                default:
                    pc += 2;
                    break;
            }
            break;
        case 0xF000:
            switch (opcode & 0x00FFu) {
                case 0x0007:
                    OP_Fx07();
                    break;
                case 0x000A:
                    OP_Fx0A();
                    break;
                case 0x0015:
                    OP_Fx15();
                    break;
                case 0x0018:
                    OP_Fx18();
                    break;
                case 0x001E:
                    OP_Fx1E();
                    break;
                case 0x0029:
                    OP_Fx29();
                    break;
                case 0x0033:
                    OP_Fx33();
                    break;
                case 0x0055:
                    OP_Fx55();
                    break;
                case 0x0065:
                    OP_Fx65();
                    break;
                default:
                    pc += 2;
                    break;
            }
            break;
        default:
            printf("Unknown opcode 0x%X\n", opcode);
            pc += 2;
            break;
    }

    if (delayTimer > 0) delayTimer--;
    if (soundTimer > 0) soundTimer--;
}
