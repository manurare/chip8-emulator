//
// Created by manuel on 28/12/2021.
//
#include "src/Chip8.h"


bool readInput(Chip8*, SDL_Event*);
bool initGraphics(SDL_Texture** tex, SDL_Window** win, SDL_Renderer** ren);

int main(int argc, char** argv){
    if (argc < 2){
        printf("Indicate ROM file as command line argument");
        return 1;
    }

    SDL_Event e;
    SDL_Window* window{};
    SDL_Renderer* renderer{};
    SDL_Texture* texture{};
    SDL_Init(SDL_INIT_VIDEO);
    if (!initGraphics(&texture, &window, &renderer)) return -1;

    Chip8 emu;
    emu.loadRom(argv[1]);
    bool quit;

    auto old_time = std::chrono::system_clock::now();
    auto c_time = std::chrono::system_clock::now();
    std::chrono::duration<double> diff{};
    while(!quit){
        quit = readInput(&emu, &e);
        c_time = std::chrono::system_clock::now();
        diff = c_time - old_time;
        if (diff.count() < 5e-3)
            continue;
        else old_time = c_time;
        emu.run();

//        if (emu.drawFlag) {
//        }
//        emu.drawFlag = false;
        SDL_UpdateTexture(texture, nullptr, emu.video, VIDEO_WIDTH*4);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);


    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool initGraphics(SDL_Texture** tex, SDL_Window** win, SDL_Renderer** ren){
    *win = SDL_CreateWindow("Chip-8",0, 0, VIDEO_WIDTH*10, VIDEO_HEIGHT*10, SDL_WINDOW_SHOWN);
    if (*win == nullptr){
        std::cout << SDL_GetError() << std::endl;
        return false;
    }

    *ren = SDL_CreateRenderer(*win, -1, SDL_RENDERER_ACCELERATED );
    if (*ren == nullptr){
        std::cout << SDL_GetError() << std::endl;
        return false;
    }

    *tex = SDL_CreateTexture(*ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, VIDEO_WIDTH, VIDEO_HEIGHT);
    if (*tex == nullptr){
        std::cout << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

bool readInput(Chip8* chip8, SDL_Event* e){
    while(SDL_PollEvent(e) != 0){
        //User presses a key
        switch( e->type )
        {
            case SDL_QUIT:
                return true;

            case SDL_KEYDOWN:
                switch( e->key.keysym.sym )
                {
                    case SDLK_ESCAPE:
                        return -1;

                    case SDLK_1:
                        chip8->keypad[0] = 1;
                        break;

                    case SDLK_2:
                        chip8->keypad[1] = 1;
                        break;

                    case SDLK_3:
                        chip8->keypad[2] = 1;
                        break;

                    case SDLK_4:
                        chip8->keypad[3] = 1;
                        break;

                    case SDLK_q:
                        chip8->keypad[4] = 1;
                        break;

                    case SDLK_w:
                        chip8->keypad[5] = 1;
                        break;

                    case SDLK_e:
                        chip8->keypad[6] = 1;
                        break;

                    case SDLK_r:
                        chip8->keypad[7] = 1;
                        break;

                    case SDLK_a:
                        chip8->keypad[8] = 1;
                        break;

                    case SDLK_s:
                        chip8->keypad[9] = 1;
                        break;

                    case SDLK_d:
                        chip8->keypad[10] = 1;
                        break;

                    case SDLK_f:
                        chip8->keypad[11] = 1;
                        break;

                    case SDLK_z:
                        chip8->keypad[12] = 1;
                        break;

                    case SDLK_x:
                        chip8->keypad[13] = 1;
                        break;

                    case SDLK_c:
                        chip8->keypad[14] = 1;
                        break;

                    case SDLK_v:
                        chip8->keypad[15] = 1;
                        break;
                }
                break;

            case SDL_KEYUP:
                switch( e->key.keysym.sym )
                {
                    case SDLK_1:
                        chip8->keypad[0] = 0;
                        break;

                    case SDLK_2:
                        chip8->keypad[1] = 0;
                        break;

                    case SDLK_3:
                        chip8->keypad[2] = 0;
                        break;

                    case SDLK_4:
                        chip8->keypad[3] = 0;
                        break;

                    case SDLK_q:
                        chip8->keypad[4] = 0;
                        break;

                    case SDLK_w:
                        chip8->keypad[5] = 0;
                        break;

                    case SDLK_e:
                        chip8->keypad[6] = 0;
                        break;

                    case SDLK_r:
                        chip8->keypad[7] = 0;
                        break;

                    case SDLK_a:
                        chip8->keypad[8] = 0;
                        break;

                    case SDLK_s:
                        chip8->keypad[9] = 0;
                        break;

                    case SDLK_d:
                        chip8->keypad[10] = 0;
                        break;

                    case SDLK_f:
                        chip8->keypad[11] = 0;
                        break;

                    case SDLK_z:
                        chip8->keypad[12] = 0;
                        break;

                    case SDLK_x:
                        chip8->keypad[13] = 0;
                        break;

                    case SDLK_c:
                        chip8->keypad[14] = 0;
                        break;

                    case SDLK_v:
                        chip8->keypad[15] = 0;
                        break;
                }
                break;
        }

    }
    return false;
}