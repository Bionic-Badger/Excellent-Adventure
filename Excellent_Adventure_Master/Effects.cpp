#include "EffectManager.h"
#include "Effects.h"
#include "EffectUtils.h"
#include "Canvas.h"
#include "Spectrum.h"


int SimpleSpectrum(Canvas *c, EffectManager *em, char mode)
{
    static unsigned n = 0;
    static char currentMode = -1;
    if (mode != currentMode)
    {
        // Performed only once.
        switch(mode)
        {
        case EFFECTMODE_INTRO:
            break;
        case EFFECTMODE_LOOP:
            break;
        case EFFECTMODE_OUTRO:
            break;
        }
        currentMode = mode;
    } else {
        // 0b00011111 = 0x1F
        n++;// = (n+1) & 0x1F;
    }
    
    unsigned short *bands = em->GetSpectrum();

    for (unsigned char y = 0; y < CANVAS_HEIGHT; y++)
    {
        unsigned high;
        Color_t color;
        switch (y)
        {
        case 0:     // Yellow
            color = COLOR_B(31,31,0);
            high = 900;
            break;
        case 1:
            color = COLOR_B(23,23,0);
            high = 800;
            break;
        case 2:
            color = COLOR(31,31,0);
            high = 700;
            break;
        case 3:     // Green
            color = COLOR_B(0,31,0);
            high = 600;
            break;
        case 4:
            color = COLOR_B(0,23,0);
            high = 500;
            break;
        case 5:
            color = COLOR(0,31,0);
            high = 400;
            break;
        case 6:     // Red
            color = COLOR_B(31,0,0);
            high = 300;
            break;
        case 7:
            color = COLOR_B(23,0,0);
            high = 200;
            break;
        case 8:
            color = COLOR(31,0,0);
            high = 100;
            break;
        case 9:
            color = COLOR(16,0,0);
            high = 0;
            break;
        default: 
            color = 0;
        }

        for (unsigned char x = 0; x < CANVAS_WIDTH; x++)
        {
            c->PutPixel(x, y, (bands[x + 1] > high) ? color : 0);
        }
    }
    return 1;
}

int WarpSpectrum(Canvas *c, EffectManager *em, char mode)
{
    static uint8_t ci = 0;
    static unsigned short pos[CANVAS_WIDTH];
    //static          short vel[CANVAS_WIDTH];
    
    static char currentMode = -1;
    if(mode != currentMode){  // performed only once
        switch(mode){
            case EFFECTMODE_INTRO:
                for(uint8_t x = 0; x < CANVAS_WIDTH; x++){
                    pos[x] = CANVAS_HM1;
                    for(uint8_t y = 0; y < CANVAS_HEIGHT; y++)
                        c->PutPixel(x, y, 0);
                }
                break;
        }
        currentMode = mode;
    }
    else{  // step
        unsigned short *spectrum = em->GetSpectrum();
        for(uint8_t x = 0; x < CANVAS_WIDTH; x++){
            short spc = spectrum[(x >> 1) + 3];
            if(spc > 800)
                ci = MOD32(ci + 1);
            pos[x] -= (spc - 150) * 10;
        }
    }

    for(uint8_t x = 0; x < CANVAS_WIDTH; x++){
        uint8_t px = pos[x] / 6554;
        for(uint8_t y = 0; y < CANVAS_HEIGHT; y++){
            if(y == px){  // shot of color
                c->PutPixel(x, y, lerpColor(c->GetPixel(x, y), colorwheel_lut[ci], 0.5));
            }
            else if(y == px - 1 || y == px + 1){  // shot of color
                c->PutPixel(x, y, lerpColor(c->GetPixel(x, y), colorwheel_lut[ci], 0.1));
            }
            else{  // fade out
                Color_t px_color = c->GetPixel(x, y);
                c->PutPixel(x, y, COLOR_B(
                    MAX(0, char(RED(px_color)) - 2),
                    MAX(0, char(GREEN(px_color)) - 2),
                    MAX(0, char(BLUE(px_color)) - 2)
                ));
            }
        }
    }
    
    return 1;
}

