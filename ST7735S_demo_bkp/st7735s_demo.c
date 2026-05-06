#define CS_HIGH GPIOA->BSRR=1<<1
#define CS_LOW GPIOA->BSRR=1<<17
#define LCD_DATA GPIOA->BSRR=1<<2
#define LCD_COMMAND GPIOA->BSRR=1<<18
#include<stm32f407xx.h>
volatile unsigned long long count=0;
void TIM6_DAC_IRQHandler(){
	TIM6->SR=0;		
	count++;
}
void sendcommand(uint32_t cmd){
	LCD_COMMAND;
	CS_LOW;
	SPI2->DR=cmd;
	while(!(SPI2->SR&1<<1));
	while(!(SPI2->SR&1<<0));
	CS_HIGH;
}

void senddata(uint32_t data){
	LCD_DATA;
	CS_LOW;
	SPI2->DR=data;
	while(!(SPI2->SR&1<<1));
	while(!(SPI2->SR&1<<0));
	CS_HIGH;
}

int main(){
RCC->CR|=0x83;
RCC->CFGR|=0;
RCC->AHB1ENR|=1<<0|1<<1;
RCC->APB1ENR|=1<<14|1<<4;
GPIOA->MODER|=1<<2|1<<4|1<<6;
GPIOA->OTYPER=0;
GPIOA->OSPEEDR|=3<<2|3<<4|3<<6;
GPIOA->PUPDR|=0;
GPIOB->MODER|=2<<20|2<<30;
GPIOB->OTYPER=0;
GPIOB->OSPEEDR|=3<<20|3<<30;
GPIOB->PUPDR|=0;
GPIOB->AFR[1]=5<<8|5<<28;
GPIOA->BSRR=1<<1|1<<3; //CS pin and RST pin
SPI2->CR1=0x034C;
TIM6->CR1=1<<0;
TIM6->CNT=0;
TIM6->PSC=0;
TIM6->ARR=159;
TIM6->DIER=1<<0;
NVIC_EnableIRQ(TIM6_DAC_IRQn);
count=0;
GPIOA->BSRR=1<<19;//rst pin low
while(count<2);
GPIOA->BSRR=1<<3; //rst pin high
TIM6->PSC=3;
TIM6->ARR=39999;
count=0;
while(count<12);
sendcommand(0x11);//Sleep exit
count=0;
while(count<12);
sendcommand(0xB1);
senddata(0x01);
senddata(0x2C);
senddata(0x2D);
sendcommand(0xB2); 
	senddata(0x01); 
	senddata(0x2C); 
	senddata(0x2D); 
	sendcommand(0xB3); 
	senddata(0x01); 
	senddata(0x2C); 
	senddata(0x2D); 
	senddata(0x01); 
	senddata(0x2C); 
	senddata(0x2D); 	
	sendcommand(0xB4); //Column inversion 
	senddata(0x07);
	//ST7735R Power Sequence
	sendcommand(0xC0); 
	senddata(0xA2); 
	senddata(0x02); 
	senddata(0x84); 
	sendcommand(0xC1); 
	senddata(0xC5); 
	sendcommand(0xC2); 
	senddata(0x0A); 
	senddata(0x00); 
	sendcommand(0xC3); 
	senddata(0x8A); 
	senddata(0x2A); 
	sendcommand(0xC4); 
	senddata(0x8A); 
	senddata(0xEE); 
	sendcommand(0xC5); //VCOM 
	senddata(0x23); //0x0E	
	sendcommand(0x36); //MX, MY, RGB mode 
	senddata(0xC0);
	//ST7735R Gamma Sequence
	sendcommand(0xe0); 
	senddata(0x0f); 
	senddata(0x1a); 
	senddata(0x0f); 
	senddata(0x18); 
	senddata(0x2f); 
	senddata(0x28); 
	senddata(0x20); 
	senddata(0x22); 
	senddata(0x1f); 
	senddata(0x1b); 
	senddata(0x23); 
	senddata(0x37); 
	senddata(0x00); 	
	senddata(0x07); 
	senddata(0x02); 
	senddata(0x10); 
	sendcommand(0xe1); 
	senddata(0x0f); 
	senddata(0x1b); 
	senddata(0x0f); 
	senddata(0x17); 
	senddata(0x33); 
	senddata(0x2c); 
	senddata(0x29); 
	senddata(0x2e); 
	senddata(0x30); 
	senddata(0x30); 
	senddata(0x39); 
	senddata(0x3f); 
	senddata(0x00); 
	senddata(0x07); 
	senddata(0x03); 
	senddata(0x10);  	
	sendcommand(0x2a);
	senddata(0x00);
	senddata(0x00);
	senddata(0x00);
	senddata(0x7f);
	sendcommand(0x2b);
	senddata(0x00);
	senddata(0x00);
	senddata(0x00);
	senddata(0x9f);
	sendcommand(0xF0); //Enable test command  
	senddata(0x01); 
	sendcommand(0xF6); //Disable ram power save mode 
	senddata(0x00); 	
	sendcommand(0x3A); //65k mode 
	senddata(0x05); 
	sendcommand(0x29);//Display on	
	
	sendcommand(0x36);
	senddata((0<<3)|(1<<6)|(1<<7));
	//set window
	sendcommand(0x2A);
	senddata(0x00);
	senddata(0);
	senddata(0x00);
	senddata(127);
	sendcommand(0x2B);
	senddata(0x00);
	senddata(0);
	senddata(0x00);
	senddata(159);
	//RAM write
	sendcommand(0x2C);
	CS_LOW;
	LCD_DATA;
	uint16_t color=0xF800;
	uint16_t color1=0x07E0;
	for(int i=0;i<(128/2);i++){
		for(int j=0;j<(160);j++){
				senddata(color>>8);
				senddata(color);
		}
	}
	for(int i=0;i<(128/2);i++){
		for(int j=0;j<(160);j++){
				senddata(color1>>8);
				senddata(color1);
		}
	}
	CS_HIGH;
	//set window
	sendcommand(0x2A);
	senddata(0x00);
	senddata(0);
	senddata(0x00);
	senddata(40);
	sendcommand(0x2B);
	senddata(0x00);
	senddata(0);
	senddata(0x00);
	senddata(40);
	//RAM write
	sendcommand(0x2C);
	CS_LOW;
	LCD_DATA;
	uint16_t blue=0x001F;
	for(int i=0;i<(40);i++){
		for(int j=0;j<(40);j++){
				senddata(blue>>8);
				senddata(blue);
		}
	}
	CS_HIGH;
	
	


while(1){
		
}
}

