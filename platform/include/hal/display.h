/*
 * @file display.h
 * @author Lukasz Skrzypczak (lukasz.skrzypczak@mudita.com)
 * @date Oct 8, 2018
 * @brief Insert brief information about this file purpose.
 * @copyright Copyright (C) 2018 mudita.com.
 * @details More detailed information related to this code.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
  * @brief  LCD drawing Line alignment mode definitions
  */
typedef enum eink_align_mode
{
  EINK_CENTER_MODE             = 0x01,    /*!< Center mode */
  EINK_RIGHT_MODE              = 0x02,    /*!< Right mode  */
  EINK_LEFT_MODE               = 0x03     /*!< Left mode   */

} eink_align_mode_t;



void eink_display_char(uint16_t xpos, uint16_t ypos, uint8_t ascii);
void eink_display_string_at(uint16_t xpos, uint16_t ypos, const char *text, eink_align_mode_t mode);
void eink_refresh_text( uint16_t x, uint16_t y, uint16_t w, uint16_t h );
void eink_log(const char *text, bool check_interactive);
void eink_clear_log( void );
void eink_init( void ) ;
