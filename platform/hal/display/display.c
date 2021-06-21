/*
 * @file display.c
 * @author Lukasz Skrzypczak (lukasz.skrzypczak@mudita.com)
 * @date Oct 8, 2018
 * @brief Insert brief information about this file purpose.
 * @copyright Copyright (C) 2018 mudita.com.
 * @details More detailed information related to this code.
 */

#include <hal/display.h>
#include "ED028TC1.h"
#include "fonts.h"
#include <hal/delay.h>
#include <boot/board.h>

#define EINK_LINE_LEN   (BOARD_EINK_DISPLAY_RES_X / 17 - 2)
#define EINK_MAX_LINES  (BOARD_EINK_DISPLAY_RES_Y / 24)

static unsigned char EINK_BUFFER_1BPP[2 + (BOARD_EINK_DISPLAY_RES_X*BOARD_EINK_DISPLAY_RES_Y/8)];
static char EINK_Text_Buffer[EINK_MAX_LINES][EINK_LINE_LEN + 1];

static void DrawChar(uint16_t Xpos, uint16_t Ypos, const uint8_t *c);

/**
 * @brief  Draws a character on LCD.
 * @param  Xpos: Line where to display the character shape
 * @param  Ypos: Start column address
 * @param  c: Pointer to the character data
 */
static void DrawChar(uint16_t Xpos, uint16_t Ypos, const uint8_t *c)
{
    uint32_t i = 0, j = 0;
    uint16_t height, width;
    uint8_t  offset;
    uint8_t  *pchar;
    uint32_t line;

    height = Font24.Height;
    width  = Font24.Width;

    offset =  8 *((width + 7)/8) -  width ;

    for(i = 0; i < height; i++)
    {
        pchar = ((uint8_t *)c + (width + 7)/8 * i);

        switch(((width + 7)/8))
        {

            case 1:
                line =  pchar[0];
                break;

            case 2:
                line =  (pchar[0]<< 8) | pchar[1];
                break;

            case 3:
            default:
                line =  (pchar[0]<< 16) | (pchar[1]<< 8) | pchar[2];
                break;
        }

        for (j = 0; j < width; j++)
        {
            int _byte = ((BOARD_EINK_DISPLAY_RES_X - Xpos - j) * BOARD_EINK_DISPLAY_RES_Y + (BOARD_EINK_DISPLAY_RES_Y - Ypos)) / 8;
            int _bit = 7 - (((BOARD_EINK_DISPLAY_RES_X - Xpos - j) * BOARD_EINK_DISPLAY_RES_Y + (BOARD_EINK_DISPLAY_RES_Y - Ypos)) % 8);
            if(line & (1 << (width- j + offset- 1)))
            {
                //BSP_LCD_DrawPixel((Xpos + j), Ypos, DrawProp[ActiveLayer].TextColor);
                EINK_BUFFER_1BPP[2 + _byte] &= ~(1<<_bit);
            }
            else
            {
                //BSP_LCD_DrawPixel((Xpos + j), Ypos, DrawProp[ActiveLayer].BackColor);
                EINK_BUFFER_1BPP[2 + _byte] |= (1<<_bit);
            }
        }
        Ypos++;
    }
}

/**
 * @brief  Displays one character in currently active layer.
 * @param  Xpos: Start column address
 * @param  Ypos: Line where to display the character shape.
 * @param  Ascii: Character ascii code
 *           This parameter must be a number between Min_Data = 0x20 and Max_Data = 0x7E
 */
void BSP_EINK_DisplayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii)
{
    DrawChar(Xpos, Ypos, &Font24.table[(Ascii-' ') * Font24.Height * ((Font24.Width + 7) / 8)]);
}

/**
 * @brief  Displays characters in currently active layer.
 * @param  Xpos: X position (in pixel)
 * @param  Ypos: Y position (in pixel)
 * @param  Text: Pointer to string to display on LCD
 * @param  Mode: Display mode
 *          This parameter can be one of the following values:
 *            @arg  CENTER_MODE
 *            @arg  RIGHT_MODE
 *            @arg  LEFT_MODE
 */
