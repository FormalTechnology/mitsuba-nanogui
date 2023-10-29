//
//  fakeglfw.cpp
//  NanoGUICocoa
//
//  Created by Dmitry Lavrov on 07/12/2015.
//  Copyright © 2015 Formal Technology. All rights reserved.
//

#include "fakeglfw.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <mach/mach_time.h>
#endif
#include <string.h>
#include <cstdint>

static short int  publicKeys[256];
// Cocoa-specific global timer data
//
#ifdef _WIN32
static uint64_t getRawTime(void)
{
    return GetTickCount64();
}
static uint64_t basetime;
void glfwInitTimer(void)
{
    basetime = getRawTime();
}
double glfwGetTime(void)
{
    return (getRawTime() - basetime) / 1000.0;
}
void glfwSetTime(double time)
{
    basetime = time * 1000.0;
}
#else

typedef struct _GLFWtimeNS
{
    double          base;
    double          resolution;

} _GLFWtimeNS;

static _GLFWtimeNS ns_time;

// Return raw time
//
static uint64_t getRawTime(void)
{
    return mach_absolute_time();
}

// Initialise timer
//
void glfwInitTimer(void)
{
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);

    ns_time.resolution = (double) info.numer / (info.denom * 1.0e9);
    ns_time.base = getRawTime();
}

double glfwGetTime(void)
{
    return (double) ( ( getRawTime() - ns_time.base ) * ns_time.resolution );
}

void glfwSetTime(double time)
{
    ns_time.base = getRawTime() -
    (uint64_t) (time / ns_time.resolution);
}

#endif

