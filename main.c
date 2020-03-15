typedef unsigned char uint8_t;
typedef unsigned short uint16_t;


//last interrupt
void irq29(void) __interrupt(29)
{
}

typedef struct GPIO_struct
{
	volatile uint8_t ODR;
	volatile uint8_t IDR;
	volatile uint8_t DDR;
	volatile uint8_t CR1;
	volatile uint8_t CR2;
}GPIO;

//10 bit total
typedef struct ADC_BUFFER_struct
{
	//high order register
	volatile uint8_t ADC_DBxRH;
	//low Order register
	volatile uint8_t ADC_DBxRL;
}ADC_BUFFER_struct;



typedef struct ADC_struct
{
	//TODO: maybe put into array
	volatile ADC_BUFFER_struct dataBuffer0;//0x0
	volatile ADC_BUFFER_struct dataBuffer1;//0x2
	volatile ADC_BUFFER_struct dataBuffer2;//0x4
	volatile ADC_BUFFER_struct dataBuffer3;//0x6
	volatile ADC_BUFFER_struct dataBuffer4;//0x8
	volatile ADC_BUFFER_struct dataBuffer5;//0xA
	volatile ADC_BUFFER_struct dataBuffer6;//0xC
	volatile ADC_BUFFER_struct dataBuffer7;//0xE
	volatile ADC_BUFFER_struct dataBuffer8;//0x10
	volatile ADC_BUFFER_struct dataBuffer9;//0x12
	volatile ADC_BUFFER_struct dataBuffer10;//0x14
	volatile ADC_BUFFER_struct dataBuffer11;//0x16
	volatile ADC_BUFFER_struct dataBuffer12;//0x18
	volatile ADC_BUFFER_struct dataBuffer13;//0x1A
	volatile ADC_BUFFER_struct dataBuffer14;//0x1C
	volatile ADC_BUFFER_struct dataBuffer15;//0x1E
	//ADC control/status register 
	volatile uint8_t ADC_CSR;//0x20
	//ADC configuration register 1 
	volatile uint8_t ADC_CR1;
	//ADC configuration register 2 
	volatile uint8_t ADC_CR2;
	//ADC configuration register 3
	volatile uint8_t ADC_CR3;
	//ADC     data     register high 
	volatile uint8_t ADC_DRH;
	//ADC     data     register low 
	volatile uint8_t ADC_DRL;
	//ADC Schmitt trigger disable register high 
	volatile uint8_t ADC_TDRH;
	//ADC Schmitt trigger disable register low
	volatile uint8_t ADC_TDRL;
	//ADC   high   threshold   register high 
	volatile uint8_t ADC_HTRH;
	//ADC   high   threshold   register low 
	volatile uint8_t ADC_HTRL;
	//ADC   low    threshold   register high 
	volatile uint8_t ADC_LTRH;
	//ADC   low    threshold   register low
	volatile uint8_t ADC_LTRL;
	//ADC   watchdog   status   register high 
	volatile uint8_t ADC_AWSRH;
	//ADC   watchdog   status   register low 
	volatile uint8_t ADC_AWSRL;
	//ADC watchdog control register high 
	volatile uint8_t ADC_AWCRH;
	//ADC watchdog control register low 
	volatile uint8_t ADC_AWCRL;

}ADC_struct;

#define ADC1 ((ADC_struct*)0x53E0)
#define GPIOA ((GPIO*)0x5000)
#define GPIOB ((GPIO*)0x5005)
#define GPIOC ((GPIO*)0x500A)
#define GPIOD ((GPIO*)0x500F)

void triggerConversion()
{
	ADC1->ADC_CR1 |= 0x01;
}

void InitGPIO();
//returns 0x00 if invalid input (0x00 also valid output though)
uint8_t GenerateMask(uint8_t hex, uint8_t dp);
//this function changes in and output for the segments and out low and high for the source pins
void SetDisplay(uint8_t display, uint8_t mask);

void ResetGPIO();


