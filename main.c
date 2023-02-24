#include "chess.h"
#include "types.h"
#include <glad/glad.h>
#include <stdio.h>

int main(void) {
    sdl_setup_window("Chess");
    application_loop();
    application_terminate();

    return exit_success;
}

