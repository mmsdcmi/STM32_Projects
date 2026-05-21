#include<stm32f407xx.h>
volatile unsigned long pulsetime=0;
float distance=0;
volatile _Bool rise=0;
unsigned short vehicle_distance=0;
volatile uint16_t speed=0;
unsigned short previous_distance=0;
unsigned short sensor_value=0;
short deciding_value=0;
volatile unsigned char user_input_val=0;
volatile _Bool change_val=0,con_start=0;
unsigned short cutoff_val=0,fullspeed_distance=0;
void USART2_Message(char *);
void delay(unsigned long t){
		while(t--);
}
void timer6_delay( unsigned long t){
	pulsetime=0;
	while(pulsetime<t){
				if(TIM6->SR&1<<0){
					TIM6->SR=0;
			    pulsetime++;
		}
	}	
	

}
void enable(){
    GPIOA->BSRR=1<<10;
		timer6_delay(100);
		GPIOA->BSRR=1<<26;
		timer6_delay(200);
		
}

void GPIOB_PinWrite(uint8_t value,uint8_t pin){

			if(value){
					GPIOB->BSRR=1<<pin;
			}
			else{
					GPIOB->BSRR=1<<(pin+16);
			}

}
		
    
	 
void lcd(uint8_t rs, char data){
		if(rs){
				GPIOA->BSRR=1<<8;
		}
		else{
				GPIOA->BSRR=1<<24;
		}
    GPIOB_PinWrite(((data&0xF0)>>4)&1,0);
		GPIOB_PinWrite(((data&0xF0)>>5)&1,1);
		GPIOB_PinWrite(((data&0xF0)>>6)&1,5);
		GPIOB_PinWrite(((data&0xF0)>>7)&1,7);
    enable();
		GPIOB_PinWrite(((data&0x0F))&1,0);
		GPIOB_PinWrite(((data&0x0F)>>1)&1,1);
		GPIOB_PinWrite(((data&0x0F)>>2)&1,5);
		GPIOB_PinWrite(((data&0x0F)>>3)&1,7);
		enable();
}
void string(char *str){
    int i=0;
    while(*(str+i)!='\0'){
        lcd(1,*(str+i));
        i++;
    }
}
void EXTI1_IRQHandler(){
	
	while(GPIOA->IDR&1<<1){
			if(TIM6->SR&1<<0){
					TIM6->SR=0;
			    pulsetime++;
		}
	}
	rise=1;
	EXTI->PR|=1<<1;
}
void EXTI15_10_IRQHandler(){
		if(EXTI->PR&1<<13){
						USART2->DR=vehicle_distance;
						while(!(USART2->SR&1<<6));
						USART2->DR=speed;
						while(!(USART2->SR&1<<6));
						EXTI->PR|=1<<13;
							
		}
		
	
}



void USART2_Message(char *mes){
while(*mes){
         USART2->DR=*mes;
         mes++;
         while(!(USART2->SR&1<<6));
   }}