void main(void)
{

	//////////////////////////////////////////////////////
	//						  init						//
	//////////////////////////////////////////////////////

	volatile uint16_t data = 0;
	volatile uint16_t dataBuffer = 0;
	volatile uint16_t dataPrint = 0;

	volatile uint8_t hex = 0x00;
	volatile uint8_t dispCounter = 0x01;
	volatile uint8_t hexSwitchH = 0x00;
	volatile uint8_t hexSwitchL = 0x00;

	//CLOCK GATING:


	//INIT ADC
	GPIOA->CR1;
	//activate ADC //this bit also needs to be written to to start conversions
	//for the start of conversation inline assembly (bset) or macro is needed, since otherwise the whole a reg get's pushed
	//confirm ^^^^
	ADC1->ADC_CR1 = 0x01;
	ADC1->ADC_CSR = 0x02;

	

	//InitGPIO();
	GPIOC->DDR |= 0x68;
	GPIOC->CR1 |= 0x68;

	
	//////////////////////////////////////////////////////
	//						init end					//
	//////////////////////////////////////////////////////

	SetDisplay(1, 0xFF);
	triggerConversion();
	for(uint8_t i = 0; i < 0xFF; i++);
	triggerConversion();
	for(uint8_t i = 0; i < 0xFF; i++);

	for (;;)
	{
		triggerConversion();
		for(uint8_t i = 0; i < 0xFF; i++);
		GPIOD->DDR &= ~0x02;//SWIM FAILSAFE
		//if(data == 0)
		{
			data = ((uint16_t)ADC1->ADC_DRH << 2);
			data += (uint16_t)ADC1->ADC_DRL;

		}

		if(data != 0)
		{
			dataBuffer = data;
		}

//highest or lowest?
		if(dispCounter == 3)
		{
			//hex = dataPrint & 0x0F;
			//hex = dataPrint % 10;
			hex = dataPrint / 100;
		}
		else if(dispCounter == 2)
		{
			//hex = (dataPrint >> 4) & 0x0F;
			hex = (dataPrint / 10)%10;
		}
		else
		{
			//hex = (dataPrint >> 8) & 0x0F;
			//hex = dataPrint / 100;
			hex = dataPrint % 10;

		}

		SetDisplay(dispCounter, GenerateMask((hex), dispCounter==0x02));
		//display end
		//logic
		
		dispCounter++;
		//hex++;
		
		if(dispCounter > 3)
		{
			dispCounter = 1;
			//hex -= 3;
		}
		//delay

		hexSwitchL++;

		if(hexSwitchL == 0x60)
		{
			hexSwitchH++;
			hexSwitchL = 0;
		}
		if(hexSwitchH == 0x3)
		{
			hexSwitchH = 0;
			//calculation
			//0V...5V -> 0°C ... 78,125°C
			//dataPrint = ((dataBuffer*780)/1024);//780 instead of 78 to have a precision of 0.1
			//how can i increase the precision?
			//dataPrint = ((dataBuffer*78)/102);//780 instead of 78 to have a precision of 0.1
			//780/1024 -> 195/256 //maybe bring in some small inconsistency, like 196/256 to do better
			//doing just that
			//196/256 -> 49/64 //can now fit into 16 bit (0x3FF * 49 -> C3Cf)~78,3°C, accurate enough
			dataPrint = ((dataBuffer*49)/64);//780 instead of 78 to have a precision of 0.1
			//still need to convert to decimal tho
		}
	}	
}




void InitGPIO()
{
	//first every display pin is set as Low Output without interrupts, with pullup/pbuffer enabled
	GPIOA->ODR = 0x00;
	GPIOB->ODR = 0x00;
	GPIOC->ODR = 0x00;
	GPIOD->ODR = 0x00;

	GPIOA->DDR = 0xFF;
	GPIOB->DDR = 0xFF;
	GPIOC->DDR = 0xFF;
	GPIOD->DDR = 0xFD; //swim

	GPIOA->CR1 = 0xFF;
	GPIOB->CR1 = 0xFF;
	GPIOC->CR1 = 0xFF;
	GPIOD->CR1 = 0xFF;

	GPIOA->CR2 = 0x00;
	GPIOB->CR2 = 0x00;
	GPIOC->CR2 = 0x00;
	GPIOD->CR2 = 0x00;


}//end <InitGPIO>

