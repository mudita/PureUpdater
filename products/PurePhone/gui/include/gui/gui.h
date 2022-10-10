#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

enum gui_screen_e {
    ScreenUpdateInProgress,
    ScreenUpdateFailed,
    ScreenUpdateSuccess,
    ScreenFactoryResetInProgress,
    ScreenFactoryResetFailed,
    ScreenFactoryResetSuccess,
    ScreenRecoveryInProgress,
    ScreenRecoveryFailed,
    ScreenRecoverySuccess,
    ScreenKeysInProgress,
    ScreenKeysFailed,
    ScreenKeysSuccess,
    ScreenBackupInProgress,
    ScreenBackupFailed,
    ScreenBackupSuccess,
    ScreenRestoreInProgress,
    ScreenRestoreFailed,
    ScreenRestoreSuccess
};

void gui_show_screen(enum gui_screen_e screen);
void gui_clear_display(void);

#ifdef __cplusplus
extern "C"
}
#endif
