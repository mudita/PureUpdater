/*
 * (c) Code Red Technologies Ltd, 2008-2013
 * (c) NXP Semiconductors 2013-2018
 * Generated linker script file for MIMXRT1052xxxxB
 * Created from memory.ldt by FMCreateLinkMemory
 * Using Freemarker v2.3.23
 * MCUXpresso IDE v10.2.0 [Build 759] [2018-05-15]  on 2018-07-30 10:43:20
 *
 * Modified by Mudita
 */
MEMORY
{
	/* Define each memory region */
	SRAM_OC (rwx) : ORIGIN = 0x20202000, LENGTH = 0xe000 /* 64K bytes (alias RAM) - 16k reserved by HAB*/
	/*SRAM_ITC (rwx) : ORIGIN = 0x0, LENGTH = 0x0*/ /* 0K bytes (alias RAM2) */
	SRAM_DTC (rwx) : ORIGIN = 0x20000000, LENGTH = 0x70000 /* 448K bytes (alias RAM3) */
	BOARD_SDRAM_TEXT (rx) : ORIGIN = 0x80000000, LENGTH = 0x0620000 /* 5.something M bytes for application code */
	BOARD_SDRAM_HEAP (rwx) : ORIGIN = 0x80620000, LENGTH = 0x9d0000 /* 9.somethin M bytes for heap (alias RAM4) */
	BOARD_SDRAM_SIG (rwx) : ORIGIN = 0x80ff0000, LENGTH = 0x10000 /* Scratch area for validate signatures */
}

