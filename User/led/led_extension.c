#include "led_extension.h"
#include "bsp_gpio_led.h"

#include <stdint.h>

static int8_t _ledR, _ledG, _ledB;
#define FLASH_TIME 2

void Led_Flash_R(void)
{
    _ledR = FLASH_TIME;
}

void Led_Flash_G(void)
{
    _ledG = FLASH_TIME;
}

void Led_Flash_B(void)
{
    _ledB = FLASH_TIME;
}


void LedExtension_TimHandler(void)
{
    if (_ledR > 0) {
        _ledR--;
        LED_ON(R_LED_GPIO_PORT, R_LED_GPIO_PIN);
    }
    else if (_ledR == 0) {
        _ledR--;
        LED_OFF(R_LED_GPIO_PORT, R_LED_GPIO_PIN);
    }

    if (_ledG > 0) {
        _ledG--;
        LED_ON(G_LED_GPIO_PORT, G_LED_GPIO_PIN);
    }
    else if (_ledG == 0) {
        _ledG--;
        LED_OFF(G_LED_GPIO_PORT, G_LED_GPIO_PIN);
    }

    if (_ledB > 0) {
        _ledB--;
        LED_ON(B_LED_GPIO_PORT, B_LED_GPIO_PIN);
    }
    else if (_ledB == 0) {
        _ledB--;
        LED_OFF(B_LED_GPIO_PORT, B_LED_GPIO_PIN);
    }
}
