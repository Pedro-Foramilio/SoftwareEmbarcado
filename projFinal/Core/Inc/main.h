/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdint.h"
#include <string.h>
#include <time.h>
#include "cmsis_os.h"
#include <funcoes_display.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/*  -=-=-=-  Algumas definicoes desse projeto (Prof. J Ranhel):    -=-=-=-
"t0000" (minha) requisita que outro envie valor do CRONOMETRO
"a0000" (minha) req que outro envie valor do ADC
"p0000" (minha) msg: "display não disponível", parei de mostrar dados da outra
"s0000" (minha/outra) requisita que minha/outra placa mostre seus valores
"n0000" (outra) requisita que minha placa deixe de mostrar valores dela
"AXXXX" (minha/outra) envia 'a' mais os digitos do ADC em ASCII
"TXXXX" (minha/outra) envia 'T' mais os digitos do cronometro em ASCII

A mensagem é montada do LSD BufOUT[0] p/ MSD BufOUT[4], c/ 5 caracteres;
P/ receber dado é preciso requisitá-lo, envie dado após receber requisição;
O intervalo entre solicitacoes deve ser < 100 ms (p/ atualizar decimo de seg).
-=-=-=- */

#define REQCRN "t0000"       // define a string para pedir leitura cronometro
#define REQADC "a0000"       // define a string para pedir leitura adc
#define REQSRV "s0000"       // define a string para solicitar servico
#define REQOFF "n0000"       // define a string para descartar servico
#define MSGDND "p0000"       // define a string msg: "display não disponível"

#define Q_SND_CRN 1
#define Q_SND_ADC 2
#define Q_REQ_CRN 3
#define Q_REQ_ADC 4
#define Q_REQ_SRV 5
#define Q_REQ_OFF 6

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

// macro = função que copia uma string (n chars) para o BufOUT[] (n items)
// char *strncpy(char *dest, const char *src, size_t n)
// copies up to n characters from the string pointed to by *src to 'dest'.
// strlen() function calc the length of a string (returns a size_t ( uint )
#define STR_BUFF(str) do { \
    const char *src = str; \
    strncpy((char *)(BufOUT), src, sizeBuffs); \
} while (0)

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

// essas macros definem kts e delays mais comuns no projeto
#define MD_CRONO 0           // cronômetro =0 incrementa, =1 decrementa
#define DT_MUX_DISP 6        // dt = ~7ms para multiplex display (142 vz/s)
#define DT_NEWREQ 89         // DT = ~90ms p/ nova requisição de dado
// 10 + x*10, x = 3 => 10 + 3*10 = 40
#define DT_ADC 40           // aqui É O SEU samples/seg de acordo com seu RA

#define DT_CRONO 99          // dt = 99 ms (== 100 e ajusta crono, reset p/ 00)
#define DT_DISPLAY_MD1 3999  // modo 1 muda display a cada 4000 ms
#define DT_DISPLAY_MD2 1999  // modo 2 muda display a cada 2000 ms
#define DT_LEDS 120          // intervalo tempo para piscar leds
// para funcoes_display
#define NDIGSDISP 4          // quatro digitos nos displays
#define NUMSEGS 7            // ligar 7 segs (leds) em cada display
#define DIG_APAGADO 0x10       // kte valor p/ apagar um dígito no display
// para interrupções
#define DT_DEBOUNCING 299    // delay P/ debouncing - ~300 ms (no xx_it.c)

#define LED1 GPIO_PIN_15
#define LED2 GPIO_PIN_14
#define LED3 GPIO_PIN_13
#define LED4 GPIO_PIN_12

#define LED_CRON 1
#define LED_ADC 2
#define LED_CRON_EXT 3
#define LED_ADC_EXT 4
#define WFI 5
#define STARTUP 6

#define DISPLAY_INTRN 0
#define DISPLAY_EXTRN 1

#define DT_VARRE_DISPLAY 6

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