// Create key code translation tables
//
void createKeyTables(void)
{
    memset(publicKeys, -1, sizeof(publicKeys));

    publicKeys[0x1D] = GLFW_KEY_0;
    publicKeys[0x12] = GLFW_KEY_1;
    publicKeys[0x13] = GLFW_KEY_2;
    publicKeys[0x14] = GLFW_KEY_3;
    publicKeys[0x15] = GLFW_KEY_4;
    publicKeys[0x17] = GLFW_KEY_5;
    publicKeys[0x16] = GLFW_KEY_6;
    publicKeys[0x1A] = GLFW_KEY_7;
    publicKeys[0x1C] = GLFW_KEY_8;
    publicKeys[0x19] = GLFW_KEY_9;
    publicKeys[0x00] = GLFW_KEY_A;
    publicKeys[0x0B] = GLFW_KEY_B;
    publicKeys[0x08] = GLFW_KEY_C;
    publicKeys[0x02] = GLFW_KEY_D;
    publicKeys[0x0E] = GLFW_KEY_E;
    publicKeys[0x03] = GLFW_KEY_F;
    publicKeys[0x05] = GLFW_KEY_G;
    publicKeys[0x04] = GLFW_KEY_H;
    publicKeys[0x22] = GLFW_KEY_I;
    publicKeys[0x26] = GLFW_KEY_J;
    publicKeys[0x28] = GLFW_KEY_K;
    publicKeys[0x25] = GLFW_KEY_L;
    publicKeys[0x2E] = GLFW_KEY_M;
    publicKeys[0x2D] = GLFW_KEY_N;
    publicKeys[0x1F] = GLFW_KEY_O;
    publicKeys[0x23] = GLFW_KEY_P;
    publicKeys[0x0C] = GLFW_KEY_Q;
    publicKeys[0x0F] = GLFW_KEY_R;
    publicKeys[0x01] = GLFW_KEY_S;
    publicKeys[0x11] = GLFW_KEY_T;
    publicKeys[0x20] = GLFW_KEY_U;
    publicKeys[0x09] = GLFW_KEY_V;
    publicKeys[0x0D] = GLFW_KEY_W;
    publicKeys[0x07] = GLFW_KEY_X;
    publicKeys[0x10] = GLFW_KEY_Y;
    publicKeys[0x06] = GLFW_KEY_Z;

    publicKeys[0x27] = GLFW_KEY_APOSTROPHE;
    publicKeys[0x2A] = GLFW_KEY_BACKSLASH;
    publicKeys[0x2B] = GLFW_KEY_COMMA;
    publicKeys[0x18] = GLFW_KEY_EQUAL;
    publicKeys[0x32] = GLFW_KEY_GRAVE_ACCENT;
    publicKeys[0x21] = GLFW_KEY_LEFT_BRACKET;
    publicKeys[0x1B] = GLFW_KEY_MINUS;
    publicKeys[0x2F] = GLFW_KEY_PERIOD;
    publicKeys[0x1E] = GLFW_KEY_RIGHT_BRACKET;
    publicKeys[0x29] = GLFW_KEY_SEMICOLON;
    publicKeys[0x2C] = GLFW_KEY_SLASH;
    publicKeys[0x0A] = GLFW_KEY_WORLD_1;

    publicKeys[0x33] = GLFW_KEY_BACKSPACE;
    publicKeys[0x39] = GLFW_KEY_CAPS_LOCK;
    publicKeys[0x75] = GLFW_KEY_DELETE;
    publicKeys[0x7D] = GLFW_KEY_DOWN;
    publicKeys[0x77] = GLFW_KEY_END;
    publicKeys[0x24] = GLFW_KEY_ENTER;
    publicKeys[0x35] = GLFW_KEY_ESCAPE;
    publicKeys[0x7A] = GLFW_KEY_F1;
    publicKeys[0x78] = GLFW_KEY_F2;
    publicKeys[0x63] = GLFW_KEY_F3;
    publicKeys[0x76] = GLFW_KEY_F4;
    publicKeys[0x60] = GLFW_KEY_F5;
    publicKeys[0x61] = GLFW_KEY_F6;
    publicKeys[0x62] = GLFW_KEY_F7;
    publicKeys[0x64] = GLFW_KEY_F8;
    publicKeys[0x65] = GLFW_KEY_F9;
    publicKeys[0x6D] = GLFW_KEY_F10;
    publicKeys[0x67] = GLFW_KEY_F11;
    publicKeys[0x6F] = GLFW_KEY_F12;
    publicKeys[0x69] = GLFW_KEY_F13;
    publicKeys[0x6B] = GLFW_KEY_F14;
    publicKeys[0x71] = GLFW_KEY_F15;
    publicKeys[0x6A] = GLFW_KEY_F16;
    publicKeys[0x40] = GLFW_KEY_F17;
    publicKeys[0x4F] = GLFW_KEY_F18;
    publicKeys[0x50] = GLFW_KEY_F19;
    publicKeys[0x5A] = GLFW_KEY_F20;
    publicKeys[0x73] = GLFW_KEY_HOME;
    publicKeys[0x72] = GLFW_KEY_INSERT;
    publicKeys[0x7B] = GLFW_KEY_LEFT;
    publicKeys[0x3A] = GLFW_KEY_LEFT_ALT;
    publicKeys[0x3B] = GLFW_KEY_LEFT_CONTROL;
    publicKeys[0x38] = GLFW_KEY_LEFT_SHIFT;
    publicKeys[0x37] = GLFW_KEY_LEFT_SUPER;
    publicKeys[0x6E] = GLFW_KEY_MENU;
    publicKeys[0x47] = GLFW_KEY_NUM_LOCK;
    publicKeys[0x79] = GLFW_KEY_PAGE_DOWN;
    publicKeys[0x74] = GLFW_KEY_PAGE_UP;
    publicKeys[0x7C] = GLFW_KEY_RIGHT;
    publicKeys[0x3D] = GLFW_KEY_RIGHT_ALT;
    publicKeys[0x3E] = GLFW_KEY_RIGHT_CONTROL;
    publicKeys[0x3C] = GLFW_KEY_RIGHT_SHIFT;
    publicKeys[0x36] = GLFW_KEY_RIGHT_SUPER;
    publicKeys[0x31] = GLFW_KEY_SPACE;
    publicKeys[0x30] = GLFW_KEY_TAB;
    publicKeys[0x7E] = GLFW_KEY_UP;

    publicKeys[0x52] = GLFW_KEY_KP_0;
    publicKeys[0x53] = GLFW_KEY_KP_1;
    publicKeys[0x54] = GLFW_KEY_KP_2;
    publicKeys[0x55] = GLFW_KEY_KP_3;
    publicKeys[0x56] = GLFW_KEY_KP_4;
    publicKeys[0x57] = GLFW_KEY_KP_5;
    publicKeys[0x58] = GLFW_KEY_KP_6;
    publicKeys[0x59] = GLFW_KEY_KP_7;
    publicKeys[0x5B] = GLFW_KEY_KP_8;
    publicKeys[0x5C] = GLFW_KEY_KP_9;
    publicKeys[0x45] = GLFW_KEY_KP_ADD;
    publicKeys[0x41] = GLFW_KEY_KP_DECIMAL;
    publicKeys[0x4B] = GLFW_KEY_KP_DIVIDE;
    publicKeys[0x4C] = GLFW_KEY_KP_ENTER;
    publicKeys[0x51] = GLFW_KEY_KP_EQUAL;
    publicKeys[0x43] = GLFW_KEY_KP_MULTIPLY;
    publicKeys[0x4E] = GLFW_KEY_KP_SUBTRACT;
}

int translateKey(unsigned int key)
{
    if (key >= sizeof(publicKeys) / sizeof(publicKeys[0]) ) {
        return GLFW_KEY_UNKNOWN;
    }

    return publicKeys[key];
}
