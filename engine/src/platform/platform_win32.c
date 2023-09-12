#include "platform/platform.h"
#include "containers/darray.h"

#include "core/logger.h"
#include "core/input.h"
#include "core/event.h"

// Windows platform layer.
#if LPLATFORM_WINDOWS


#include <stdio.h>
#include <windows.h>
#include <windowsx.h> // param input extraction

// For surface creation
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include "renderer/vulkan/vulkan_types.inl"


typedef struct internal_state {
    HINSTANCE h_instance;
    HWND hwnd;
    VkSurfaceKHR surface;
} internal_state;

// Clock
static f64 clock_freq;
static LARGE_INTEGER start_time; 

// Forward declaration of win32_process_message to use in Window Class creation
LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);

b8 platform_startup(
    platform_state* plat_state,
    const char* application_name,
    i32 x,
    i32 y, 
    i32 width, 
    i32 height) 
    {
            
        plat_state->internal_state = malloc(sizeof(internal_state));
        internal_state* state = (internal_state *)plat_state->internal_state;

        state->h_instance = GetModuleHandleA(0);

        // Create Window Class
        HICON icon = LoadIcon(state->h_instance, IDI_APPLICATION);
        WNDCLASSA wc;
        memset(&wc, 0, sizeof(wc));
        wc.style = CS_DBLCLKS;                    // Get Double-Clicks
        wc.lpfnWndProc = win32_process_message;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = state->h_instance;
        wc.hIcon = icon;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW); // NULL: Manage the cursor manually
        wc.hbrBackground = NULL;                  // Transparent 
        wc.lpszClassName = "luminix_window_class";

        // Register Window Class 
        if (!RegisterClassA(&wc)) {
            MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
            return false;
        }

        // Initializing window dimensions and style
        u32 client_x = x;
        u32 client_y = y;
        u32 client_width = width;
        u32 client_height = height;

        u32 window_x = client_x;
        u32 window_y = client_y;
        u32 window_width = client_width;
        u32 window_height = client_height;

        u32 window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
        u32 window_ex_style = WS_EX_APPWINDOW;

        window_style |= WS_MAXIMIZEBOX;
        window_style |= WS_MINIMIZEBOX;
        window_style |= WS_THICKFRAME;

        // Obtain the size of the border.
        RECT border_rect = {0,0,0,0};
        AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

        // We've set it up st the border rectance is negative.
        window_x += border_rect.left;
        window_y += border_rect.top;

        // Increase the size of the OS border accordingly
        window_width += border_rect.right - border_rect.left;
        window_height += border_rect.bottom - border_rect.top;

        // Create Window
        HWND handle = CreateWindowExA(
            window_ex_style, "luminix_window_class", application_name,
            window_style, window_x, window_y, window_width, window_height, 
            0, 0, state->h_instance, 0); 

        if (handle == 0) {
            MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

            LFATAL("Window creation failed!");
            return false;
        } else {
            state->hwnd = handle;
        }

        // Show the window
        b32 should_activate = 1; // TODO: If the window should not accept input, this should be false.
        i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
        // If the window should be initially minimized, use SW_MINIMIZE : SW_SHOWMINNOACTIVE;
        // If the window should be initially maximized, use SW_SHOWMAXIMIZED : SW_MAXIMIZE;
        ShowWindow(state->hwnd, show_window_command_flags);

        // Clock Setup
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        clock_freq = 1.0 / (f64)freq.QuadPart;
        QueryPerformanceCounter(&start_time);

        return true;
    }


void platform_shutdown(platform_state* plat_state) 
{
    // Cold-cast to the known type
    internal_state* state = (internal_state *)plat_state->internal_state;

    if (state->hwnd){
        DestroyWindow(state->hwnd);
        state->hwnd = 0;
    }  
}

b8 platform_pump_messages(platform_state* plat_state)
{
    MSG message;
    while(PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return true;
}

void* platform_allocate(u64 size, b8 aligned)
{
    return malloc(size);
}

void platform_free(void* block, b8 aligned)
{
    free(block);
}

void* platform_zero_memory(void* block, u64 size)
{
    return memset(block, 0, size);
}

void* platform_copy_memory(void* dest, const void* source, u64 size)
{
    return memcpy(dest, source, size);
}

void* platform_set_memory(void* dest, i32 value, u64 size)
{
    return memset(dest, value, size);
}

void platform_console_write(const char* message, u8 color)
{
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[color]);

    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, number_written, 0);
}