int ElevatorSpectrum(Canvas *c, EffectManager *em, char mode)
{
    static unsigned short pos = 0;
    static          short vel = 0;
    
    // step
    unsigned short level = em->GetSpectrum()[4];
    vel *= 0.95;
    vel += level;
    pos -= vel;

    for(uint8_t y = 0; y < CANVAS_HEIGHT; y++){
        Color_t color;
        if(vel > 10000){
            uint8_t cw_pos = (pos + (y << 12)) >> 11;
            color = colorwheel_lut[cw_pos];
        }
        else{
            uint8_t sin_pos = (pos + (y << 13)) >> 11;  // 65536 -> 32
            uint8_t gs = sin_lut[sin_pos] >> 3;  // 256 -> 32
            color = COLOR_B(gs, gs, gs);
        }
        for(uint8_t x = 0; x < CANVAS_WIDTH; x++){
            c->PutPixel(x, y, color);
        }
    }
    
    return 1;
}

int PinwheelSpectrum(Canvas *c, EffectManager *em, char mode)
{
    static int step = 0;
    
    static int ctrx = (CANVAS_WM1 * 100) / 2;
    static int ctry = (CANVAS_HM1 * 100) / 2;
    
    for(char y = 0; y < CANVAS_HEIGHT; y++){
        for(char x = 0; x < CANVAS_WIDTH; x++){
            int angle = MOD32(int(atan2(y * 100 - ctry, x * 100 - ctrx) * TWO_PI_TO_32) + step);
            c->PutPixel(x, y, colorwheel_lut[angle]);
        }
    }
    
    step += 2;
}

// int SolidDot(Canvas *c, EffectManager *em, char mode)
// {
//     static int step;
//     static Color_t color;
//     static unsigned short posx, posy;
//     
//     static char currentMode = -1;
//     if(mode != currentMode){  // performed only once
//         switch(mode){
//             case EFFECTMODE_INTRO:
//                 step = 0;
//                 
//                 break;
//         }
//         currentMode = mode;
//     }
//     else{  // step
//         unsigned short *spectrum = em->GetSpectrum();
//         for(uint8_t x = 0; x < CANVAS_WIDTH; x++){
//             short spc = spectrum[(x >> 1) + 3];
//             if(spc > 800)
//                 ci = MOD32(ci + 1);
//             pos[x] -= (spc - 150) * 10;
//         }
//     }
//     
//     for(char x = 0; x < CANVAS_WIDTH; x += 2){
//         
//         for(char y = 0; y < CANVAS_HEIGHT; y++){
//             r = MAX(0.0f, 4.0f - dist(x, y, rlx, rly)) * 0x1F;
//             g = MAX(0.0f, 3.0f - dist(x, y, glx, gly)) * 0x1F;
//             b = MAX(0.0f, 4.0f - dist(x, y, blx, bly)) * 0x1F;
//             c->PutPixel(x, y, COLOR_B(MIN(r, 0x1F), MIN(g, 0x1f), MIN(b, 0x1f)));
//         }
//     }
// 
//     step++;
// 
//     return 1;
// }


int SimpleColumns(Canvas *c, EffectManager *em, char mode)
{
    static Channel_t n = 0;
    static char currentMode = -1;
    if (mode != currentMode)
    {
        // Performed only once.
        switch(mode)
        {
        case EFFECTMODE_INTRO:
            n = 0;
            break;
        case EFFECTMODE_LOOP:
            break;
        case EFFECTMODE_OUTRO:
            break;
        }
        currentMode = mode;
    } else {
        // 0b00011111 = 0x1F
        n = (n+1) & 0x1F;
    }


    for (unsigned char y = 0; y < CANVAS_HEIGHT; y++)
    {
        // Create ascending values
        // [0, 
        
        Channel_t nn = (n+y) & 0x1F;
        /*
        uint32_t color = COLOR(0,nn,0);
        c->PutPixel(0, y, color);
        c->PutPixel(2, y, color);
        c->PutPixel(3, y, color);
        c->PutPixel(5, y, color);
        c->PutPixel(6, y, color);
        c->PutPixel(8, y, color);
        */
        /*
        Color_t color = COLOR_B(31,31,31);
        c->PutPixel(0, y, color);
        c->PutPixel(1, y, color);
        c->PutPixel(2, y, color);
        c->PutPixel(3, y, color);
        c->PutPixel(4, y, color);
        c->PutPixel(5, y, color);
        */

        
        c->PutPixel(0, y, COLOR(nn,0,0));
        c->PutPixel(1, y, COLOR(0,nn,0));

        c->PutPixel(2, y, COLOR(0,0,nn));


        c->PutPixel(3, y, COLOR_B(nn,0,0));

        c->PutPixel(4, y, COLOR_B(0,nn,0));
        c->PutPixel(5, y, COLOR_B(0, 0,nn));
        
    }
    return 1;
}

