#include <string.h>
#include <math.h>
#include <stdio.h>

#include "board.h"
#include "fsl_spi_master_driver.h"
#include "fsl_clock_manager.h"



/*
 *	Borrowed from https://github.com/adafruit/Adafruit-SSD1331-OLED-Driver-Library-for-Arduino
 */
#define SSD1331_COLORORDER_RGB
#define SSD1331_DELAYS_HWFILL		(3)
#define SSD1331_DELAYS_HWLINE		(1)

/*
 *
 */
#define SSD1331_CMD_DRAWLINE 		0x21
#define SSD1331_CMD_DRAWRECT 		0x22
#define SSD1331_CMD_FILL 		0x26
#define SSD1331_CMD_SETCOLUMN 		0x15
#define SSD1331_CMD_SETROW    		0x75
#define SSD1331_CMD_CONTRASTA 		0x81
#define SSD1331_CMD_CONTRASTB 		0x82
#define SSD1331_CMD_CONTRASTC		0x83
#define SSD1331_CMD_MASTERCURRENT 	0x87
#define SSD1331_CMD_SETREMAP 		0xA0
#define SSD1331_CMD_STARTLINE 		0xA1
#define SSD1331_CMD_DISPLAYOFFSET 	0xA2
#define SSD1331_CMD_NORMALDISPLAY 	0xA4
#define SSD1331_CMD_DISPLAYALLON  	0xA5
#define SSD1331_CMD_DISPLAYALLOFF 	0xA6
#define SSD1331_CMD_INVERTDISPLAY 	0xA7
#define SSD1331_CMD_SETMULTIPLEX  	0xA8
#define SSD1331_CMD_SETMASTER 		0xAD
#define SSD1331_CMD_DISPLAYOFF 		0xAE
#define SSD1331_CMD_DISPLAYON     	0xAF
#define SSD1331_CMD_POWERMODE 		0xB0
#define SSD1331_CMD_PRECHARGE 		0xB1
#define SSD1331_CMD_CLOCKDIV 		0xB3
#define SSD1331_CMD_PRECHARGEA 		0x8A
#define SSD1331_CMD_PRECHARGEB 		0x8B
#define SSD1331_CMD_PRECHARGEC 		0x8C
#define SSD1331_CMD_PRECHARGELEVEL 	0xBB
#define SSD1331_CMD_VCOMH 		0xBE


volatile spi_master_state_t		spiMasterState;
volatile spi_master_user_config_t	spiUserConfig;

volatile uint8_t inBuffer[32];
volatile uint8_t payloadBytes[32];



static void LED_toggle_master(void)
{
    GPIO_DRV_TogglePinOutput(kGpioLED1);
}


void
enableSPIpins(void)
{
	CLOCK_SYS_EnableSpiClock(0);


	/*	KL03_SPI_MISO	--> PTA6	(ALT3)		*/
	PORT_HAL_SetMuxMode(PORTA_BASE, 6, kPortMuxAlt3);

	/*	KL03_SPI_MOSI	--> PTA8	(ALT3)		*/
	PORT_HAL_SetMuxMode(PORTA_BASE, 8, kPortMuxAlt3);

	/*	KL03_SPI_SCK	--> PTA9	(ALT3)		*/
	PORT_HAL_SetMuxMode(PORTA_BASE, 9, kPortMuxAlt3);


	/*
	 *	Initialize SPI master. See KSDK13APIRM.pdf Section 70.4
	 *
	 */
	uint32_t			calculatedBaudRate;
	spiUserConfig.polarity		= kSpiClockPolarity_ActiveHigh;
	spiUserConfig.phase		= kSpiClockPhase_FirstEdge;
	spiUserConfig.direction		= kSpiMsbFirst;
	spiUserConfig.bitsPerSec	= 50000;
	SPI_DRV_MasterInit(0 /* SPI master instance */, (spi_master_state_t *)&spiMasterState);
	SPI_DRV_MasterConfigureBus(0 /* SPI master instance */, (spi_master_user_config_t *)&spiUserConfig, &calculatedBaudRate);
	printf("Calculated baud rate is %ld\n", calculatedBaudRate);
}



void
writeCommand(uint8_t commandByte)
{
	spi_status_t status;

	/*
	 *	/CS (PTA12) low
	 */
	GPIO_DRV_ClearPinOutput(kGpioOC);


	payloadBytes[0] = commandByte;
	status = SPI_DRV_MasterTransferBlocking(0	/* master instance */,
					NULL		/* spi_master_user_config_t */,
					(const uint8_t * restrict)&payloadBytes[0],
					(uint8_t * restrict)&inBuffer[0],
					1		/* transfer size */,
					1000		/* timeout in microseconds (unlike I2C which is ms) */);					
	printf("writeCommand, status = %d\n\r", status);

	/*
	 *	/CS (PTA12) high
	 */
	GPIO_DRV_SetPinOutput(kGpioOC);

	return;
}



