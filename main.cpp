#include "GioEditor.h"

// Main code
int main(int, char**)
{
    GioEditor app;

    app.ready();

    while (!app.window_should_close()) {
        app.update();
    }

    app.clean_up();

    return 0;
}