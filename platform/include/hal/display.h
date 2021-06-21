/*
 * @file display.h
 * @author Lukasz Skrzypczak (lukasz.skrzypczak@mudita.com)
 * @date Oct 8, 2018
 * @brief Insert brief information about this file purpose.
 * @copyright Copyright (C) 2018 mudita.com.
 * @details More detailed information related to this code.
 */

#ifndef SOURCE_DISPLAY_H_
#define SOURCE_DISPLAY_H_

#include <stdio.h>

/**
  * @brief  LCD drawing Line alignment mode definitions
  */
typedef enum
{
  CENTER_MODE             = 0x01,    /*!< Center mode */
  RIGHT_MODE              = 0x02,    /*!< Right mode  */
  LEFT_MODE               = 0x03     /*!< Left mode   */

} Text_AlignModeTypdef;
#define BSP_EINK_YES    1
#define BSP_EINK_NO     0

void BSP_EINK_DisplayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii);
void BSP_EINK_DisplayStringAt(uint16_t Xpos, uint16_t Ypos, char *Text, Text_AlignModeTypdef Mode);
void BSP_EINK_Refresh_Text( uint16_t X, uint16_t Y, uint16_t W, uint16_t H );
void BSP_EINK_Log(char *Text, uint8_t checkInteractive);
void BSP_EINK_Clear_Log( void );
void BSP_EINK_Display_Init( void ) ;
void BSP_EINK_InteractiveMode(uint8_t enable);
uint8_t BSP_EINK_Ask_User(char *text);
uint8_t BSP_EINK_User_Menu(char *text);

#endif /* SOURCE_DISPLAY_H_ */
