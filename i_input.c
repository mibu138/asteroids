#include "i_input.h"
#include "d_display.h"
#include "g_game.h"
#include "def.h"
#include <stdio.h>
#include <stdlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_util.h>
#include <X11/keysym.h>

#define MAX_EVENTS 32

static xcb_key_symbols_t* pXcbKeySymbols;

static I_Event events[MAX_EVENTS];
static int     eventHead;
static int     eventTail;

static I_EventData getKeyCode(xcb_button_press_event_t* event)
{
    // XCB documentation is fucking horrible. fucking last parameter is called col. wtf? 
    // no clue what that means. ZERO documentation on this function. trash.
    xcb_keysym_t keySym = xcb_key_symbols_get_keysym(pXcbKeySymbols, event->detail, 0); 
    switch (keySym)
    {
        case XK_w: printf("FORWARD\n"); return KEY_W;
        case XK_a: printf("LEFT\n"); return KEY_A;
        case XK_s: printf("BACKWARD\n"); return KEY_S;
        case XK_d: printf("RIGHT\n"); return KEY_D;
        default: return 0;
    }
}

static void postEvent(I_Event event)
{
    events[eventHead] = event;
    eventHead = (eventHead + 1) % MAX_EVENTS;
    printf("Eventhead: %d\n", eventHead);
}

void i_Init(void)
{
    pXcbKeySymbols = xcb_key_symbols_alloc(d_XcbWindow.connection);
}

void i_GetEvents(void)
{
    xcb_generic_event_t* xEvent = NULL;
    while ((xEvent = xcb_poll_for_event(d_XcbWindow.connection)))
    {
        I_Event event;
        switch (XCB_EVENT_RESPONSE_TYPE(xEvent))
        {
            case XCB_KEY_PRESS: 
                event.type = i_Keydown; 
                event.data = getKeyCode((xcb_button_press_event_t*)xEvent);
                break;
            case XCB_KEY_RELEASE: 
                event.type = i_Keyup;
                event.data = getKeyCode((xcb_button_press_event_t*)xEvent);
                break;
            default: return;
        }
        postEvent(event);
        free(xEvent);
    }
    // to do
}

void i_ProcessEvents(void)
{
    I_Event* event;
    for ( ; eventTail != eventHead; eventTail = (eventTail + 1) % MAX_EVENTS) 
    {
        event = &events[eventTail];   
        g_Responder(event);
        printf("eventTail: %d\n", eventTail);
    }
}

void i_CleanUp(void)
{
    xcb_key_symbols_free(pXcbKeySymbols);
}
