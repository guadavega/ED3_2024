/*
 * Programar el microcontrolador para que utilizando un timer y un pin de capture de esta placa sea
 * posible demodular una señal pwm que ingresa por dicho pin (calcular la frecuencia de trabajo y el
 * periodo), y sacar una tensión continua proporcional al ciclo de trabajo a través del DAC de rango
 * dinámico 0-2 V con un rate de 0.5s del promedio de los últimos 10 valores obtenidos en la captura
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_dac.h"
#endif

#include <cr_section_macros.h>

#define MAX_VALUES 10

volatile uint32_t T_On = 0;
volatile uint32_t period = 0;
uint32_t capture_rising = 0;
uint32_t capture_falling = 0;
volatile uint32_t duty_cycle_values[MAX_VALUES];
volatile uint8_t index = 0;
volatile uint32_t suma = 0;

void config_GPIO(){
	PINSEL_CFG_Type pinsel_cfg;
		pinsel_cfg.Portnum = 0;
		pinsel_cfg.Pinnum = 26;
		pinsel_cfg.Funcnum = 2; //P0.26 salida para DAC
		PINSEL_ConfigPin(&pinsel_cfg);

}
void config_TMR0(){
	 TIM_TIMERCFG_Type TimerConfig;
	 TIM_CAPTURECFG_Type captureConfig;

	 //configuración en modo timer
	 TimerConfig.PrescaleOption = TIM_PRESCALE_USVAL;
	 TimerConfig.PrescaleValue = 1;      // 1us
     TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &TimerConfig);

	  //configura la captura en el canal 0
	  captureConfig.CaptureChannel = 0;
	  captureConfig.RisingEdge = ENABLE;     //captura flanco de subida
   	  captureConfig.FallingEdge = ENABLE;    //captura por flanco de bajada
	  captureConfig.IntOnCaption = ENABLE;

	  TIM_ConfigCapture(LPC_TIM0, &captureConfig);
	  TIM_Cmd(LPC_TIM0, ENABLE);  // Inicia el Timer0
	  NVIC_EnableIRQ(TIMER0_IRQn);  // Habilita interrupción de Timer0

}
void config_TMR1(){
	//configuración en modo match
	 TIM_TIMERCFG_Type TimerConfig;
	 TimerConfig.PrescaleOption = TIM_PRESCALE_USVAL;
     TimerConfig.PrescaleValue = 1000000; // cada 1ms
     TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &TimerConfig);

	 //cada 500ms interrumpe con M
	 TIM_MATCHCFG_Type MatchConfig;
     MatchConfig.MatchChannel = 0;
     MatchConfig.IntOnMatch = ENABLE;
     MatchConfig.ResetOnMatch = ENABLE;
     MatchConfig.StopOnMatch = DISABLE;
     MatchConfig.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
     MatchConfig.MatchValue = 500;
     TIM_ConfigMatch(LPC_TIM1, &MatchConfig);

     TIM_Cmd(LPC_TIM1, ENABLE);
     NVIC_EnableIRQ(TIMER1_IRQn); //habilita interrupción por timer1 para dac

}
void config_DAC(){
	DAC_Init(LPC_DAC);
	DAC_SetBias(LPC_DAC, DAC_MAX_CURRENT_700uA);

}

void TMR0_IRQHandler(){
	if(TIM_GetIntCaptureStatus(LPC_TIM0, TIM_CR0_INT)){
		 capture_rising = TIM_GetCaptureValue(LPC_TIM0, 0);  //por flanco ascendente
		 TIM_ClearIntCapturePending(LPC_TIM0, 0);
	}

	if(TIM_GetIntCaptureStatus(LPC_TIM0, TIM_CR1_INT)){
		 capture_falling = TIM_GetCaptureValue(LPC_TIM0, 0); //por flanco descendente
		 T_On = capture_falling - capture_rising;
		 period = capture_falling;
		 TIM_ClearIntCapturePending(LPC_TIM0, 1);
	}
}

void TMR1_IRQHandler(){
	if (TIM_GetIntStatus(LPC_TIM1, TIM_MR0_INT)) {
		 duty_cycle_values[index] = (T_On * 100) / period;  //calcula el ciclo de trabajo en porcentaje
		 index = (index + 1) % MAX_VALUES; //cuando llegue a 10, vuelve a cero

		 for (int i = 0; i < MAX_VALUES; i++) {
		      suma += duty_cycle_values[i];
		    }

		    uint32_t prom_duty_cycle = suma / MAX_VALUES; //promedio de los ultimos 10 ciclos de trabajo
		    uint32_t dac_value = (prom_duty_cycle * 1023) / 100; //ciclo de trabajo a valor DAC (0-1023)
		    DAC_UpdateValue(LPC_DAC, DAC_VALUE(dac_value)); // actualiza DAC cada 0.5s

	        TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
	    }
}
int main(void) {

    config_GPIO();
    config_TMR0();
    config_TMR1();
    config_DAC();

    while(1) {

    }
    return 0 ;
}
