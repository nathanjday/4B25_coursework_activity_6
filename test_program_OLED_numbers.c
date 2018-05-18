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
#define SSD1331_CMD_FILL 			0x26
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
#define SSD1331_CMD_VCOMH 			0xBE

volatile spi_master_state_t		spiMasterState;
volatile spi_master_user_config_t	spiUserConfig;
volatile uint8_t inBuffer[32];
volatile uint8_t payloadBytes[32];

static void LED_toggle_master(void){
    GPIO_DRV_TogglePinOutput(kGpioLED1);
}

void performSetup(){
	// Set up.
	hardware_init();
	dbg_uart_init();
	OSA_Init();
	GPIO_DRV_Init(0, ledPins);
	
	// Confirm alive.
    printf("\r\n====== OLED TEST PROGRAM ======\r\n\r\n");

	// Flash lights.
    OSA_TimeDelay(500);
    LED_toggle_master();
    OSA_TimeDelay(500);
    LED_toggle_master();
    OSA_TimeDelay(500);
    LED_toggle_master();
    OSA_TimeDelay(500);
    LED_toggle_master();
}

void enableSPIpins(void){
	CLOCK_SYS_EnableSpiClock(0);

	/*	KL03_SPI_MISO	--> PTA6	(ALT3)		*/
	PORT_HAL_SetMuxMode(PORTA_BASE, 6, kPortMuxAlt3);
	/*	KL03_SPI_MOSI	--> PTA8	(ALT3)		*/
	PORT_HAL_SetMuxMode(PORTA_BASE, 8, kPortMuxAlt3);
	/*	KL03_SPI_SCK	--> PTA9	(ALT3)		*/
	PORT_HAL_SetMuxMode(PORTA_BASE, 9, kPortMuxAlt3);

	// Initialize SPI master.
	uint32_t calculatedBaudRate;
	spiUserConfig.polarity = kSpiClockPolarity_ActiveHigh;
	spiUserConfig.phase	= kSpiClockPhase_FirstEdge;
	spiUserConfig.direction	= kSpiMsbFirst;
	spiUserConfig.bitsPerSec = 50000;
	SPI_DRV_MasterInit(0 /* SPI master instance */, (spi_master_state_t *)&spiMasterState);
	SPI_DRV_MasterConfigureBus(0 /* SPI master instance */, (spi_master_user_config_t *)&spiUserConfig,								&calculatedBaudRate);
	printf("Calculated baud rate is %ld\n", calculatedBaudRate);
}

void writeCommand(uint8_t commandByte){
	spi_status_t status;

	// CS (PTA12) low.
	GPIO_DRV_ClearPinOutput(kGpioOC);

	payloadBytes[0] = commandByte;
	status = SPI_DRV_MasterTransferBlocking(0	/* master instance */,
					NULL		/* spi_master_user_config_t */,
					(const uint8_t * restrict)&payloadBytes[0],
					(uint8_t * restrict)&inBuffer[0],
					1		/* transfer size */,
					1000		/* timeout in microseconds (unlike I2C which is ms) */);

	// CS (PTA12) high.
	GPIO_DRV_SetPinOutput(kGpioOC);

	return;
}

void initializeOLED(){
	//RST (PTA2) high->low->high.
	GPIO_DRV_SetPinOutput(kGpioRST);
	OSA_TimeDelay(500);
	GPIO_DRV_ClearPinOutput(kGpioRST);
	OSA_TimeDelay(500);
	GPIO_DRV_SetPinOutput(kGpioRST);
	OSA_TimeDelay(500);

	/* Initialization sequence, borrowed from https://github.com/adafruit/Adafruit-SSD1331-OLED-Driver-Library-for-Arduino 
	*/
	writeCommand(SSD1331_CMD_DISPLAYOFF);		
	writeCommand(SSD1331_CMD_SETREMAP);		
	writeCommand(0x72);				
	writeCommand(SSD1331_CMD_STARTLINE);		
	writeCommand(0x0);
	writeCommand(SSD1331_CMD_DISPLAYOFFSET);	
	writeCommand(0x0);
	writeCommand(SSD1331_CMD_NORMALDISPLAY);	
	writeCommand(SSD1331_CMD_SETMULTIPLEX);
	writeCommand(0x3F);				
	writeCommand(SSD1331_CMD_SETMASTER);		
	writeCommand(0x8E);
	writeCommand(SSD1331_CMD_POWERMODE);		
	writeCommand(0x0B);
	writeCommand(SSD1331_CMD_PRECHARGE);		
	writeCommand(0x31);
	writeCommand(SSD1331_CMD_CLOCKDIV);		
	writeCommand(0xF0);				
	writeCommand(SSD1331_CMD_PRECHARGEA);		
	writeCommand(0x64);
	writeCommand(SSD1331_CMD_PRECHARGEB);		
	writeCommand(0x78);
	writeCommand(SSD1331_CMD_PRECHARGEA);		
	writeCommand(0x64);
	writeCommand(SSD1331_CMD_PRECHARGELEVEL);	
	writeCommand(0x3A);
	writeCommand(SSD1331_CMD_VCOMH);		
	writeCommand(0x3E);
	writeCommand(SSD1331_CMD_MASTERCURRENT);	
	writeCommand(0x06);
	writeCommand(SSD1331_CMD_CONTRASTA);		
	writeCommand(0x91);
	writeCommand(SSD1331_CMD_CONTRASTB);		
	writeCommand(0x50);
	writeCommand(SSD1331_CMD_CONTRASTC);		
	writeCommand(0x7D);
	writeCommand(SSD1331_CMD_DISPLAYON);
}

