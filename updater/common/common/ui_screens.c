#include "ui_screens.h"
#include <ED028TC1.h>

#include "ui_update_in_progress.h"
#include "ui_update_success.h"
#include "ui_update_error.h"

const unsigned short logo_width = 600;
const unsigned short logo_height = 480;

void show_update_success_screen(void) {
    EinkDisplayImage(0, 0, logo_width, logo_height, (uint8_t *) update_success);
}

void show_update_in_progress_screen(void) {
    EinkDisplayImage(0, 0, logo_width, logo_height, (uint8_t *) update_in_progress);
}

void show_update_error_screen(void) {
    EinkDisplayImage(0, 0, logo_width, logo_height, (uint8_t *) update_error);
}
