#include "config.h"

#if CONFIG_QT5EXTRAS
#include <QtX11Extras/QX11Info>

#include "X11Util.h"

void X11Util::registerHotkey(int keycode)
{
    Display* display = QX11Info::display();
    Window   root    = DefaultRootWindow(display);
    XEvent   event;
    // 0, ControlMask, ShiftMask, ShiftLockMask, Mod1Mask.
    int modifiers =  ControlMask | Mod1Mask;  
    int key = XKeysymToKeycode(display, keycode);

    XGrabKey(display, key, modifiers, root, True, GrabModeAsync, GrabModeAsync);
    //XSelectInput(display, root, KeyPressMask | KeyReleaseMask);
    //printf("register HotKey\n");
}

void X11Util::unregisterHotkey(int keycode)
{
    Display* display = QX11Info::display();
    Window   root    = DefaultRootWindow(display);
    int modifiers =  ControlMask | Mod1Mask;  
    int key = XKeysymToKeycode(display, keycode);

    XUngrabKey(display, key, modifiers, root);
    //printf("unregister Hotkey\n");
}

// Fixed: not working
void X11Util::forwardHotKey(int keycode)
{
    XEvent xevent;
    xevent.xkey.type = KeyPress;
    xevent.xkey.keycode = keycode;
    xevent.xkey.state = ControlMask | Mod1Mask;
    sendKeyEvent(QX11Info::display(), xevent);
}

void X11Util::sendKeyEvent(Display *display, XEvent event)
{
    Window current_focus_window;
    XKeyEvent& xkey = event.xkey;

    int current_focus_revert;
    XGetInputFocus(display, &current_focus_window, &current_focus_revert);

    XSendEvent(display, InputFocus,  True, xkey.type, (XEvent *)(&event));
}
#else
#include "X11Util.h"
void X11Util::registerHotkey(int keycode)   {}
void X11Util::unregisterHotkey(int keycode) {}
void X11Util::forwardHotKey(int keycode)    {}
void X11Util::sendKeyEvent(Display *display, XEvent event) {}
#endif
