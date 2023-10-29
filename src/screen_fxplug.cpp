//
//  screen_fxplug.cpp
//  XPC Service
//
//  Created by Dmitry Lavrov on 28/10/2023.
//

#include <iostream>
#include <map>
#include <nanogui/metal.h>
#include <nanogui/popup.h>
#include <nanogui/screen.h>
#include <nanogui/theme.h>
#include <nanogui/window.h>

#include "nanovg_mtl.h"

/* Calculate pixel ratio for hi-dpi devices. */
static float get_pixel_ratio( GLFWwindow* window ) {
    // TODO: Use FxPlug API to get the pixel ratio
    return 2.0;
}

NAMESPACE_BEGIN(nanogui)

Screen::Screen()
    : Widget(nullptr)
    , m_glfw_window(nullptr)
    , m_nvg_context(nullptr)
    , m_cursor(Cursor::Arrow)
    , m_background(0.3f, 0.3f, 0.32f, 1.f)
    , m_shutdown_glfw(false)
    , m_fullscreen(false)
    , m_depth_buffer(false)
    , m_stencil_buffer(false)
    , m_float_buffer(false)
    , m_redraw(false)
{
    memset(m_cursors, 0, sizeof(GLFWcursor*) * (size_t)Cursor::CursorCount);
}

Screen::Screen(const Vector2i& size, const std::string& caption, bool resizable,
    bool fullscreen, bool depth_buffer, bool stencil_buffer,
    bool float_buffer, unsigned int gl_major, unsigned int gl_minor)
    : Widget(nullptr)
    , m_glfw_window(nullptr)
    , m_nvg_context(nullptr)
    , m_cursor(Cursor::Arrow)
    , m_background(0.3f, 0.3f, 0.32f, 1.f)
    , m_caption(caption)
    , m_shutdown_glfw(false)
    , m_fullscreen(fullscreen)
    , m_depth_buffer(depth_buffer)
    , m_stencil_buffer(stencil_buffer)
    , m_float_buffer(float_buffer)
    , m_redraw(false)
{
    memset(m_cursors, 0, sizeof(GLFWcursor*) * (int)Cursor::CursorCount);

    int color_bits = 8, depth_bits = 0, stencil_bits = 0;

    if (stencil_buffer && !depth_buffer)
        throw std::runtime_error(
            "Screen::Screen(): stencil_buffer = True requires depth_buffer = True");
    if (depth_buffer) {
        depth_bits = 32;
    }

    if (stencil_buffer) {
        depth_bits = 24;
        stencil_bits = 8;
    }
    if (m_float_buffer) {
        color_bits = 16;
    }

#if defined(__APPLE__)
    /* Poll for events once before starting a potentially
       lengthy loading process. This is needed to be
       classified as "interactive" by other software such
       as iTerm2 */

    // glfwPollEvents();
#endif

    // Propagate GLFW events to the appropriate Screen instance
    /*
        glfwSetCursorPosCallback(m_glfw_window,
            [](GLFWwindow* w, double x, double y) {
                auto it = __nanogui_screens.find(w);
                if (it == __nanogui_screens.end())
                    return;
                Screen* s = it->second;
                if (!s->m_process_events)
                    return;
                s->cursor_pos_callback_event(x, y);
            });

        glfwSetMouseButtonCallback(m_glfw_window,
            [](GLFWwindow* w, int button, int action, int modifiers) {
                auto it = __nanogui_screens.find(w);
                if (it == __nanogui_screens.end())
                    return;
                Screen* s = it->second;
                if (!s->m_process_events)
                    return;
                s->mouse_button_callback_event(button, action, modifiers);
            });

        glfwSetKeyCallback(m_glfw_window,
            [](GLFWwindow* w, int key, int scancode, int action, int mods) {
                auto it = __nanogui_screens.find(w);
                if (it == __nanogui_screens.end())
                    return;
                Screen* s = it->second;
                if (!s->m_process_events)
                    return;
                s->key_callback_event(key, scancode, action, mods);
            });

        glfwSetCharCallback(m_glfw_window,
            [](GLFWwindow* w, unsigned int codepoint) {
                auto it = __nanogui_screens.find(w);
                if (it == __nanogui_screens.end())
                    return;
                Screen* s = it->second;
                if (!s->m_process_events)
                    return;
                s->char_callback_event(codepoint);
            });

        glfwSetDropCallback(m_glfw_window,
            [](GLFWwindow* w, int count, const char** filenames) {
                auto it = __nanogui_screens.find(w);
                if (it == __nanogui_screens.end())
                    return;
                Screen* s = it->second;
                if (!s->m_process_events)
                    return;
                s->drop_callback_event(count, filenames);
            });

        glfwSetScrollCallback(m_glfw_window,
            [](GLFWwindow* w, double x, double y) {
                auto it = __nanogui_screens.find(w);
                if (it == __nanogui_screens.end())
                    return;
                Screen* s = it->second;
                if (!s->m_process_events)
                    return;
                s->scroll_callback_event(x, y);
            });


    //     React to framebuffer size events -- includes window
    //       size events and also catches things like dragging
    //       a window from a Retina-capable screen to a normal
    //       screen on Mac OS X
        glfwSetFramebufferSizeCallback(m_glfw_window,
            [](GLFWwindow* w, int width, int height) {
                auto it = __nanogui_screens.find(w);
                if (it == __nanogui_screens.end())
                    return;
                Screen* s = it->second;

                if (!s->m_process_events)
                    return;

                s->resize_callback_event(width, height);
            });

        // notify when the screen has lost focus (e.g. application switch)
        glfwSetWindowFocusCallback(m_glfw_window,
            [](GLFWwindow* w, int focused) {
                auto it = __nanogui_screens.find(w);
                if (it == __nanogui_screens.end())
                    return;

                Screen* s = it->second;
                // focus_event: 0 when false, 1 when true
                s->focus_event(focused != 0);
            });

        glfwSetWindowContentScaleCallback(m_glfw_window,
            [](GLFWwindow* w, float, float) {
                auto it = __nanogui_screens.find(w);
                if (it == __nanogui_screens.end())
                    return;
                Screen* s = it->second;

                s->m_pixel_ratio = get_pixel_ratio(w);
                s->resize_callback_event(s->m_size.x(), s->m_size.y());
            });
     */
    initialize(m_glfw_window, true);

#if defined(NANOGUI_USE_METAL)
    if (depth_buffer) {
        m_depth_stencil_texture = new Texture(
            stencil_buffer ? Texture::PixelFormat::DepthStencil
                           : Texture::PixelFormat::Depth,
            Texture::ComponentFormat::Float32,
            framebuffer_size(),
            Texture::InterpolationMode::Bilinear,
            Texture::InterpolationMode::Bilinear,
            Texture::WrapMode::ClampToEdge,
            1,
            Texture::TextureFlags::RenderTarget);
    }
#endif
}

