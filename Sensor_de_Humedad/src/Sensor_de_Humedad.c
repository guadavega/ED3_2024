/*
 * Implementar un sistema que lea el valor de un sensor de temperatura analógico conectado
 * al canal 0 del ADC, configurado para generar una conversión periódica mediante un temporizador.
 * El sistema debe levantar una interrupción cuando la conversión ADC esté lista, y en base al valor
 * de temperatura, tomar decisiones. Si la temperatura supera un umbral predefinido, se debe
 * encender un LED rojo (simulando una alarma), sino un LED verde. Tener en cuenta los valores de
 * referencia del ADC.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#endif

volatile int16_t temperatura_actual = 0;

void config_GPIO(){
	PINSEL_CFG_Type pinsel_cfg_rojo;
	pinsel_cfg_rojo.Portnum = 0;
	pinsel_cfg_rojo.Pinnum = 0;
	pinsel_cfg_rojo.Funcnum = 0; //P0.0 como GPIO para led rojo
	PINSEL_ConfigPin(&pinsel_cfg_rojo);
	FIO_SetDir(0, 0, 1);       //P0.0 como salida

	PINSEL_CFG_Type pinsel_cfg_verde;
	pinsel_cfg_verde.Portnum = 0;
	pinsel_cfg_verde.Pinnum = 1;
	pinsel_cfg_verde.Funcnum = 0; //P0.1 como GPIO para led verde
	PINSEL_ConfigPin(&pinsel_cfg_verde);
	FIO_SetDir(0, 1, 1);  //P0.1 como salida
}

void config_TMR0(){
	 TIM_TIMERCFG_Type timCfg;
	 timCfg.PrescaleOption = TIM_PRESCALE_USVAL; //en microsegundos
	 timCfg.PrescaleValue = 1000; //actualiza el TC cada 1mS

	 //MAT0.1
	 TIM_MATCHCFG_Type matchCfg;
	 matchCfg.MatchChannel = 1;
	 matchCfg.IntOnMatch = DISABLE; //no interrumpe
	 matchCfg.StopOnMatch = DISABLE;
	 matchCfg.ResetOnMatch = ENABLE;
	 matchCfg.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
	 matchCfg.MatchValue = 1000; //para que cada 1 seg el sensor mida la temperatura

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timCfg);
	TIM_ConfigMatch(LPC_TIM0, &matchCfg);
	TIM_Cmd(LPC_TIM0, ENABLE);

}

void config_ADC(){
	//Pin del ADC0
	PINSEL_CFG_Type pinsel_cfg;
	pinsel_cfg.Portnum = 0;
	pinsel_cfg.Pinnum = 23;
	pinsel_cfg.Funcnum = 1; //como AD0.0
	PINSEL_ConfigPin(&pinsel_cfg);

	ADC_StartCmd(LPC_ADC, ADC_START_ON_MAT01); // arranca cuando el match 1 del tmr0 se activa
	ADC_ChannelCmd(LPC_ADC, 0, ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);
	ADC_BurstCmd(LPC_ADC, DISABLE);

	NVIC_EnableIRQ(ADC_IRQn);  //habilita interrupción del ADC

}

void ADC_IRQHandler(){
	uint16_t ADC_value = ADC_ChannelGetData(LPC_ADC, 0);
	temperatura_actual = (ADC_value * 3.3 / 4095.0) * 100.0; //  Pasa 0 - 4095 -> 0°C a 100°C

	 if(temperatura_actual >= 30){
		 GPIO_SetValue(0,0);     //se activa el led rojo
		 GPIO_ClearValue(0,1);
	 }else{
		 GPIO_SetValue(0,1); //se activa el led verde
		 GPIO_ClearValue(0,0);
	 }
	 ADC_IntConfig(LPC_ADC,0, DISABLE);

}
int main(void) {
	config_GPIO();
	config_TMR0();
	config_ADC();


    while(1) {

    }
    return 0 ;
}
