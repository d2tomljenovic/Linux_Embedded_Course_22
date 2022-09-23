/* Available LEDs */
#define GPIO_LED_RED    3
#define GPIO_LED_GREEN  15

/* Exported functions */
void gpio_led_init(void);
int gpio_led_set(int led);
int gpio_led_clear(int led);
int gpio_led_blink(int led, unsigned int hold_high, unsigned int hold_low);
