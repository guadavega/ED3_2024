#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_nvic.h"
#include "lpc17xx_gpdma.h"
#include "debug_frmwrk.h"

/************************** PRIVATE DEFINITIONS *************************/
#define _ADC_CHANNEL    ADC_CHANNEL_2
#define ADC_CONVERSION_RATE 200000  // Frecuencia de muestreo a 200 kHz

/** DMA size of transfer */
#define DMA_SIZE        1

/************************** PRIVATE VARIABLES *************************/

/* Terminal Counter flag for Channel 0 */
volatile uint32_t Channel0_TC;
volatile uint32_t Channel0_Err;

void DMA_IRQHandler(void) {
    if (GPDMA_IntGetStatus(GPDMA_STAT_INT, 0)) {
        if (GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 0)) {
            GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, 0);
            Channel0_TC++;
        }
        if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 0)) {
            GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR, 0);
            Channel0_Err++;
        }
    }
}

void config(void) {
    PINSEL_CFG_Type PinCfg;
    GPDMA_Channel_CFG_Type GPDMACfg;
    uint32_t adc_value;

    /* Initialize ADC */
    PinCfg.Funcnum = 1;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    PinCfg.Pinnum = 25;
    PinCfg.Portnum = 0;
    PINSEL_ConfigPin(&PinCfg);

    debug_frmwrk_init();

    // Configurar el ADC para una frecuencia de muestreo más alta
    ADC_Init(LPC_ADC, ADC_CONVERSION_RATE); // 200 kHz
    ADC_ChannelCmd(LPC_ADC, _ADC_CHANNEL, ENABLE); // Activar canal 2

    /* Configuration for DAC */
    PinCfg.Funcnum = 2;
    PinCfg.Pinnum = 26;
    PINSEL_ConfigPin(&PinCfg);
    DAC_Init(LPC_DAC);

    /* GPDMA block section */
    NVIC_DisableIRQ(DMA_IRQn);
    NVIC_SetPriority(DMA_IRQn, ((0x01 << 3) | 0x01));
    GPDMA_Init();

    GPDMACfg.ChannelNum = 0;
    GPDMACfg.SrcMemAddr = 0;
    GPDMACfg.DstMemAddr = (uint32_t)&adc_value;
    GPDMACfg.TransferSize = DMA_SIZE;
    GPDMACfg.TransferWidth = 0;
    GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_P2M;
    GPDMACfg.SrcConn = GPDMA_CONN_ADC;
    GPDMACfg.DstConn = 0;
    GPDMACfg.DMALLI = 0;
    GPDMA_Setup(&GPDMACfg);

    Channel0_TC = 0;
    Channel0_Err = 0;
    NVIC_EnableIRQ(DMA_IRQn);

    while (1) {
        GPDMA_ChannelCmd(0, ENABLE);

        // Iniciar la conversión del ADC y esperar a que finalice
        ADC_StartCmd(LPC_ADC, ADC_START_NOW);
        while (Channel0_TC == 0);
        GPDMA_ChannelCmd(0, DISABLE);

        // Enviar valor del ADC al UART para monitoreo
        _DBG("ADC Value: ");
        _DBD16((uint16_t)ADC_DR_RESULT(adc_value));
        _DBG("\r\n");

        // Enviar el valor capturado directamente al DAC
        DAC_UpdateValue(LPC_DAC, (uint16_t)(adc_value >> 6)); // Mapeo a 10 bits

        GPDMA_Setup(&GPDMACfg);  // Reconfigurar DMA
        Channel0_TC = 0;  // Reset del contador terminal
        Channel0_Err = 0; // Reset de error
    }
    ADC_DeInit(LPC_ADC);
}

int main(void) {
    config();
    return 0;
}