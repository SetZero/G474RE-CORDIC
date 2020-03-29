/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  *
  *
  ******************************************************************************
  */

/* Includes */
#include "stdint.h"
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

[[nodiscard]] volatile uint32_t &memory(const uint32_t loc) {
    return *reinterpret_cast<volatile uint32_t*>(loc);
}

void delay_ms(uint32_t n) {
    for(; n > 0; n--)
        for(uint32_t i = 0; i < 3195; i++);
}


/**
  * @brief  The application entry point.
  * @retval int
  */
int main() {

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