void distance_trigger(){
	
	GPIOA->BSRR=1<<3;
	timer6_delay(10);
	GPIOA->BSRR=1<<19;
	pulsetime=0;
	//USART2_Message("Pulse Sent");
	while(rise==0);
		

}
float sr04_data(){
	if(rise==0){
								distance_trigger();
						}
						if(rise==1){
							//USART2_Message("Rise");
							if((pulsetime<38000)){
									distance=pulsetime*0.0352;
									timer6_delay(20000);
							}
							rise=0;
							}
return distance;

}
void distance_pwm_control(){
	vehicle_distance=0;
						for(int i=0;i<5;i++){
									sensor_value=sr04_data();
									vehicle_distance+=sensor_value;
						
						}
						vehicle_distance/=5;
						
						deciding_value=vehicle_distance-previous_distance;
									
					
						if(vehicle_distance>=fullspeed_distance){
														TIM1->CCR2=5330;
									
						}
						else if (vehicle_distance>(fullspeed_distance/2) && vehicle_distance<=(fullspeed_distance-1)){
										TIM1->CCR2=4000;
							if(deciding_value>=50){
										TIM1->CCR2=4500;
							}
							else if(deciding_value<=-50){
									TIM1->CCR2=3500;
							}
						
						}
						else if(vehicle_distance<=(fullspeed_distance/2) && vehicle_distance>(fullspeed_distance/4)){
													TIM1->CCR2=2500;
							
							if(deciding_value>=50){
										TIM1->CCR2=3000;
							}
							else if(deciding_value<=-50){
									TIM1->CCR2=2000;
							}
						}
						else if(vehicle_distance<=(fullspeed_distance/4) && vehicle_distance>cutoff_val){
										TIM1->CCR2=1250;
							if(deciding_value>=50){
										TIM1->CCR2=1000;
							}
							else if(deciding_value<=-50){
									TIM1->CCR2=800;
							}
						}
						else if(vehicle_distance<=cutoff_val){
									TIM1->CCR2=0;
						}
						//char buffer[20]={0};
									//sprintf(buffer,"%u\r\n",vehicle_distance);
									//USART2_Message(buffer);
						previous_distance=vehicle_distance;
						lcd(0,0x89);
						lcd(1,'0'+(vehicle_distance/100));
						lcd(1,'0'+((vehicle_distance/10)%10));
						lcd(1,'0'+(vehicle_distance%10));
						lcd(0,0xC6);
						speed=((TIM1->CCR2/5330.0)*100);
						lcd(1,'0'+(speed/100));
						lcd(1,'0'+((speed/10)%10));
						lcd(1,'0'+(speed%10));
						
}
uint8_t eeprom_read(uint8_t swa){
		I2C2->CR1|=1<<8;//START
	while(I2C2->SR2&1<<1);
	while(!(I2C2->SR2&1<<0));
	while(!(I2C2->SR1&1<<0));	
	//(void)I2C2->SR1;
	//delay(1000);
	I2C2->DR=0xA0;//EEPROM ADDRESS write mode
	while(!(I2C2->SR1&1<<1)); // address bit
  while(!(I2C2->SR2&1<<2)); //comms
	while(!(I2C2->SR1&1<<7));
	//(void)I2C2->SR1;
	(void)I2C2->SR2;
	I2C2->DR=0x00; //FWA
	while(!(I2C2->SR1&1<<7));
	I2C2->DR=swa; //SWA
	while(!(I2C2->SR1&1<<7));
	I2C2->CR1|=1<<9; //STOP
	//after address set
	I2C2->CR1|=1<<8;//START
	while(I2C2->SR2&1<<1);
	while(!(I2C2->SR2&1<<0));
	while(!(I2C2->SR1&1<<0));
	(void)I2C2->SR1;
	I2C2->DR=0xA1;//EEPROM ADDRESS read mode
	while(!(I2C2->SR1&1<<1));
	(void)I2C2->SR1;
	(void)I2C2->SR2;
	while(!(I2C2->SR1&1<<6));
	uint8_t data=I2C2->DR;
	I2C2->CR1|=1<<9; //STOP
	return data;
	

}

void eeprom_write(uint8_t swa,uint8_t value){
	I2C2->CR1|=1<<8;//START
	while(I2C2->SR2&1<<1);
	while(!(I2C2->SR2&1<<0));
	while(!(I2C2->SR1&1<<0));
	(void)I2C2->SR1;
	I2C2->DR=0xA0;//EEPROM ADDRESS write mode
	while(!(I2C2->SR1&1<<1));
	(void)I2C2->SR1;
	(void)I2C2->SR2;
	I2C2->DR=0x00; //FWA
	while(!(I2C2->SR1&1<<7));
	I2C2->DR=swa; //SWA
	while(!(I2C2->SR1&1<<7));
	I2C2->DR=value;//data to write
	while(!(I2C2->SR1&1<<7));
	I2C2->CR1|=1<<9; //STOP
	
	
	

}

void userinput(uint16_t address){
			while(1){	
				if(GPIOA->IDR&1<<4){
					
						user_input_val++;
					lcd(0,0xC0);
					lcd(1,'0'+user_input_val);
						change_val=1;
					while(GPIOA->IDR&1<<4);
				}
				else if(GPIOA->IDR&1<<5){
					
						user_input_val--;
					lcd(0,0xC0);
					lcd(1,'0'+user_input_val);
					change_val=1;
					while(GPIOA->IDR&1<<5);
				}
				else if(GPIOA->IDR&1<<6){
					
						if(change_val==1){
							eeprom_write(address,user_input_val);
								lcd(0,0xC0);
								lcd(1,'0'+ user_input_val);
							while(GPIOA->IDR&1<<6);
								break;
							
						}
				}
				else if(GPIOA->IDR&1<<7){
					while(GPIOA->IDR&1<<7);
								break;
					
				}
	}

}

