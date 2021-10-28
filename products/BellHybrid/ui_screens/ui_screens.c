#include "ui_screens.h"
#include "display.h"

void show_update_success_screen(void)
{
    eink_log("update success", true);
}

void show_update_in_progress_screen(void)
{
    eink_log("system update", true);
}

void show_update_error_screen(void)
{
    eink_log("update error", true);
}