int main(void)
{
	hardware_init();
	dbg_uart_init();
	OSA_Init();
	GPIO_DRV_Init(0, ledPins);

	OSA_TimeDelay(500);
	LED_toggle_master();
	OSA_TimeDelay(500);
	LED_toggle_master();
	OSA_TimeDelay(500);
	LED_toggle_master();
	OSA_TimeDelay(500);
	LED_toggle_master();
	OSA_TimeDelay(500);
	LED_toggle_master();
	OSA_TimeDelay(500);
	LED_toggle_master();
	OSA_TimeDelay(500);
	LED_toggle_master();
	OSA_TimeDelay(500);
	LED_toggle_master();

	enableSPIpins();


	/*
	 *	RST (PTA2) high->low->high
	 */
	GPIO_DRV_SetPinOutput(kGpioRST);
	OSA_TimeDelay(500);
	GPIO_DRV_ClearPinOutput(kGpioRST);
	OSA_TimeDelay(500);
	GPIO_DRV_SetPinOutput(kGpioRST);
	OSA_TimeDelay(500);


	/*
	 *	Initialization sequence, borrowed from https://github.com/adafruit/Adafruit-SSD1331-OLED-Driver-Library-for-Arduino
	 */
	 writeCommand(SSD1331_CMD_DISPLAYOFF);		// 0xAE
	 writeCommand(SSD1331_CMD_SETREMAP);		// 0xA0
	 writeCommand(0x72);				// RGB Color
	 writeCommand(SSD1331_CMD_STARTLINE);		// 0xA1
	 writeCommand(0x0);
	 writeCommand(SSD1331_CMD_DISPLAYOFFSET);	// 0xA2
	 writeCommand(0x0);
	 writeCommand(SSD1331_CMD_NORMALDISPLAY);	// 0xA4
	 writeCommand(SSD1331_CMD_SETMULTIPLEX);	// 0xA8
	 writeCommand(0x3F);				// 0x3F 1/64 duty
	 writeCommand(SSD1331_CMD_SETMASTER);		// 0xAD
	 writeCommand(0x8E);
	 writeCommand(SSD1331_CMD_POWERMODE);		// 0xB0
	 writeCommand(0x0B);
	 writeCommand(SSD1331_CMD_PRECHARGE);		// 0xB1
	 writeCommand(0x31);
	 writeCommand(SSD1331_CMD_CLOCKDIV);		// 0xB3
	 writeCommand(0xF0);				// 7:4 = Oscillator Frequency, 3:0 = CLK Div Ratio (A[3:0]+1 = 1..16)
	 writeCommand(SSD1331_CMD_PRECHARGEA);		// 0x8A
	 writeCommand(0x64);
	 writeCommand(SSD1331_CMD_PRECHARGEB);		// 0x8B
	 writeCommand(0x78);
	 writeCommand(SSD1331_CMD_PRECHARGEA);		// 0x8C
	 writeCommand(0x64);
	 writeCommand(SSD1331_CMD_PRECHARGELEVEL);	// 0xBB
	 writeCommand(0x3A);
	 writeCommand(SSD1331_CMD_VCOMH);		// 0xBE
	 writeCommand(0x3E);
	 writeCommand(SSD1331_CMD_MASTERCURRENT);	// 0x87
	 writeCommand(0x06);
	 writeCommand(SSD1331_CMD_CONTRASTA);		// 0x81
	 writeCommand(0x91);
	 writeCommand(SSD1331_CMD_CONTRASTB);		// 0x82
	 writeCommand(0x50);
	 writeCommand(SSD1331_CMD_CONTRASTC);		// 0x83
	 writeCommand(0x7D);
	 writeCommand(SSD1331_CMD_DISPLAYON);		//--turn on oled panel    


	/*
	 *	From the SSD1331_1.2.pdf manual, Section 9.2.2 Draw Rectangle
	 *
	 *	1. Enter the “draw rectangle mode” by execute the command 22h
	 *	2. Set the starting column coordinates, Column 1. e.g., 03h.
	 *	3. Set the starting row coordinates, Row 1. e.g., 02h.
	 *	4. Set the finishing column coordinates, Column 2. e.g., 12h
	 *	5. Set the finishing row coordinates, Row 2. e.g., 15h
	 *	6. Set the outline color C, B and A. e.g., (28d, 0d, 0d) for blue color
	 *	7. Set the filled color C, B and A. e.g., (0d, 0d, 40d) for red color
	 *
	 */
	writeCommand(0x22);
	writeCommand(0x00);
	writeCommand(0x00);
	writeCommand(0x30);
	writeCommand(0x30);

	writeCommand(0x00);
	writeCommand(0x00);
	writeCommand(0x20);

	writeCommand(0x00);
	writeCommand(0x00);
	writeCommand(0x20);



	/*
	 *	NOTE:  **** You need to issue an additional rectangle fill commmand here ****
	 */
	
	// First set the fill enable flag to enabled.
	writeCommand(SSD1331_CMD_FILL);	
	writeCommand(0x01);		// Enabled.
	
	// Clear the screen by overwriting with full-screen black rectangle.
	writeCommand(SSD1331_CMD_DRAWRECT);
	writeCommand(0x00);		// X1.
	writeCommand(0x00);		// Y1.
	writeCommand(0x5F);		// X2.
	writeCommand(0x3F);		// Y2.
	writeCommand(0x00);		// R - outline.
	writeCommand(0x00);		// G - outline.
	writeCommand(0x00);		// B - outline.
	writeCommand(0x00);		// R - filled.
	writeCommand(0x00);		// G - filled.
	writeCommand(0x00);		// B - filled.

	// Then redraw the initial rectangle, this time filled.
	writeCommand(SSD1331_CMD_DRAWRECT);
	writeCommand(0x00);		// X1.
	writeCommand(0x00);		// Y1.
	writeCommand(0x30);		// X2.
	writeCommand(0x30);		// Y2.
	writeCommand(0x00);		// R - outline.
	writeCommand(0x00);		// G - outline.
	writeCommand(0x20);		// B - outline.
	writeCommand(0x00);		// R - filled.
	writeCommand(0x00);		// G - filled.
	writeCommand(0x20);		// B - filled.

	// Then sign the work with initials.
	// 'ND'.
	writeCommand(SSD1331_CMD_DRAWLINE);
	writeCommand(0x46);		// X1.
	writeCommand(0x3C);		// Y1.
	writeCommand(0x46);		// X2.
	writeCommand(0x32);		// Y2.
	writeCommand(0xFF);		// R - line.
	writeCommand(0xFF);		// G - line.
	writeCommand(0xFF);		// B - line.

	writeCommand(SSD1331_CMD_DRAWLINE);
	writeCommand(0x46);		// X1.
	writeCommand(0x32);		// Y1.
	writeCommand(0x4E);		// X2.
	writeCommand(0x3C);		// Y2.
	writeCommand(0xFF);		// R - line.
	writeCommand(0xFF);		// G - line.
	writeCommand(0xFF);		// B - line.

	writeCommand(SSD1331_CMD_DRAWLINE);
	writeCommand(0x4E);		// X1.
	writeCommand(0x3C);		// Y1.
	writeCommand(0x4E);		// X2.
	writeCommand(0x32);		// Y2.
	writeCommand(0xFF);		// R - line.
	writeCommand(0xFF);		// G - line.
	writeCommand(0xFF);		// B - line.

	writeCommand(SSD1331_CMD_DRAWLINE);
	writeCommand(0x52);		// X1.
	writeCommand(0x3C);		// Y1.
	writeCommand(0x52);		// X2.
	writeCommand(0x32);		// Y2.
	writeCommand(0xFF);		// R - line.
	writeCommand(0xFF);		// G - line.
	writeCommand(0xFF);		// B - line.

	writeCommand(SSD1331_CMD_DRAWLINE);
	writeCommand(0x52);		// X1.
	writeCommand(0x32);		// Y1.
	writeCommand(0x58);		// X2.
	writeCommand(0x34);		// Y2.
	writeCommand(0xFF);		// R - line.
	writeCommand(0xFF);		// G - line.
	writeCommand(0xFF);		// B - line.

	writeCommand(SSD1331_CMD_DRAWLINE);
	writeCommand(0x58);		// X1.
	writeCommand(0x34);		// Y1.
	writeCommand(0x5A);		// X2.
	writeCommand(0x36);		// Y2.
	writeCommand(0xFF);		// R - line.
	writeCommand(0xFF);		// G - line.
	writeCommand(0xFF);		// B - line.

	writeCommand(SSD1331_CMD_DRAWLINE);
	writeCommand(0x5A);		// X1.
	writeCommand(0x36);		// Y1.
	writeCommand(0x5A);		// X2.
	writeCommand(0x38);		// Y2.
	writeCommand(0xFF);		// R - line.
	writeCommand(0xFF);		// G - line.
	writeCommand(0xFF);		// B - line.

	writeCommand(SSD1331_CMD_DRAWLINE);
	writeCommand(0x5A);		// X1.
	writeCommand(0x38);		// Y1.
	writeCommand(0x58);		// X2.
	writeCommand(0x3A);		// Y2.
	writeCommand(0xFF);		// R - line.
	writeCommand(0xFF);		// G - line.
	writeCommand(0xFF);		// B - line.

	writeCommand(SSD1331_CMD_DRAWLINE);
	writeCommand(0x58);		// X1.
	writeCommand(0x3A);		// Y1.
	writeCommand(0x52);		// X2.
	writeCommand(0x3C);		// Y2.
	writeCommand(0xFF);		// R - line.
	writeCommand(0xFF);		// G - line.
	writeCommand(0xFF);		// B - line.

	while (1)
	{
		//OSA_TimeDelay(500);
		//LED_toggle_master();
	}
}
