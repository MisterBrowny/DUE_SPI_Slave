#include <SPI.h>

// Prototypes
ISR(SPI0_Handler);

void	SPI_Slave_Initialize (unsigned long Mode);
void	SPI_Mask_Interrupts (void);
void	SPI_Unmask_Interrupts (void);
void	SPI_Print_Data (void);


// Declaration
// SPI
#define			NB_DATAS			100	// Nb data to receive by frame
#define			SPI_TIME_OUT		5	// Time between each frame (ms)

typedef struct	StructSpi{
	unsigned int	Counter;
	unsigned long	Last_Time_Rcv;
	unsigned char	Data[NB_DATAS];
	bool			Check_Time_Out;
}StruSpi;

StruSpi		Spi0;


// Interruption
ISR (SPI0_Handler)
{
	/*
	// Activate Overrun to check if there is no data lost
	if (REG_SPI0_SR & SPI_SR_OVRES)
	{
		// At least, 1 byte lost 
	}
	*/

	if (REG_SPI0_SR & SPI_SR_RDRF)
	{
		// Store data received in buffer
		Spi0.Data[Spi0.Counter] = REG_SPI0_RDR;

		// Active the time out check
		Spi0.Check_Time_Out = true;
		Spi0.Last_Time_Rcv = millis();
	}
}

void setup()
{
	// SPI initialization
	SPI_Slave_Initialize(SPI_MODE1);
}

void loop()
{   
	if (Spi0.Check_Time_Out == true)
	{
		if ((millis() - Spi0.Last_Time_Rcv) > SPI_TIME_OUT)
		{
			Spi0.Check_Time_Out = false;
			Spi0.Counter = 0;

			SPI_Print_Data();
		}
	}
}

void SPI_Slave_Initialize (unsigned long Mode)
{
	// SPI on Arduino DUE
	// MOSI	ICSP-4
	// MISO	ICSP-1
	// SCK	ICSP-3
	// SS0  pin10
	SPI.begin();
	REG_SPI0_CR = SPI_CR_SWRST;			// reset SPI
	REG_SPI0_CR = SPI_CR_SPIEN;			// enable SPI
	REG_SPI0_MR = SPI_MR_MODFDIS;		// slave and no modefault
	REG_SPI0_CSR = Mode;				// DLYBCT=0, DLYBS=0, SCBR=0, 8 bit transfer
	REG_SPI0_IER = SPI_IER_RDRF;		// active RX interruption on SPI
	//REG_SPI0_IER |= SPI_IER_OVRES;	// active Overrun RX interruption on SPI
	NVIC_EnableIRQ(SPI0_IRQn);			// active interruptions on SPI
}

void SPI_Mask_Interrupts (void)
{
	REG_SPI0_IMR = SPI_IMR_RDRF;
	//REG_SPI0_IMR |= SPI_IMR_OVRES;
	
}

void SPI_Unmask_Interrupts (void)
{
	REG_SPI0_IMR &= ~(SPI_IMR_RDRF);
	//REG_SPI0_IMR &= ~(SPI_IMR_RDRF | SPI_IMR_OVRES);
}


