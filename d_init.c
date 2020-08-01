#include "d_init.h"
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <string.h>

XcbWindow xcbWindow;

static const char* windowName = "floating";

void d_Init(void)
{
    int screenNum = 0;
    xcbWindow.connection =     xcb_connect(NULL, &screenNum);
    xcbWindow.window     =     xcb_generate_id(xcbWindow.connection);

    const xcb_setup_t* setup = xcb_get_setup(xcbWindow.connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);

    for (int i = 0; i < screenNum; i++)
    {
        xcb_screen_next(&iter);   
    }

    xcb_screen_t* screen = iter.data;

    xcb_create_window(xcbWindow.connection, 
            XCB_COPY_FROM_PARENT,              // depth 
            xcbWindow.window,                  // window id
            screen->root,                      // parent
            0, 0,                              // x and y coordinate of new window
            WINDOW_WIDTH, WINDOW_HEIGHT, 
            0,                                 // border wdith 
            XCB_WINDOW_CLASS_COPY_FROM_PARENT, // class 
            XCB_COPY_FROM_PARENT,              // visual 
            0, NULL);                          // masks (TODO: set to get inputs)

    xcb_change_property(xcbWindow.connection, 
            XCB_PROP_MODE_REPLACE, 
            xcbWindow.window, 
            XCB_ATOM_WM_NAME, 
            XCB_ATOM_STRING, 8, strlen(windowName), windowName);

    xcb_map_window(xcbWindow.connection, xcbWindow.window);
    xcb_flush(xcbWindow.connection);
}

void d_CleanUp(void)
{
    xcb_disconnect(xcbWindow.connection);
}