int Spotlight(Canvas *c, EffectManager *em, char mode)
{
    static int step = 0;
    
    float rlx = sin_lut[MOD32(step + 8)] / 255.0f * CANVAS_WM1;
    float rly = sin_lut[MOD32(step)] / 255.0f * CANVAS_HM1;
    
    float glx = sin_lut[MOD32(-step + 8)] / 255.0f * CANVAS_WM1;
    float gly = sin_lut[MOD32(-step)] / 255.0f * CANVAS_HM1;
    
    float blx = sin_lut[MOD32(step + 24)] / 255.0f * CANVAS_WM1;
    float bly = sin_lut[MOD32(step + 16)] / 255.0f * CANVAS_HM1;
    
    uint8_t r, g, b;
    for(char y = 0; y < CANVAS_HEIGHT; y++){
        for(char x = 0; x < CANVAS_WIDTH; x++){
            r = MAX(0.0f, 4.0f - dist(x, y, rlx, rly)) * 0x1F;
            g = MAX(0.0f, 3.0f - dist(x, y, glx, gly)) * 0x1F;
            b = MAX(0.0f, 4.0f - dist(x, y, blx, bly)) * 0x1F;
            c->PutPixel(x, y, COLOR_B(MIN(r, 0x1F), MIN(g, 0x1f), MIN(b, 0x1f)));
        }
    }
    
    step++;
    
    return 1;
}

int CheckerBoard(Canvas *c, EffectManager *em, char mode)
{
    // Sub-pixel
    static unsigned char offsetX = 0;

    static unsigned char offsetY = 0;

    static Channel_t n = 0;
    static char currentMode = -1;
    if (mode != currentMode)
    {
        // Performed only once.
        switch(mode)
        {
        case EFFECTMODE_INTRO:
            n = 0;
            break;
        case EFFECTMODE_LOOP:
            break;
        case EFFECTMODE_OUTRO:
            break;
        }
        currentMode = mode;
    } else {
        // 0b00011111 = 0x1F
        n = (n+1) & 0x1F;
    }

    Color_t color = COLOR(n,n,n);

    for (unsigned char y = 0; y < CANVAS_HEIGHT; y++)
    {
        char offset = offsetX+offsetY+y;
        for (unsigned char x = 0; x < CANVAS_WIDTH; x++)
        {
            bool draw = ((x+offset) & 0x1) == 0;
            if (draw)
            {
                c->PutPixel(x,y, color);
            } else {
                c->PutPixel(x,y, 0);
            }
        }
        
    }
    if (n == 0)
    {
        offsetX++;
        //if ((offsetX & 3) == 3)
        //{
        //    offsetY++;
        //}
    }
    return 1;
}


int BlitzyIdle(Canvas *c, EffectManager *em, char mode)
{
    static bool n = false;

    n = !n;
    c->Clear(0);
    c->ClearCeiling(COLOR_CEILING(192,0));
    if (em->GetRandomNumber() > 0xC000)
    {
        c->PutPixelCeiling(1, 0);
    }
    if (em->GetRandomNumber() > 0xD000)
    {
        c->PutPixelCeiling(4, COLOR_CEILING(255,0));
    }
    return 1;
}


int RingFlash(Canvas *c, EffectManager *em, char mode)
{
    static bool bright = false;
    if (bright)
    {
        c->Clear(COLOR(31,31,31));
    } else {
        c->Clear(0);
    }
    bright = !bright;
/*
    for (unsigned char y = 0; y < CANVAS_HEIGHT; y++)
    {
        Color_t color = em->GetRandomNumber();//COLOR(r,g,b);
        for (unsigned char x = 0; x < CANVAS_WIDTH; x++)
        {
            c->PutPixel(x,y, color);
        }
    }
*/
    return 1;
}


int Overtime(Canvas *c, EffectManager *em, char mode)
{
    c->ClearCeiling(COLOR_CEILING(0,255));
    c->Clear(0);
    return 1;
}
