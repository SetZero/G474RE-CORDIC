/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include <cstdint>
#include "main.h"

/* REGISTER BASE ADDRESSES */

static constexpr auto RCC_BASE = 0x40021000;

static constexpr inline auto GPIO_A_BASE = 0x48000000;
static constexpr inline auto GPIO_B_BASE = 0x48000400;
static constexpr inline auto GPIO_C_BASE = 0x48000800;
static constexpr inline auto GPIO_D_BASE = 0x48000C00;
static constexpr inline auto GPIO_E_BASE = 0x48001000;
static constexpr inline auto GPIO_F_BASE = 0x48001400;
static constexpr inline auto GPIO_G_BASE = 0x48001800;

/* RCC */
static constexpr auto RCC_AHB1ENR = 0x48;
static constexpr auto RCC_AHB2ENR = 0x4C;
static constexpr auto RCC_APB2ENR = 0x60;

/* GPIO */

static constexpr inline auto GPIO_X_MODER = 0x00;
static constexpr inline auto GPIO_X_OTYPER = 0x04;
static constexpr inline auto GPIO_X_ODER = 0x14;
static constexpr inline auto GPIO_X_BSRR = 0x18;

/* Utils */

volatile uint32_t &memory(const uint32_t loc) {
    return *reinterpret_cast<volatile uint32_t*>(loc);
}

volatile void delay_ms(uint32_t n) {
    for(; n > 0; n--)
        for(uint32_t i = 0; i < 3195; i++);
}


/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {

    //SystemClock_Config();
    memory(RCC_BASE + RCC_AHB2ENR) |= 1u;
    memory(GPIO_A_BASE + GPIO_X_MODER) &= ~(0b11u << 10u);
    memory(GPIO_A_BASE + GPIO_X_MODER) |= (1u << 10u);

    while (true) {
        memory(GPIO_A_BASE + GPIO_X_BSRR) = (1u << 5u);
        //memory(GPIO_A_BASE + GPIO_X_ODER) |= (1u << 5u);
        delay_ms(500);
        //memory(GPIO_A_BASE + GPIO_X_ODER) &= ~(1u << 5u);
        memory(GPIO_A_BASE + GPIO_X_BSRR) = (1u << (5u + 16));
        delay_ms(500);
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
//void SystemClock_Config(void) {
    //RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    //RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    //HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
    /** Initializes the CPU, AHB and APB busses clocks
    */
    /*RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
      Error_Handler();
    }*/
    /** Initializes the CPU, AHB and APB busses clocks
    */
    /*RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
      Error_Handler();
    }*/
//}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */

    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