void Screen::initialize(GLFWwindow* window, bool shutdown_glfw)
{
    m_glfw_window = window;
    m_shutdown_glfw = shutdown_glfw;
//    glfwGetWindowSize(m_glfw_window, &m_size[0], &m_size[1]);
//    glfwGetFramebufferSize(m_glfw_window, &m_fbsize[0], &m_fbsize[1]);

    m_pixel_ratio = get_pixel_ratio(window);

    int flags = NVG_ANTIALIAS;
    if (m_stencil_buffer)
        flags |= NVG_STENCIL_STROKES;
#if !defined(NDEBUG)
    flags |= NVG_DEBUG;
#endif

    /*
    void* nswin = glfwGetCocoaWindow(window);
    metal_window_init(nswin, m_float_buffer);
    metal_window_set_size(nswin, m_fbsize);
    m_nvg_context = nvgCreateMTL(metal_layer(),
        metal_command_queue(),
        flags | NVG_TRIPLE_BUFFER);

    if (!m_nvg_context)
        throw std::runtime_error("Could not initialize NanoVG!");

    m_visible = glfwGetWindowAttrib(window, GLFW_VISIBLE) != 0;
    set_theme(new Theme(m_nvg_context));
    m_mouse_pos = Vector2i(0);
    m_mouse_state = m_modifiers = 0;
    m_drag_active = false;
    m_last_interaction = glfwGetTime();
    m_process_events = true;
    m_redraw = true;
    __nanogui_screens[m_glfw_window] = this;

    for (size_t i = 0; i < (size_t)Cursor::CursorCount; ++i)
        m_cursors[i] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR + (int)i);

    /// Fixes retina display-related font rendering issue (#185)
    nvgBeginFrame(m_nvg_context, m_size[0], m_size[1], m_pixel_ratio);
    nvgEndFrame(m_nvg_context);
     */
}

Screen::~Screen()
{
    //__nanogui_screens.erase(m_glfw_window);
    for (size_t i = 0; i < (size_t)Cursor::CursorCount; ++i) {
       // if (m_cursors[i])
           // glfwDestroyCursor(m_cursors[i]);
    }

    if (m_nvg_context) {
        nvgDeleteMTL(m_nvg_context);
    }

    //if (m_glfw_window && m_shutdown_glfw)
       // glfwDestroyWindow(m_glfw_window);
}

void Screen::draw_setup()
{
    
}

bool Screen::resize_event(Array<int, 2ul> const&) {
    return true;
}

void Screen::update_focus(Widget*) {
    
}

void Screen::center_window(Window*) {
    
}

void Screen::draw_contents() {
    
}

void Screen::draw_teardown() {
    
}

void Screen::dispose_window(Window*) {
    
}

bool Screen::keyboard_event(int, int, int, int) {
    return true;
}

bool Screen::keyboard_character_event(unsigned int) {
    return true;
}

void Screen::clear() {
    
}

void Screen::draw_all() {
    
}
void Screen::nvg_flush() {
    
}

//#if defined(NANOGUI_USE_METAL)
void  * Screen::metal_layer() const {
    return nullptr;
}
Texture::PixelFormat Screen::pixel_format() const {
    return Texture::PixelFormat::RGBA;
}
Texture::ComponentFormat  Screen::component_format() const {
    return Texture::ComponentFormat::Float16;
}
//#endif

NAMESPACE_END(nanogui)