int main(){
RCC->CR|=0x00000083;
RCC->CFGR|=0x00000000;
RCC->APB1ENR|=RCC_APB1ENR_TIM6EN|RCC_APB1ENR_USART2EN;
RCC->AHB1ENR|=RCC_AHB1ENR_GPIOAEN|RCC_AHB1ENR_GPIOBEN;
RCC->APB2ENR|=RCC_APB2ENR_SYSCFGEN|RCC_APB2ENR_TIM1EN;
GPIOA->MODER|=0x00190060;
GPIOA->OTYPER|=0x0000;
GPIOA->OSPEEDR|=0x003F00F0;
GPIOA->PUPDR|=0x00000000;
GPIOA->AFR[0]|=0x00000700;
GPIOA->AFR[1]|=0x00000010;

GPIOB->MODER|=0x00004405|2<<20|2<<22;
GPIOB->OTYPER|=0x0000|1<<10|1<<11;
GPIOB->OSPEEDR|=0x0000CC0F|3<<20|3<<22;
GPIOB->PUPDR|=0x00010000;
GPIOB->AFR[1]=4<<8|4<<12;
USART2->BRR |=  0x683;       // ~92.6k @16MHz
USART2->CR1 |= (1<<13);      // UART mode, 8N1 etc defaults
USART2->CR1 |= (1<<3);
EXTI->IMR|=1<<1|1<<13;
EXTI->RTSR|=1<<1|1<<13;
SYSCFG->EXTICR[0]|=0x0000;
SYSCFG->EXTICR[3]|=1<<4;
NVIC_EnableIRQ(EXTI1_IRQn);
//NVIC_EnableIRQ(EXTI4_IRQn);
//NVIC_EnableIRQ(EXTI9_5_IRQn);
NVIC_EnableIRQ(EXTI15_10_IRQn);
TIM6->CR1=1<<0;
TIM6->CNT=0;
TIM6->PSC=0;
TIM6->ARR=15;

TIM1->CR1=1<<0|1<<7;
TIM1->CR2=0;
TIM1->CCMR1=1<<11|6<<12;
TIM1->CCER=1<<4;
TIM1->BDTR=1<<15;
TIM1->PSC=0;
TIM1->ARR=5332;
TIM1->EGR=1<<0;
TIM1->CCR2=0;//power down
RCC->APB1ENR|=RCC_APB1ENR_I2C2EN;
I2C2->CR1=1<<0;
I2C2->CR2=16;
I2C2->CCR=80;
I2C2->TRISE=0x11;

//USART2_Message(" Hello ");
lcd(0,0x02);
lcd(0,0x28);
lcd(0,0x0E);
timer6_delay(1000000);
if(!(GPIOB->IDR&1<<8)){
	lcd(0,0x01);
	lcd(0,0x80);
	string("Service Mode");
	timer6_delay(1000000);
	lcd(0,0x01);
	lcd(0,0x80);
	string("Full Speed:");
	fullspeed_distance=eeprom_read(0x00);
	user_input_val=fullspeed_distance;
	lcd(0,0xC0);
	lcd(1,'0'+ user_input_val);
	userinput(0x00);
	lcd(0,0x01);
	lcd(0,0x80);
	string("Cutoff:");
	cutoff_val=eeprom_read(0x01);
	user_input_val=cutoff_val;
	lcd(0,0xC0);
	lcd(1,'0'+ user_input_val);
	userinput(0x01);
	lcd(0,0x01);
	lcd(0,0x80);
	string("Config Complete");
	while(!(GPIOB->IDR&1<<8));
	//return 0;
}
fullspeed_distance=(eeprom_read(0x00))*100;
cutoff_val=(eeprom_read(0x01))*100;
while(1){

				if(GPIOB->IDR&1<<12){
						break;
				}
}
            lcd(0,0x01);
						lcd(0,0x80);
						string("Distance: ");
						lcd(0,0xC0);
						string("Speed:");

while(1){
		
			distance_pwm_control();
						
							
					
}
						

}




