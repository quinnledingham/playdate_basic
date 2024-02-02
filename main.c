#ifdef _WINDLL
__declspec(dllexport)
#endif


#include <stdio.h>
#include <stdlib.h>
//#include <ctype.h>
//#include <math.h>

#include "pd_api.h"


#include "defines.h"
#include "types.h"
#include "types_math.c"
#include "render.h"

#include "shapes.cpp"

struct Data {
    LCDFont* font;
    struct Mesh mesh;

    union Camera camera;
    Matrix_4x4 model;
    Matrix_4x4 projection;
    Matrix_4x4 view;
} data;

#define TEXT_WIDTH 86
#define TEXT_HEIGHT 16

int x = (400-TEXT_WIDTH)/2;
int y = (240-TEXT_HEIGHT)/2;
int dx = 1;
int dy = 2;

static const char *str = "Hello World! Ohhh hGmaer";

internal Vector2_s32
get_viewport(float32 x, float32 y, 
             s32 x_wmax, s32 y_wmax, s32 x_wmin, s32 y_wmin,
             s32 x_vmax, s32 y_vmax, s32 x_vmin, s32 y_vmin) {
    Vector2_s32 result;
    float32 sx, sy;
    sx = (float32)(x_vmax - x_vmin) / (x_wmax - x_wmin);
    sy = (float32)(y_vmax - y_vmin) / (y_wmax - y_wmin);

    result.x = x_vmin + (float32)((x - (float32)x_wmin) * sx);
    result.y = y_vmin + (float32)((y - (float32)y_wmin) * sy);
    return result;
}

internal Matrix_4x4
get_viewport_4x4(float32 x, float32 y, float32 z) {
    Matrix_4x4 result =  {
        x/2.0f, 0, 0, (x - 1.0f)/2.0f,
        0, y/2.0f, 0, (y - 1.0f)/2.0f,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    return result;
}

internal Vector2_s32
vertex_get_pos(struct Vertex_XNU v) {
    Vector3 v3 = v.position;
    Vector4 v4 = { v3.x, v3.y, v3.z, 1.0f };
    Vector4 pos = m4x4_mul_v4(data.projection, m4x4_mul_v4(data.view, m4x4_mul_v4(data.model, v4)));
    pos.x = pos.x / pos.w;
    pos.y = pos.y / pos.w;
    pos.z = pos.z / pos.w;
    pos.w = pos.w / pos.w;
    Vector2_s32 result = get_viewport(pos.x, pos.y, 
                                    1, 1, -1, -1,
                                      LCD_COLUMNS, LCD_ROWS, 0, 0
                                      );
    return result;
}

LCDPattern test;

LCDPattern grey50 = {
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, // Bitmap, each byte is a row of pixel
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Mask, here fully opaque
};

internal void
draw_mesh(PlaydateAPI* pd, struct Mesh *mesh) {
    struct Vertex_XNU *vertices = mesh->vertices;
    for (u32 i = 0; i < mesh->indices_count; i += 3) {
        Vector2_s32 a = vertex_get_pos(vertices[mesh->indices[i]]);
        Vector2_s32 b = vertex_get_pos(vertices[mesh->indices[i + 1]]);
        Vector2_s32 c = vertex_get_pos(vertices[mesh->indices[i + 2]]);
        pd->graphics->fillTriangle(a.x, a.y, c.x, c.y, b.x, b.y, grey50);
    }
}

internal int 
update(void* userdata) {
    PlaydateAPI* pd = userdata;
    
    pd->graphics->clear(kColorWhite);
    pd->graphics->setFont(data.font);
    pd->graphics->drawText(str, strlen(str), kASCIIEncoding, x, y);

    x += dx;
    y += dy;
    
    if ( x < 0 || x > LCD_COLUMNS - TEXT_WIDTH )
        dx = -dx;
    
    if ( y < 0 || y > LCD_ROWS - TEXT_HEIGHT )
        dy = -dy;
        
    pd->system->drawFPS(0,0);

    PDButtons down;
    pd->system->getButtonState(&down, 0, 0);

    Vector3 magnitude = { 0.5f, 0.5f, 0.5f };

    if (down & kButtonDown) {
        data.camera.position = v3_subtract(data.camera.position, v3_multiply(data.camera.target, magnitude));
    }

    if (down & kButtonUp) {
        data.camera.position = v3_add(data.camera.position, v3_multiply(data.camera.target, magnitude));
    }

    if (down & kButtonRight) {
        data.camera.yaw += 1.0f;
    }

    if (down & kButtonLeft) {
        data.camera.yaw -= 1.0f;
    }

    update_camera_target(&data.camera);
    data.view = get_view(data.camera);

    draw_mesh(pd, &data.mesh);

    return 1;
}

int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg) {
    switch(event) {
        case kEventInit: {
            const char *fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
            const char* err;
            data.font = pd->graphics->loadFont(fontpath, &err);
            
            if (data.font == NULL)
                pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontpath, err);

            // Note: If you set an update callback in the kEventInit handler, the system assumes the game is pure C and doesn't run any Lua code in the game
            pd->system->setUpdateCallback(update, pd);

            data.camera.position = (Vector3) { 0,  0,  5 };
            data.camera.target   = (Vector3) { 0,  0,  0 };
            data.camera.up       = (Vector3) { 0,  1,  0 };
            data.camera.fov      = 75.0f;
            data.camera.yaw      = 45.0f;
            data.camera.pitch    = 0.0f;
            update_camera_target(&data.camera);

            memset(test, kColorBlack, sizeof(u8) * 16);
            memset(test + (sizeof(u8) * 16), kColorWhite, sizeof(u8) * 8);

            Vector3 pos = { 0, 0, 0 };
            Vector3 scale = { 1, 1, 1 };
            data.model = create_transform_m4x4(pos, scale);
            data.projection = perspective_projection(75.0f, (float32)LCD_COLUMNS / (float32)LCD_ROWS, 0.1f, 1000.0f);
            data.view = get_view(data.camera);
            data.mesh = get_rect_mesh();
        } break;

        case kEventKeyPressed: {
            switch(arg) {
                case kButtonDown: {
                    data.camera.pitch -= 1.0f;
                    update_camera_target(&data.camera);
                    data.view = get_view(data.camera);
                } break;
            }

        } break;

    }
    
    return 0;
}



