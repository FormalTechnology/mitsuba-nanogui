//
//  fakeglfw.cpp
//  NanoGUICocoa
//
//  Created by Dmitry Lavrov on 07/12/2015.
//  Copyright © 2015 Formal Technology. All rights reserved.
//

#import <nanogui/fakeglfw.h>
#import <mach/mach_time.h>
#import <Cocoa/Cocoa.h>

static char * clipboardString;

#pragma mark - Clipboard

void glfwSetClipboardString(GLFWwindow * /*w*/, const char *string)
{
    NSPasteboard * pasteboard = [NSPasteboard generalPasteboard];
    [pasteboard declareTypes:@[NSStringPboardType] owner:nil];
    [pasteboard setString:[NSString stringWithUTF8String:string]
                  forType:NSStringPboardType];
}

const char* glfwGetClipboardString(GLFWwindow * /*w*/)
{
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];

    if (![[pasteboard types] containsObject:NSStringPboardType]) {
        NSLog(@"Cocoa: Failed to retrieve string from pasteboard");
        return NULL;
    }

    NSString* object = [pasteboard stringForType:NSStringPboardType];

    if (!object) {
        NSLog(@"Cocoa: Failed to retrieve object from pasteboard");
        return NULL;
    }

    free(clipboardString);
    clipboardString = strdup([object UTF8String]);

    return clipboardString;
}

int translateFlags(uint32_t flags)
{
    int mods = 0;

    if (flags & NSEventModifierFlagShift)
        mods |= GLFW_MOD_SHIFT;
    if (flags & NSEventModifierFlagControl)
        mods |= GLFW_MOD_CONTROL;
    if (flags & NSEventModifierFlagOption)
        mods |= GLFW_MOD_ALT;
    if (flags & NSEventModifierFlagCommand)
        mods |= GLFW_MOD_SUPER;

    return mods;
}
