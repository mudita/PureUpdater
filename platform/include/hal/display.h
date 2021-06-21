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

/** Initialize eink display
 */
void eink_init( void );

/** Display char at selected position
 * @param xpos X axis position
 * @param ypos Y axis position
 * @param ascii ascii char to display
 */
void eink_display_char(uint16_t xpos, uint16_t ypos, uint8_t ascii);

/** Display string begining from position
 * @param xpos X starting position
 * @param ypos Y starting position
 * @param text Text for display
 * @param mode Alignement mode @see eink_align_mode
 */
void eink_display_string_at(uint16_t xpos, uint16_t ypos, const char *text, eink_align_mode_t mode);

/** Display refresh text at selected position
 * @param x X starting position
 * @param y Y starting position
 * @param w Width of clean area
 * @param h Weight of clean area
 */
void eink_refresh_text( uint16_t x, uint16_t y, uint16_t w, uint16_t h );

/** Display to the eink like log console
 * @param text Text for display
 * @param interactive True if interactive refreshing enabled
 */
void eink_log(const char *text, bool interactive);

/** Clear eink log console
 */
void eink_clear_log( void );