void BSP_EINK_DisplayStringAt(uint16_t Xpos, uint16_t Ypos, char *Text, Text_AlignModeTypdef Mode)
{
    uint16_t refcolumn = 1, i = 0;
    uint32_t size = 0, xsize = 0;
    char  *ptr = Text;

    /* Get the text size */
    while (*ptr++) size ++ ;

    /* Characters number per line */
    xsize = (BOARD_EINK_DISPLAY_RES_X/Font24.Width);

    switch (Mode)
    {
        case CENTER_MODE:
        {
            refcolumn = Xpos + ((xsize - size)* Font24.Width) / 2;
            break;
        }
        case LEFT_MODE:
        {
            refcolumn = Xpos;
            break;
        }
        case RIGHT_MODE:
        {
            refcolumn = - Xpos + ((xsize - size)*Font24.Width);
            break;
        }
        default:
        {
            refcolumn = Xpos;
            break;
        }
    }

    /* Check that the Start column is located in the screen */
    if ((refcolumn < 1) || (refcolumn >= 0x8000))
    {
        refcolumn = 1;
    }

    /* Send the string character by character on LCD */
    while ((*Text != 0) & (((BOARD_EINK_DISPLAY_RES_X - (i*Font24.Width)) & 0xFFFF) >= Font24.Width))
    {
        /* Display one character on LCD */
        BSP_EINK_DisplayChar(refcolumn, Ypos, (uint8_t)*Text);
        /* Decrement the column position by 16 */
        refcolumn += Font24.Width;

        /* Point on the next character */
        Text++;
        i++;
    }

}

void BSP_EINK_Refresh_Text( uint16_t X, uint16_t Y, uint16_t W, uint16_t H ) {

    EINK_BUFFER_1BPP[0] = EinkDataStartTransmission1;
    EINK_BUFFER_1BPP[1] = 0;
    EinkDisplayImage (X, Y, W, H, EINK_BUFFER_1BPP);
}

void BSP_EINK_Log(char *Text, uint8_t checkInteractive) {
    int i = 0;
    const int interactiveMode = 1;
    if ((Text == NULL) || (Text[0] == 0))
        return;

    if (strlen(Text) > EINK_LINE_LEN)
        Text[EINK_LINE_LEN - 1] = 0;

    /* Scroll text up */
    for (i = (EINK_MAX_LINES - 1); i > 0; --i) {
        if (EINK_Text_Buffer[i-1][0] == 0)
            continue;
        memset(EINK_Text_Buffer[i], ' ', EINK_LINE_LEN - 1);
        memcpy(EINK_Text_Buffer[i], EINK_Text_Buffer[i-1], strlen(EINK_Text_Buffer[i-1]));
        EINK_Text_Buffer[i][EINK_LINE_LEN] = 0;
    }

    memset(EINK_Text_Buffer[0], ' ', EINK_LINE_LEN);
    memcpy(EINK_Text_Buffer[0], Text, strlen(Text));
    EINK_Text_Buffer[0][EINK_LINE_LEN] = 0;

    i = 1;
    while ((EINK_Text_Buffer[i-1][0] != 0) && (i < EINK_MAX_LINES)) {
        if ((checkInteractive) && (interactiveMode == 1))
            BSP_EINK_DisplayStringAt(Font24.Width, BOARD_EINK_DISPLAY_RES_Y - (i * Font24.Height), EINK_Text_Buffer[i - 1], LEFT_MODE);
        i++;
    }
    if ((checkInteractive) && (interactiveMode == 1))
        BSP_EINK_Refresh_Text( 0, 0, BOARD_EINK_DISPLAY_RES_Y, BOARD_EINK_DISPLAY_RES_X );

}

void BSP_EINK_Clear_Log( void ) {
    memset(EINK_Text_Buffer, 0x00, sizeof(EINK_Text_Buffer));
}

void BSP_EINK_Display_Init( void ) {
    int i;

    //fill white screen table
    EINK_BUFFER_1BPP[0] = EinkDataStartTransmission1;
    EINK_BUFFER_1BPP[1] = 0x00;
    for (i = 0; i < (480*600/8); i++)
        EINK_BUFFER_1BPP[2 + i] = 0xFF;

    EinkInitialize(Eink1Bpp);
}
