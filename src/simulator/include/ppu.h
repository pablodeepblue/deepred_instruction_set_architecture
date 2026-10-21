//clear && g++ cpp/*.cpp -Iheader -Lcmake-build-debug -o PPU -g -Wall -Wextra -Wshadow -O3 -std=c++17 -lSDL

// windows
// replace x64 with system architeture
//g++ cpp/cpu.cpp cpp/main.cpp cpp/memory.cpp cpp/ui.cpp -o PPU -I./include -L./lib/x64 -g -Wall -Wextra -Wshadow -lmingw32 -lSDL3

#ifndef PPU_H
#define PPU_H
#include "isadefines.h"
#include "memory.h"

#include <stdio.h>
#include <SDL3/SDL.h>
#include <chrono>
#include <thread>

#define FLIP_VERTICAL 64
#define FLIP_HORIZONTAL 128

// Note: in the orignal nes, the program has to wait for interupts to render
// but since we control the cycles, we can assum the PPU is always available to be over written

static char ppu_counter = 0;

constexpr int WIDTH = 256;
constexpr int HEIGHT = 224;
constexpr int SCALE = 4;

const byte_ pallet[] = {
    124, 124, 124,
    0, 0, 252,
    0, 0, 188,
    68, 40, 188,
    148, 0, 132,
    168, 0, 32,
    168, 16, 0,
    136, 20, 0,
    80, 48, 0,
    0, 120, 0,
    0, 104, 0,
    0, 88, 0,
    0, 64, 88,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0,
    188, 188, 188,
    0, 120, 248,
    0, 88, 248,
    104, 68, 252,
    216, 0, 204,
    228, 0, 88,
    248, 56, 0,
    228, 92, 16,
    172, 124, 0,
    0, 184, 0,
    0, 168, 0,
    0, 168, 68,
    0, 136, 136,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0,
    248, 248, 248,
    60, 188, 252,
    104, 136, 252,
    152, 120, 248,
    248, 120, 248,
    248, 88, 152,
    248, 120, 88,
    252, 160, 68,
    248, 184, 0,
    184, 248, 24,
    88, 216, 84,
    88, 248, 152,
    0, 232, 216,
    120, 120, 120,
    0, 0, 0,
    0, 0, 0,
    252, 252, 252,
    164, 228, 252,
    184, 184, 248,
    216, 184, 248,
    248, 184, 248,
    248, 164, 192,
    240, 208, 176,
    252, 224, 168,
    248, 216, 120,
    216, 248, 120,
    184, 248, 184,
    184, 248, 216,
    0, 252, 252,
    248, 216, 248,
    0, 0, 0,
    0, 0, 0,
};

#define TILE_SIZE     4096
#define PALLET_SIZE     32
#define ATR_SIZE        64
#define NAMETABLE_SIZE  960
#define OAM_SIZE        256

constexpr int TILE_T      = MAXMEMORY - TILE_SIZE;
constexpr int PALLET_T    = MAXMEMORY - TILE_SIZE - PALLET_SIZE;
constexpr int ATR_T       = MAXMEMORY - TILE_SIZE - PALLET_SIZE - ATR_SIZE;
constexpr int NAMETABLE_T = MAXMEMORY - TILE_SIZE - PALLET_SIZE - ATR_SIZE - NAMETABLE_SIZE;
constexpr int OAM_T       = MAXMEMORY - TILE_SIZE - PALLET_SIZE - ATR_SIZE - NAMETABLE_SIZE - OAM_SIZE;

class PPU {
    public:
    memory_line* memory = NULL;
    //memory_line* tiles;  // 16x16 8x8 tiles, 16 bytes each
    //memory_line* pallets;  // 4 background (nametable) pallets 4 bytes each (color 1, color 2, color 3, color 4)
        // 4 foreground (OAM) pallets 4 bytes each (color 1, color 2, color 3, color 4)
    //memory_line* atributestable;  // pallets for each tile, 2 bits each
    //memory_line* nametable;  // 32 x 30 tile lables, 1 bytes each
    //memory_line* OAM;  // 64 sprites, 4 bytes each (y-position tile attributes x-position)
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    // OAM formata
    // y-cord: byte_ as pixel value. anything greater than 224 is not rendered. 0 if top, 223 for bottom. >224 is not rendered (offscreen)
    // tile: tile from currently selected tiles (left or right). Only one table can be used per render.
    // attributes: B[0:1] select one of the foreground pallets,  B[6] flip vertically, B[7] flip horizontally
    // x-cord: byte_ as pixel value. Edges not ignored

    // nametable format: Each byte_ points to a location in tiles_l (update in future to allow both)

    PPU ( memory_line* memory_ ) : memory(memory_){}

    inline uint8_t byte_extract(memory_line* base, int offset){
        const int byte_pos = (3 - (offset%4)) << 3;
        return (
            base[offset/32].data[(offset/4) % 8] >> byte_pos
        ) & 0xFF;
    }

    inline uint8_t tiles(int offset){
        return byte_extract(&memory[TILE_T/32], offset);
    }

    inline uint8_t pallets(int offset){
        return byte_extract(&memory[PALLET_T/32], offset);
    }

    inline uint8_t pallets_f(int offset){
        return byte_extract(&memory[PALLET_T/32], offset);
    }

    inline uint8_t pallets_b(int offset){
        return byte_extract(&memory[(PALLET_T + 16)/32], offset);
    }

    inline uint8_t atributestable(int offset){
        return byte_extract(&memory[ATR_T/32], offset);
    }

