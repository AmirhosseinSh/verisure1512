/**
  ******************************************************************************
  * @file    SAI/SAI_Audio/Src/main.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    21-September-2015
  * @brief   This example provides a description of how to configure SAI
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32F7xx_HAL_Examples
  * @{
  */

/** @addtogroup SAI_Audio
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define MESSAGE1   "     STM32F7xx    "
#define MESSAGE2   " Device running on  " 
#define MESSAGE3   "   STM32756G-EVAL    "

   
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t uwVolume = AUDIO_DEFAULT_VOLUME;
__IO uint8_t uwPauseEnabledStatus = 0;

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void CPU_CACHE_Enable(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /* Enable the CPU Cache */
  CPU_CACHE_Enable();

  /* STM32F7xx HAL library initialization:
       - Configure the Flash ART accelerator on ITCM interface
       - Systick timer is configured by default as source of time base, but user 
         can eventually implement his proper time base source (a general purpose 
         timer for example or other time source), keeping in mind that Time base 
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization: global MSP (MCU Support Package) initialization
     */
  HAL_Init();

  /* Configure the system clock to 200 MHz */
  SystemClock_Config();

  /* -1- Initialize LEDs mounted on EVAL board */
  /* Configure LED1*/
  BSP_LED_Init(LED1);
  
  /* -2- Configure User push-button in Gpio mode */
  BSP_PB_Init(BUTTON_TAMPER, BUTTON_MODE_GPIO);
  
  /* Initialize the LCD */
  BSP_LCD_Init();
  
  BSP_LCD_LayerDefaultInit(1, LCD_FB_START_ADDRESS);
  
  BSP_LCD_SelectLayer(1);
  
  /* Display message on EVAL LCD **********************************************/
  /* Clear the LCD */ 
  BSP_LCD_Clear(LCD_COLOR_BLUE);  
  
  /* Set the LCD Back Color */
  BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
  
  /* Set the LCD Text Color */
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_DisplayStringAt(0, LINE(0), (uint8_t *)MESSAGE1, CENTER_MODE);
  BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)MESSAGE2, CENTER_MODE);
  BSP_LCD_DisplayStringAt(0, LINE(2), (uint8_t *)MESSAGE3, CENTER_MODE);
  
  /* Turn on LEDs available on EVAL *******************************************/
  BSP_LED_On(LED1);
  BSP_LED_On(LED3);
  
  BSP_LCD_SetFont(&Font16);

  /* Initialize the Audio codec and all related peripherals (SAI, I2C, IOs...) */  
  if(BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_BOTH, uwVolume, SAI_AUDIO_FREQUENCY_48K) == 0)
  {
    BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)"====================", CENTER_MODE);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_DisplayStringAt(0, LINE(8), (uint8_t *)"Short press on User button: Volume Down ", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, LINE(9), (uint8_t *)"Long  press on User button: Pause/Resume", CENTER_MODE);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, LINE(8), (uint8_t *)"====================", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, LINE(9), (uint8_t *)"   AUDIO CODEC OK   ", CENTER_MODE);
  }
  else
  {
    BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)"  AUDIO CODEC  FAIL ", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, LINE(6), (uint8_t *)" Try to reset board ", CENTER_MODE);
  }
  
  /* 
  Start playing the file from a circular buffer, once the DMA is enabled, it is 
  always in running state. Application has to fill the buffer with the audio data 
  using Transfer complete and/or half transfer complete interrupts callbacks 
  (EVAL_AUDIO_TransferComplete_CallBack() or EVAL_AUDIO_HalfTransfer_CallBack()...
  */
  AUDIO_Start();
  
  /* Display the state on the screen */
  BSP_LCD_DisplayStringAt(0, LINE(10), (uint8_t *)"       PLAYING...     ", CENTER_MODE);

   
  /* IMPORTANT:
     AUDIO_Process() is called by the SysTick Handler, as it should be called 
     within a periodic process */
   
  /* Infinite loop */
  while(1)
  {

    if (BSP_PB_GetState(BUTTON_TAMPER) != RESET)
    {
       /* User push-button pressed, check if it is a long or a short press */

      /* Insert 350 ms delay */
      HAL_Delay(350);
      
      if (BSP_PB_GetState(BUTTON_TAMPER) == RESET)
      {
        /* User push-button short press : Volume down */
        /* Decrease volume by 15% */
        if (uwVolume > 20)
        {
          uwVolume -= 15;
        }
        else
        {
          uwVolume = 100;
        }
        /* Apply the new volume to the codec */
        BSP_AUDIO_OUT_SetVolume(uwVolume);

        if (uwVolume > 85)
          BSP_LCD_DisplayStringAt(0, LINE(11), (uint8_t *)"     VOL: <     X>    ", CENTER_MODE);
        else if (uwVolume > 70)
          BSP_LCD_DisplayStringAt(0, LINE(11), (uint8_t *)"     VOL: <    X >    ", CENTER_MODE);
        else if (uwVolume > 55)
          BSP_LCD_DisplayStringAt(0, LINE(11), (uint8_t *)"     VOL: <   X  >    ", CENTER_MODE);
        else if (uwVolume > 40)
          BSP_LCD_DisplayStringAt(0, LINE(11), (uint8_t *)"     VOL: <  X   >    ", CENTER_MODE);
        else if (uwVolume > 25)
          BSP_LCD_DisplayStringAt(0, LINE(11), (uint8_t *)"     VOL: < X    >    ", CENTER_MODE);
        else
          BSP_LCD_DisplayStringAt(0, LINE(11), (uint8_t *)"     VOL: <X     >    ", CENTER_MODE);
      }
      else
      {
        /* User push-button pressed for more then 350ms : Pause/Resume  */
        if (uwPauseEnabledStatus == 1)
        { /* Pause is enabled, call Resume */
          BSP_AUDIO_OUT_Resume();
          uwPauseEnabledStatus = 0;
          BSP_LCD_DisplayStringAt(0, LINE(10), (uint8_t *)"       PLAYING ...    ", CENTER_MODE);
        } else
        { /* Pause the playback */
          BSP_AUDIO_OUT_Pause();
          uwPauseEnabledStatus = 1;
          BSP_LCD_DisplayStringAt(0, LINE(10), (uint8_t *)"       PAUSE ...    ", CENTER_MODE);
        }
        while (BSP_PB_GetState(BUTTON_TAMPER) != RESET);
      }
    }
    
    /* Toggle LED1 */
    BSP_LED_Toggle(LED1);
    
    /* Insert 100 ms delay */
    HAL_Delay(100);

  } 
}

/**
  * @brief  Manages the DMA FIFO error event.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_Error_CallBack(void)
{
  /* Display message on the LCD screen */
  BSP_LCD_SetBackColor(LCD_COLOR_RED);
  BSP_LCD_DisplayStringAt(0, LINE(12), (uint8_t *)"       DMA ERROR      ", CENTER_MODE);
  BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
  BSP_LCD_ClearStringLine(10);

  /* Stop the program with an infinite loop */
  while (1)
  {}

  /* could also generate a system reset to recover from the error */
  /* .... */
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 200000000
  *            HCLK(Hz)                       = 200000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 400
  *            PLL_P                          = 2
  *            PLL_Q                          = 8
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 6
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
  
  /* Activate the OverDrive to reach the 200 MHz Frequency */  
  ret = HAL_PWREx_EnableOverDrive();
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2; 
  
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }  
}

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/