#include "gui/gui.h"
#include "gui/images/gui_images_backup.h"
#include "gui/images/gui_images_factory_reset.h"
#include "gui/images/gui_images_keys.h"
#include "gui/images/gui_images_recovery.h"
#include "gui/images/gui_images_restore.h"
#include "gui/images/gui_images_update.h"
#include <hal/ED028TC1.h>

const unsigned short image_width = 600;
const unsigned short image_height = 480;

void gui_show_screen(enum gui_screen_e screen) {
    switch (screen) {
        case ScreenUpdateInProgress:
            EinkDisplayImage(0, 0, image_width, image_height, (uint8_t *) gui_image_update_in_progress);
            break;
        case ScreenUpdateFailed:
            EinkDisplayImage(0, 0, image_width, image_height, (uint8_t *) gui_image_update_failed);
            break;
        case ScreenUpdateSuccess:
            EinkDisplayImage(0, 0, image_width, image_height, (uint8_t *) gui_image_update_success);
            break;
        case ScreenFactoryResetInProgress:
            EinkDisplayImage(0, 0, image_width, image_height, (uint8_t *) gui_image_factory_reset_in_progress);
            break;
        case ScreenFactoryResetFailed:
            EinkDisplayImage(0, 0, image_width, image_height, (uint8_t *) gui_image_factory_reset_failed);
            break;
        case ScreenFactoryResetSuccess:
            EinkDisplayImage(0, 0, image_width, image_height, (uint8_t *) gui_image_factory_reset_success);
            break;
        case ScreenRecoveryInProgress:
            EinkDisplayImage(0, 0, image_width, image_height, (uint8_t *) gui_image_recovery_in_progress);
            break;
        case ScreenRecoveryFailed:
            EinkDisplayImage(0, 0, image_width, image_height, (uint8_t *) gui_image_recovery_failed);
            break;
        case ScreenRecoverySuccess:
            EinkDisplayImage(0, 0, image_width, image_height, (uint8_t *) gui_image_recovery_success);
            break;
        case ScreenKeysInProgress:
            EinkDisplayImage(0, 0, image_width, image_height, (uint8_t *) gui_image_keys_in_progress);
            break;
        case ScreenKeysFailed:
            EinkDisplayImage(0, 0, image_width, image_height, (uint8_t *) gui_image_keys_failed);
            break;
        case ScreenKeysSuccess:
            EinkDisplayImage(0, 0, image_width, image_height, (uint8_t *) gui_image_keys_success);
            break;
        case ScreenBackupInProgress:
            EinkDisplayImage(0, 0, image_width, image_height, (uint8_t *) gui_image_backup_in_progress);
            break;
        case ScreenBackupFailed:
            EinkDisplayImage(0, 0, image_width, image_height, (uint8_t *) gui_image_backup_failed);
            break;
        case ScreenBackupSuccess:
            EinkDisplayImage(0, 0, image_width, image_height, (uint8_t *) gui_image_backup_success);
            break;
        case ScreenRestoreInProgress:
            EinkDisplayImage(0, 0, image_width, image_height, (uint8_t *) gui_image_restore_in_progress);
            break;
        case ScreenRestoreFailed:
            EinkDisplayImage(0, 0, image_width, image_height, (uint8_t *) gui_image_restore_failed);
            break;
        case ScreenRestoreSuccess:
            EinkDisplayImage(0, 0, image_width, image_height, (uint8_t *) gui_image_restore_success);
            break;
        default:
            printf("Requested to show non-existent screen!\n");
            break;

    }
}

void gui_clear_display(void) {
    EinkClearScreen();
}