uint8_t GenerateMask(uint8_t hex, uint8_t dp)
{
	uint8_t mask = 0x00;
	
	switch(hex)
	{
		case 0x00: mask = 0xFC;break;//0b11111100u;break;
		case 0x01: mask = 0x60;break;//0b01100000u;break;
		case 0x02: mask = 0xDA;break;//0b11011010u;break;
		case 0x03: mask = 0xF2;break;//0b11110010u;break;
		case 0x04: mask = 0x66;break;//0b01100110u;break;
		case 0x05: mask = 0xB6;break;//0b10110110u;break;
		case 0x06: mask = 0xBE;break;//0b10111110u;break;
		case 0x07: mask = 0xE0;break;//0b11100000u;break;
		case 0x08: mask = 0xFE;break;//0b11111110u;break;
		case 0x09: mask = 0xF6;break;//0b11110110u;break;
		case 0x0A: mask = 0xEE;break;//0b11101110u;break;
		case 0x0B: mask = 0x3E;break;//0b00111110u;break;
		case 0x0C: mask = 0x9C;break;//0b10011100u;break;
		case 0x0D: mask = 0x7A;break;//0b01111010u;break;
		case 0x0E: mask = 0x9E;break;//0b10011110u;break;
		case 0x0F: mask = 0x8E;break;//0b10001110u;break;
	
		default:
		mask = 0x00; 
		break;
	}
	if(dp)
	{
		mask |= 0x01;
	}
	return mask;
}


void SetDisplay(uint8_t display, uint8_t mask)
{
	ResetGPIO();

	//display select
	switch(display)
	{
		case 0x01:
		GPIOC->ODR |= 0x08;
		break;
		case 0x02:
		GPIOC->ODR |= 0x20;
		break;
		case 0x03:
		GPIOC->ODR |= 0x40;
		break;
		default://ERROR
		break;
	}

	//display select end


	//mask
	if(mask & 0x80)
	{
		GPIOD->DDR |= 0x10;
	}
	else
	{
		GPIOD->DDR &= (~0x10);
	}
	
	if(mask & 0x40)
	{
		GPIOD->DDR |= 0x20;
	}
	else
	{
		GPIOD->DDR &= (~0x20);
	}
	if(mask & 0x20)
	{
		GPIOD->DDR |= 0x40;
	}
	else
	{
		GPIOD->DDR &= (~0x40);
	}
	if(mask & 0x10)
	{
		GPIOA->DDR |= 0x02;
	}
	else
	{
		GPIOA->DDR &= (~0x02);
	}
	if(mask & 0x08)
	{
		GPIOA->DDR |= 0x04;
	}
	else
	{
		GPIOA->DDR &= (~0x04);
	}
	if(mask & 0x04)
	{
		GPIOA->DDR |= 0x08;
	}
		else
	{
		GPIOA->DDR &= (~0x08);
	}
	if(mask & 0x02)
	{
		GPIOC->DDR |= 0x80;
	}
		else
	{
		GPIOC->DDR &= (~0x80);
	}
	if(mask & 0x01)
	{
		GPIOD->DDR |= 0x04;
	}
		else
	{
		GPIOD->DDR &= (~0x04);
	}


	//mask end

}

void ResetGPIO()
{
	//first every display pin is set as Low Output without interrupts, with pullup/pbuffer enabled
	GPIOC->ODR &= ~0x81;
	GPIOC->ODR &= ~0xE8;
	GPIOD->ODR &= ~0x08;

	GPIOA->DDR &= ~0x0E;
	//GPIOB->DDR &= ~0x30;
	GPIOC->DDR &= ~0x80;
	GPIOD->DDR &= ~0x7D; //swim

}