#include <asm/io.h>
#include <linux/delay.h>
#include "gpio_led.h"

/* Physical addresses of used registers */
#define GPIO_LED_RED_FCN_REG 0x3f20000c
#define GPIO_LED_GREEN_FCN_REG 0x3f200010

#define GPIO_LED_SET_REG 0x3f200020
#define GPIO_LED_CLEAR_REG 0x3f20002c

/* Register manipulation macros */
#define GPIO_LED_RED_FCN_REG_MASK 0x00038000
#define GPIO_LED_RED_FCN_REG_OFFSET 15

#define GPIO_LED_GREEN_FCN_REG_MASK 0x00e00000
#define GPIO_LED_GREEN_FCN_REG_OFFSET 21

/* Remapped Phys addresses */
unsigned int* gpio_led_red_phys;
unsigned int* gpio_led_green_phys;

unsigned int* gpio_led_set_phys;
unsigned int* gpio_led_clear_phys;

int gpio_led_set(int led)
{
    /* Only Red and Green LED supported */
    if((led != GPIO_LED_RED) && 
       (led != GPIO_LED_GREEN))
       {
            return -1;
       }
       
    /* Set the LED */
    iowrite32(1 << led, gpio_led_set_phys);
    
    return 0;
}

int gpio_led_clear(int led)
{
    /* Only Red and Green LED supported */
    if((led != GPIO_LED_RED) && 
       (led != GPIO_LED_GREEN))
       {
            return -1;
       }
       
    /* Clear the LED */
    iowrite32(1 << led, gpio_led_clear_phys);
    
    return 0;
}

int gpio_led_blink(int led, unsigned int hold_high, unsigned int hold_low)
{
    /* Only Red and Green LED supported */
    if((led != GPIO_LED_RED) && 
       (led != GPIO_LED_GREEN))
       {
            return -1;
       }
       
    /* Set the led and hold it high */
    if(hold_high > 0)
      {
           gpio_led_set(led);
   	       msleep(hold_high);
      }  

   
   /* Clear the led and hold it low */
   gpio_led_clear(led);
   msleep(hold_low);
   
   return 0;
}

void gpio_led_init(void)
{
    unsigned int state;
    
    /* Remap all used registers */
    gpio_led_red_phys = ioremap(GPIO_LED_RED_FCN_REG, 4);
    gpio_led_green_phys = ioremap(GPIO_LED_GREEN_FCN_REG, 4);
    gpio_led_set_phys = ioremap(GPIO_LED_SET_REG, 4);
    gpio_led_clear_phys = ioremap(GPIO_LED_CLEAR_REG, 4);
    
    /* Set Red LED to output */
    state = ioread32(gpio_led_red_phys);
    state &= ~GPIO_LED_RED_FCN_REG_MASK;
    state |= 1 << GPIO_LED_RED_FCN_REG_OFFSET;
    iowrite32(state, gpio_led_red_phys);
    
    /* Set Green LED to output */
    state = ioread32(gpio_led_green_phys);
    state &= ~GPIO_LED_GREEN_FCN_REG_MASK;
    state |= 1 << GPIO_LED_GREEN_FCN_REG_OFFSET;
    iowrite32(state, gpio_led_green_phys);
    
    /* Turn both LEDs off */
    gpio_led_clear(GPIO_LED_RED);
    gpio_led_clear(GPIO_LED_GREEN);
}