    inline uint8_t nametabletable(int offset){
        return byte_extract(&memory[NAMETABLE_T/32], offset);
    }

    inline uint8_t OAM(int offset){
        return byte_extract(&memory[OAM_T/32], offset);
    }

    void setup_sdl(){
        SDL_Init(SDL_INIT_VIDEO);
        char s[] = {'P', 'P', 'U', '_', 'O', 'U', 'T','_',  (char)('0' + ppu_counter++), '\0'};
        SDL_CreateWindowAndRenderer(s, WIDTH * SCALE, HEIGHT * SCALE, 0, &window, &renderer);
        SDL_SetRenderScale(renderer, SCALE, SCALE); // 4 times larger than it should be
        SDL_SetWindowResizable(window, true);
    }

    void delete_sdl(){
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        window = NULL;
        renderer = NULL;
        // setup_sdl();
    }



    void render_point(int bits, int pallet_offset, int x, int y, bool background){
        if(background | bits)
        {
            const byte_ *add = &pallet[3*pallets(pallet_offset + bits)];
            SDL_SetRenderDrawColor(renderer, add[0], add[1], add[2], SDL_ALPHA_OPAQUE);
            SDL_RenderPoint(renderer, x, y);
        }
    }

    void output_tile_sprite(byte_ tile_n, int pallet_offset, int upper_right_x, int upper_right_y, bool horizontal_fliped, bool vertical_flip){
        int base = tile_n * 16;
        for (int y = 0; y < 8; y++)
        {
            int y_out = horizontal_fliped ? upper_right_y + 7 - y : upper_right_y + y;
            byte_ lower = tiles(base + y);
            byte_ upper = tiles(base + y + 8);
            for(int x = 7; x >= 0; x-=1){
                int x_out = vertical_flip ? upper_right_x + x : upper_right_x + (7-x);
                byte_ u = (upper & (0b1 << x) ?  0b10 : 0b00);
                byte_ l = (lower & (0b1 << x) ?  0b01 : 0b00);
                // 0 1 2 3
                render_point(u + l, pallet_offset*4, x_out, y_out, false);   
            }
        }
    }
//
    void output_tile_background(byte_ tile_n, int pallet_offset, int upper_right_x, int upper_right_y){
        //printf("%02x ", tile_n);
        int base = tile_n * 16;
        for (int y = 0; y < 8; y++)
        {
            if (upper_right_y + y > HEIGHT) continue;
            byte_ lower = tiles(base + y);
            byte_ upper = tiles(base + y + 8);
            for(int x = 7; x >= 0; x-=1){
                if (upper_right_x + (7-x) > WIDTH) continue;
                byte_ u = (upper & (0b1 << x) ?  0b10 : 0b00);
                byte_ l = (lower & (0b1 << x) ?  0b01 : 0b00);
                render_point(u + l, pallet_offset*4 + 16, upper_right_x + (7-x), upper_right_y + y, true);
            }
        }
    }

    void output_tiles(){
        int t = 0;
        for(int y = 0; y < HEIGHT; y+=8)
        {
            for (int x = 0; x < WIDTH; x+=8)
            {
                if (t > 960) return ;
                else output_tile_background(t++, 0, x, y);
            }
        }
        
    }

    void output_background(){
        int t = 0;
        for(int y = 0; y < HEIGHT; y+=8)
        {
            for (int x = 0; x < WIDTH; x+=8)
            {
                if (t > 960) return ;
                output_tile_background(nametabletable(t++), 0, x, y);
                
            }
        }
        
    }
//
    void output_sprites(){
        for(int i = 0; i < 256; i+=4){
            byte_ y = OAM(i);
            if (y == 0) continue;
            byte_ x = OAM(i+3);
            byte_ tile = OAM(i+1);
            byte_ palletl = OAM(i+2) & 0b11;
            byte_ flip_vertically = (OAM(i+2) & FLIP_VERTICAL ? 1 : 0);
            byte_ flip_horizontally = (OAM(i+2) & FLIP_HORIZONTAL ? 1 : 0);
            output_tile_sprite(tile, palletl, x, y, flip_horizontally, flip_vertically);
        }
    }
//
    void output(){
        if (window == NULL) setup_sdl();
        // setup_sdl();
        output_background();
        output_sprites();
        SDL_RenderPresent(renderer);
    }

    
    bool timelock = false;
    bool wait(int ms){
        static auto start = std::chrono::steady_clock::now();
        if (!timelock) {
            start = std::chrono::steady_clock::now();
            timelock = true;
        }
        auto current = std::chrono::steady_clock::now();
        if (current - start >= std::chrono::milliseconds(ms)) {
            timelock = false;
            return true;
        }
        return false;
    }

    uint32_t pollevent(){
        SDL_Event event;
        if(SDL_PollEvent(&event))
        if (event.type == SDL_EVENT_KEY_DOWN)
            switch (event.key.key)
            {
                case SDLK_X:
                    return 0;
                case SDLK_C:
                    return 1;
                case SDLK_ESCAPE:
                    return 2;
                case SDLK_TAB:
                    return 3;
                case SDLK_UP:
                    return 4;
                case SDLK_DOWN:
                    return 5;
                case SDLK_LEFT:
                    return 6;
                case SDLK_RIGHT:
                    return 7;
                default: break;
            }
        return -1;
    }

    void run(){
        SDL_Event event;
        bool f = true;
        f = true;
        while (f)
        {
            output();
            if (SDL_PollEvent(&event))
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                f = false;
                break;
            default:
                break;
            }
        }
    }
};

#endif