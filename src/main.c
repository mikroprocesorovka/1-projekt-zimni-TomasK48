#include "stm8s.h"
#include "milis.h"
#include "spse_stm8.h"
#include "stm8_hd44780.h"

/*#include "delay.h"*/
#include <stdio.h>*/
#include "../lib/uart.c"*/

#define _ISOC99_SOURCE
#define _GNU_SOURCE

#define LED_PORT GPIOC
#define LED_PIN  GPIO_PIN_5
#define LED_HIGH   GPIO_WriteHigh(LED_PORT, LED_PIN)
#define LED_LOW  GPIO_WriteLow(LED_PORT, LED_PIN)
#define LED_TOGG GPIO_WriteReverse(LED_PORT, LED_PIN)

#define BTN_PORT GPIOE
#define BTN_PIN  GPIO_PIN_4
#define BTN_PUSH (GPIO_ReadInputPin(BTN_PORT, BTN_PIN)==RESET) 

#define NCODER_CLK_PORT GPIOE
#define NCODER_DATA_PORT GPIOE
#define NCODER_CLK_PIN GPIO_PIN_1
#define NCODER_DATA_PIN GPIO_PIN_2
#define NCODER_GET_CLK (GPIO_ReadInputPin(NCODER_CLK_PORT, NCODER_CLK_PIN)!=RESET)
#define NCODER_GET_DATA (GPIO_ReadInputPin(NCODER_DATA_PORT, NCODER_DATA_PIN)!=RESET)
#define NCODER_SWT_PORT GPIOB
#define NCODER_SWT_PIN GPIO_PIN_7
#define NCODER_SWT (GPIO_ReadInputPin(NCODER_SWT_PORT, NCODER_SWT_PIN)== RESET)

/*
#define LCD_RS_PORT GPIOF
#define LCD_RW_PORT GPIOF
#define LCD_E_PORT GPIOF
#define LCD_D4_PORT GPIOG
#define LCD_D5_PORT GPIOG
#define LCD_D6_PORT GPIOG
#define LCD_D7_PORT GPIOG

#define LCD_RS_PIN GPIO_PIN_7
#define LCD_RW_PIN GPIO_PIN_6
#define LCD_E_PIN GPIO_PIN_5
#define LCD_D4_PIN GPIO_PIN_0
#define LCD_D5_PIN GPIO_PIN_1
#define LCD_D6_PIN GPIO_PIN_2
#define LCD_D7_PIN GPIO_PIN_3
*/

void setup(void)
{
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);      // taktovani MCU na 16MHz

    GPIO_Init(LED_PORT, LED_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);

    init_uart();
    init_milis();    //spustit časovač milis
    lcd_init();
    /*
    GPIO_Init(LCD_RS_PORT, LCD_RS_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(LCD_RW_PORT, LCD_RW_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(LCD_E_PORT, LCD_E_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(LCD_D4_PORT, LCD_D4_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(LCD_D5_PORT, LCD_D5_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(LCD_D6_PORT, LCD_D6_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(LCD_D7_PORT, LCD_D7_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
    */
    // na pinech/vstupech ADC_IN2 (PB2) a ADC_IN3 (PB3) vypneme vstupní buffer
    ADC2_SchmittTriggerConfig(ADC2_SCHMITTTRIG_CHANNEL4, DISABLE);
    ADC2_SchmittTriggerConfig(ADC2_SCHMITTTRIG_CHANNEL5, DISABLE);
    // při inicializaci volíme frekvenci AD převodníku mezi 1-4MHz při 3.3V
    // mezi 1-6MHz při 5V napájení
    // nastavíme clock pro ADC (16MHz / 4 = 4MHz)
    ADC2_PrescalerConfig(ADC2_PRESSEL_FCPU_D4);
    // volíme zarovnání výsledku (typicky vpravo, jen vyjmečně je výhodné vlevo)
    ADC2_AlignConfig(ADC2_ALIGN_RIGHT);
    // nasatvíme multiplexer na některý ze vstupních kanálů
    ADC2_Select_Channel(ADC2_CHANNEL_4);
    // rozběhneme AD převodník
    ADC2_Cmd(ENABLE);
    // počkáme než se AD převodník rozběhne (~7us)
    ADC2_Startup_Wait();
}

int8_t ncoder(void)
{
    uint8_t minuly = 0;
    if (minuly == 0 && NCODER_GET_CLK == 1){
        minuly = 1;
    if (NCODER_GET_DATA == 0){
        return 1; 
    }else{
        return -1;
    }
    }else if (minuly == 1 && NCODER_GET_CLK ==0){
        minuly = 0;
    if (NCODER_GET_DATA ==0){
        return -1;
    }else{
        return 1;
        }
    }
    return 0;
}

void main(void)
{
    char text[32];
    uint32_t voltage = 0;
    uint32_t temperature = 0;
    uint32_t time = 0;
    uint16_t ADCx= 0;
    uint16_t hodnota = 0;
    setup();

    while (1) {

        if (milis() - time > 333 && BTN_PUSH) {
            LED_TOGG; 
            time = milis();
            
            ADCx = ADC_get(ADC2_CHANNEL_4);
            voltage = (uint32_t)3300 * ADCx / 1024;
            //temperature = ((uint32_t)33000 * ADCx - (uint32_t)4000 * 1024) / 1024 /195;
            temperature = ((uint32_t)33000 * ADCx - (uint32_t)4096000 +19968/2) / 19968;
            printf("value %ld \n \r", voltage);
            printf("teplota %ld.%ld °C \n \r", temperature/10,temperature%10);

            lcd_clear();
            lcd_gotoxy(0,0);
            sprintf(text,"Teplota: ",temperature/10);
            lcd_puts(text);
        }
        hodnota += ncoder();
    }
}

/*-------------------------------  Assert -----------------------------------*/
#include "__assert__.h"
