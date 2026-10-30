#include "config.h"

#if CONFIG_X11HOTKEY
#include <cstddef>

#include "X11Util.h"

// AlphaDict only needs a plain Xlib connection to grab the global hotkey.
// We open our own instead of going through Qt5X11Extras' QX11Info: that module
// was dropped in Qt6, and this file has no other Qt dependency.
static Display *x11Display()
{
    static Display *display = XOpenDisplay(NULL);
    return display;
}

void X11Util::registerHotkey(int keycode)
{
    Display* display = x11Display();
    if (display == NULL)
        return;

    Window   root    = DefaultRootWindow(display);
    // 0, ControlMask, ShiftMask, ShiftLockMask, Mod1Mask.
    int modifiers =  ControlMask | Mod1Mask;
    int key = XKeysymToKeycode(display, keycode);

    XGrabKey(display, key, modifiers, root, True, GrabModeAsync, GrabModeAsync);
    XSync(display, False);
    //XSelectInput(display, root, KeyPressMask | KeyReleaseMask);
    //printf("register HotKey\n");
}

void X11Util::unregisterHotkey(int keycode)
{
    Display* display = x11Display();
    if (display == NULL)
        return;

    Window   root    = DefaultRootWindow(display);
    int modifiers =  ControlMask | Mod1Mask;
    int key = XKeysymToKeycode(display, keycode);

    XUngrabKey(display, key, modifiers, root);
    XSync(display, False);
    //printf("unregister Hotkey\n");
}

// Fixed: not working
void X11Util::forwardHotKey(int keycode)
{
    Display* display = x11Display();
    if (display == NULL)
        return;

    XEvent xevent;
    xevent.xkey.type = KeyPress;
    xevent.xkey.keycode = keycode;
    xevent.xkey.state = ControlMask | Mod1Mask;
    sendKeyEvent(display, xevent);
}

void X11Util::sendKeyEvent(Display *display, XEvent event)
{
    if (display == NULL)
        return;

    Window current_focus_window;
    XKeyEvent& xkey = event.xkey;

    int current_focus_revert;
    XGetInputFocus(display, &current_focus_window, &current_focus_revert);

    XSendEvent(display, InputFocus,  True, xkey.type, (XEvent *)(&event));
}
#else
#include "X11Util.h"
void X11Util::registerHotkey(int keycode)   { (void)keycode; }
void X11Util::unregisterHotkey(int keycode) { (void)keycode; }
void X11Util::forwardHotKey(int keycode)    { (void)keycode; }
void X11Util::sendKeyEvent(Display *display, XEvent event) { (void)display; (void)event; }
#endif
