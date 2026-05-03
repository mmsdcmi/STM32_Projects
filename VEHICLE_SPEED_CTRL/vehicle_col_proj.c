#include<stm32f407xx.h>
#include<stdio.h>
volatile unsigned long long pulsetime=0;
volatile float distance=0;
volatile unsigned short rise=0;
unsigned long long vehicle_distance=0;
unsigned long long previous_distance=0;
unsigned long long sensor_value=0;
unsigned long long deciding_value=0;
void USART2_Message(char *);
void delay(unsigned long t){
		while(t--);
}
void timer6(){
		if(TIM6->SR&1<<0){
			pulsetime++;
			TIM6->SR=0;
			
		}

}
void EXTI1_IRQHandler(){
	
	while(GPIOA->IDR&1<<1){
			timer6();
	}
	rise=1;
	EXTI->PR=1<<1;
	
	
	
	
	
	

}
void USART2_Message(char *mes){
while(*mes){
         USART2->DR=*mes;
         mes++;
         while(!(USART2->SR&1<<6));
   }}

void distance_trigger(){
	
	GPIOA->BSRR=1<<3;
	pulsetime=0;
	
	while(pulsetime<10){
			timer6();
	}
	pulsetime=0;
	GPIOA->BSRR=1<<19;
	USART2_Message("Pulse Sent");
	while(rise==0);
		

}
unsigned long long sr04_data(){
	if(rise==0){
								distance_trigger();
						}
						else if(rise==1){
							//USART2_Message("Rise");
							if((pulsetime<38000)){
									distance=pulsetime*0.0352;
								
								
									
									
									
									pulsetime=0;
									while(pulsetime<20000){
											timer6();
									}
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
					/*	
						if(vehicle_distance>100){
								TIM1->CCR2=5330;
						}
						else if(vehicle_distance<50){
						
									TIM1->CCR2=2500;
						}
						else if(vehicle_distance<10){
							TIM1->CCR2=0;
						}
						*/
						if(vehicle_distance>previous_distance){
									deciding_value=vehicle_distance-previous_distance;
									
						}
						else{
								deciding_value=previous_distance-vehicle_distance;
						}
						if(vehicle_distance>=100){
														TIM1->CCR2=5330;
									
						}
						else if (vehicle_distance>50 && vehicle_distance<=99){
										TIM1->CCR2=4000;
							if(deciding_value>=20){
										TIM1->CCR2=3300;
							}
						
						}
						else if(vehicle_distance<=50 && vehicle_distance>25){
													TIM1->CCR2=2500;
							if(deciding_value>=40){
											TIM1->CCR2=1800;
							
							}
						}
						else if(vehicle_distance<=25 && vehicle_distance>10){
										TIM1->CCR2=1250;
							if(deciding_value>=55){
										TIM1->CCR2=800;
							}
						}
						else if(vehicle_distance<=10){
									TIM1->CCR2=0;
						}
						char buffer[20]={0};
									sprintf(buffer,"%llu\r\n",vehicle_distance);
									USART2_Message(buffer);
						previous_distance=vehicle_distance;
}
int main(){
RCC->CR|=0x00000083;
RCC->CFGR|=0x00000000;
RCC->APB1ENR|=RCC_APB1ENR_TIM6EN|1<<17;
RCC->AHB1ENR|=RCC_AHB1ENR_GPIOAEN;
RCC->APB2ENR|=RCC_APB2ENR_SYSCFGEN|1<<0;
GPIOA->MODER|=0x00080060;
GPIOA->OTYPER|=0x0000;
GPIOA->OSPEEDR|=0x000C00F0;
GPIOA->PUPDR|=0x00000000;
GPIOA->AFR[0]|=0x00000700;
GPIOA->AFR[1]|=0x00000010;
USART2->BRR |=  0x683;       // ~92.6k @16MHz
USART2->CR1 |= (1<<13);      // UART mode, 8N1 etc defaults
USART2->CR1 |= (1<<3);
EXTI->IMR|=1<<1;
EXTI->RTSR|=1<<1;
SYSCFG->EXTICR[0]|=0x0000;
NVIC_EnableIRQ(EXTI1_IRQn);
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
TIM1->CCR2=2500;//power down
USART2_Message(" Hello ");
while(1){
		
			distance_pwm_control();
						
							
					
}
						

}