/*
//
//  main.c
//  Extension
//
//  Created by Dave Hayden on 7/30/14.
//  Copyright (c) 2014 Panic, Inc. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"

/*
 Game of Life:
 
 Any live cell with fewer than two live neighbours dies, as if caused by under-population.
 Any live cell with two or three live neighbours lives on to the next generation.
 Any live cell with more than three live neighbours dies, as if by overcrowding.
 Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
*/
/*
static inline int ison(uint8_t* row, int x)
{
    return !(row[x/8] & (0x80 >> (x%8)));
}

static inline int val(uint8_t* row, int x)
{
    return 1 - ((row[x/8] >> (7 - (x%8))) & 1);
}

static inline int rowsum(uint8_t* row, int x)
{
    if ( x == 0 )
        return val(row, LCD_COLUMNS-1) + val(row, x) + val(row, x+1);
    else if ( x < LCD_COLUMNS - 1 )
        return val(row, x-1) + val(row, x) + val(row, x+1);
    else
        return val(row, x-1) + val(row, x) + val(row, 0);
}

static inline int middlerowsum(uint8_t* row, int x)
{
    if ( x == 0 )
        return val(row, LCD_COLUMNS-1) + val(row, x+1);
    else if ( x < LCD_COLUMNS - 1 )
        return val(row, x-1) + val(row, x+1);
    else
        return val(row, x-1) + val(row, 0);
}

static inline void
doRow(uint8_t* rowabove, uint8_t* row, uint8_t* rowbelow, uint8_t* outrow)
{
    char b = 0;
    int bitpos = 0x80;
    int x;
    
    for ( x = 0; x < LCD_COLUMNS; ++x )
    {
        // If total is 3 cell is alive
        // If total is 4, no change
        // Else, cell is dead
        
        int sum = rowsum(rowabove, x) + middlerowsum(row, x) + rowsum(rowbelow, x);
        
        if ( sum == 3 || (ison(row, x) && sum == 2) )
            b |= bitpos;
        
        bitpos >>= 1;
        
        if ( bitpos == 0 )
        {
            outrow[x/8] = ~b;
            b = 0;
            bitpos = 0x80;
        }
    }
}


static PlaydateAPI* pd = NULL;

static void randomize(void)
{
    int x, y;
    uint8_t* frame = pd->graphics->getDisplayFrame();

    for ( y = 0; y < LCD_ROWS; ++y )
    {
        uint8_t* row = &frame[y * LCD_ROWSIZE];
        
        for ( x = 0; x < LCD_COLUMNS / 8; ++x )
            row[x] = rand();
    }
}

static int
update(void* ud)
{
    PDButtons pushed;
    pd->system->getButtonState(NULL, &pushed, NULL);
    
    if ( pushed & kButtonA )
        randomize();
    
    uint8_t* nextframe = pd->graphics->getFrame(); // working buffer
    uint8_t* frame = pd->graphics->getDisplayFrame(); // buffer currently on screen (or headed there, anyway)
    
    if ( frame != NULL )
    {
        uint8_t* rowabove = &frame[LCD_ROWSIZE * (LCD_ROWS - 1)];
        uint8_t* row = frame;
        uint8_t* rowbelow = &frame[LCD_ROWSIZE];
        
        for ( int y = 0; y < LCD_ROWS; ++y )
        {
            doRow(rowabove, row, rowbelow, &nextframe[y * LCD_ROWSIZE]);
            
            rowabove = row;
            row = rowbelow;
            rowbelow = &frame[((y+2)%LCD_ROWS) * LCD_ROWSIZE];
        }
    }
    
    // we twiddled the framebuffer bits directly, so we have to tell the system about it
    pd->graphics->markUpdatedRows(0, LCD_ROWS);
    
    return 1;
}

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg)
{
    if ( event == kEventInit )
    {
        pd = playdate;
        pd->display->setRefreshRate(0); // run as fast as possible
        pd->system->setUpdateCallback(update, NULL);

        randomize();
    }
    
    return 0;
}
*/