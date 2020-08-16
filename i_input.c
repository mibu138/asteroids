#include "i_input.h"
#include "d_display.h"
#include <stdio.h>
#include <xcb/xcb.h>

void i_Init(void)
{
    // nothing for now
}

void i_GetEvents(void)
{
    xcb_generic_event_t* xEvent = NULL;
    while ((xEvent = xcb_poll_for_event(d_XcbWindow.connection)))
    {
        I_Event event;
        printf("Got event\n");
    }
    // to do
}
