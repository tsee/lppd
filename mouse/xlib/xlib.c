#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <X11/Xlib.h>

int main() {
    int delta_x = 0, delta_y = 0;
    Display *display = XOpenDisplay(NULL);
    Window root = DefaultRootWindow(display);
    XWarpPointer(display, None, root, 0, 0, 0, 0, delta_x, delta_y);
    XCloseDisplay(display);
    return 0;
}