void setOLEDBackground(){
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
}

void drawLineOLED(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2){
	writeCommand(SSD1331_CMD_DRAWLINE);
	writeCommand(x1);		// X1.
	writeCommand(y1);		// Y1.
	writeCommand(x2);		// X2.
	writeCommand(y2);		// Y2.
	writeCommand(0xFF);		// R - line.
	writeCommand(0xFF);		// G - line.
	writeCommand(0xFF);		// B - line.
}
void displayDigitOLED(uint8_t value, uint8_t index){
	uint8_t xLow, xHigh;
	uint8_t yLow = 0x0F;
	uint8_t yMid = 0x1F;
	uint8_t yHigh = 0x2F;
	xLow = (24 * index) + 3;
	xHigh = (24 * index) + 19;
	
	switch(value){
		case 0:
			drawLineOLED(xLow, xLow, yLow, yHigh);
			drawLineOLED(xLow, xHigh, yHigh, yHigh);
			drawLineOLED(xHigh, xHigh, yHigh, yLow);
			drawLineOLED(xHigh, xLow, yLow, yLow);
			break;
		case 1:
			drawLineOLED(xHigh, xHigh, yHigh, yLow);
			break;
		case 2:
			drawLineOLED(xLow, xHigh, yHigh, yHigh);
			drawLineOLED(xHigh, xHigh, yHigh, yMid);
			drawLineOLED(xHigh, xLow, yMid, yMid);
			drawLineOLED(xLow, xLow, yMid, yLow);
			drawLineOLED(xLow, xHigh, yLow, yLow);
			break;
		case 3:
			drawLineOLED(xLow, xHigh, yHigh, yHigh);
			drawLineOLED(xHigh, xHigh, yHigh, yLow);
			drawLineOLED(xHigh, xLow, yMid, yMid);
			drawLineOLED(xHigh, xLow, yLow, yLow);
			break;
		case 4:
			drawLineOLED(xLow, xLow, yHigh, yMid);
			drawLineOLED(xLow, xHigh, yMid, yMid);
			drawLineOLED(xHigh, xLow, yHigh, yHigh);
			break;
		case 5:
			drawLineOLED(xHigh, xLow, yHigh, yHigh);
			drawLineOLED(xLow, xLow, yHigh, yMid);
			drawLineOLED(xLow, xHigh, yMid, yMid);
			drawLineOLED(xHigh, xHigh, yMid, yLow);
			drawLineOLED(xHigh, xLow, yLow, yLow);
			break;
		case 6:
			drawLineOLED(xHigh, xLow, yHigh, yHigh);
			drawLineOLED(xLow, xLow, yHigh, yLow);
			drawLineOLED(xLow, xHigh, yLow, yLow);
			drawLineOLED(xHigh, xHigh, yLow, yMid);
			drawLineOLED(xHigh, xLow, yMid, yMid);
			break;
		case 7:
			drawLineOLED(xLow, xHigh, yHigh, yHigh);
			drawLineOLED(xHigh, xHigh, yHigh, yLow);
			break;
		case 8:
			drawLineOLED(xLow, xLow, yLow, yHigh);
			drawLineOLED(xLow, xHigh, yHigh, yHigh);
			drawLineOLED(xHigh, xHigh, yHigh, yLow);
			drawLineOLED(xHigh, xLow, yLow, yLow);
			drawLineOLED(xHigh, xLow, yMid, yMid);
			break;
		case 9:
			drawLineOLED(xHigh, xHigh, yLow, yHigh);
			drawLineOLED(xHigh, xLow, yHigh, yHigh);
			drawLineOLED(xLow, xLow, yHigh, yMid);
			drawLineOLED(xLow, xHigh, yMid, yMid);
			break;
	}
}

void displayNumberOLED(uint32_t number){
	// 4 digits displayed. Leading 0's always displayed.
	uint8_t digit0, digit1, digit2, digit3;
	digit3 = number % 10;
	number /= 10;
	digit2 = number % 10;
	number /= 10;
	digit1 = number % 10;
	number /= 10;
	digit0 = number % 10;
	
	displayDigitOLED(digit0, 0);
	displayDigitOLED(digit1, 1);
	displayDigitOLED(digit2, 2);
	displayDigitOLED(digit3, 3);
}

int main(void){
	performSetup();
	enableSPIpins();
	initializeOLED();
	setOLEDBackground();
	displayNumberOLED(8888);
	
	while (1)
	{
		// Loop.
	}
}