void platform_console_write_error(const char* message, u8 color)
{
    HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 0};
    SetConsoleTextAttribute(console_handle, levels[color]);

    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, number_written, 0);
}

f64 platform_get_absolute_time()
{
    LARGE_INTEGER now_time;
    QueryPerformanceCounter(&now_time);
    return (f64)now_time.QuadPart * clock_freq;
}

void platform_sleep(u64 ms)
{
    Sleep(ms);
}

void platform_get_required_extension_names(const char*** names_darray)
{
    darray_push(*names_darray, &"VK_KHR_win32_surface");
}

// Surface creation for Vulkan
// Explosing the platform layer to Vulkan stuff
// is not the most ideal solution, but will do for now
b8 platform_create_vulkan_surface(platform_state* plat_state, vulkan_context* context)
{
    // Simply cold-cast to the known type
    internal_state* state = (internal_state*)plat_state->internal_state;

    VkWin32SurfaceCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    create_info.hinstance = state->h_instance;
    create_info.hwnd = state->hwnd;

    VkResult result = vkCreateWin32SurfaceKHR(context->instance, &create_info, context->allocator, &state->surface); 

    if (result != VK_SUCCESS) {
        LFATAL("Vulkan surface creation failed!");
    }

    context->surface = state->surface;
    return true;
}

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param)
{
    switch(msg) 
    {
        case WM_ERASEBKGND:
            // Notifies the OS that the screen erasure will be handled by the application
            return 1;

        case WM_CLOSE:
            event_context data = {};
            event_fire(EVENT_CODE_APPLICATION_QUIT, 0, data);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_SIZE: {
            // Get the updated size of the window
            RECT r;
            GetClientRect(hwnd, &r);
            u32 width = r.right - r.left;
            u32 height = r.bottom - r.top;

            // Fire the event. The application layer should pick this up, but not handle it
            // as it shouldn't be visible to other parts of the application.
            event_context context;
            context.data.u16[0] = (u16)width;
            context.data.u16[1] = (u16)height;
            event_fire(EVENT_CODE_RESIZED, 0, context);
             
        } break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            // Key pressed / released
            b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            keys key = (u16)w_param;
            

            //ALT keys 
            if(w_param == VK_MENU) {
                if(GetKeyState(VK_RMENU) & 0x8000) {
                    key = KEY_RALT;
                } else if(GetKeyState(VK_LMENU) & 0x8000) {
                    key = KEY_LALT;
                }
            }
            //SHIFT keys 
            if(w_param == VK_SHIFT) {
                if(GetKeyState(VK_RSHIFT) & 0x8000) {
                    key = KEY_RSHIFT;
                } else if(GetKeyState(VK_LSHIFT) & 0x8000) {
                    key = KEY_LSHIFT;
                }
            }
            //CTRL keys 
            if(w_param == VK_CONTROL) {
                if(GetKeyState(VK_LCONTROL) & 0x8000) {
                    key = KEY_LCONTROL;
                } else if(GetKeyState(VK_RCONTROL) & 0x8000) {
                    key = KEY_RCONTROL;
                }
            }



            // Pass to input subsystem for processing
            input_process_key(key, pressed);

        } break;

        case WM_MOUSEMOVE: {
            // Mouse move
            i32 x_position = GET_X_LPARAM(l_param);
            i32 y_position = GET_Y_LPARAM(l_param);
            
            // Pass over to input subsystem
            input_process_mouse_move(x_position, y_position);
            
        } break;

        case WM_MOUSEWHEEL: {
            i32 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
            if (z_delta != 0) {
                // Flatten the input to be OS - independent (-1, 1)
                // Not checking for variable wheel speeds.
                z_delta = (z_delta <= 0) ? -1 : 1;

                // Pass to input subsystem
                input_process_mouse_wheel(z_delta);
            }
            
        } break;
    
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            // Above are handled together, then filtered 
            b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_RBUTTONDOWN; 
            buttons mouse_button = BUTTON_MAX_BUTTONS;

            switch (msg)
            {
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
                mouse_button = BUTTON_LEFT;
                break;

            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
                mouse_button = BUTTON_MIDDLE;
                break;

            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
                mouse_button = BUTTON_RIGHT;
                break;
            }

            // Pass to input subsystem
            if (mouse_button != BUTTON_MAX_BUTTONS) {
                input_process_button(mouse_button, pressed);
            }
        } break;

    }

    return DefWindowProcA(hwnd, msg, w_param, l_param);
}

#endif // LPLATFORM_WINDOWS