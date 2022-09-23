#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include "morse_code.h"
#include "gpio_led.h"

#define FIRST_MINOR 0
#define MINOR_COUNT 1
#define DEVICE_COUNT 1
#define DEVICE_NAME "code_device"
#define DATA_MAX 50



ssize_t device_read(struct file *filePtr, char __user *userPtr, size_t size, loff_t *offPtr);
ssize_t device_write(struct file *filePtr, const char __user *userPtr, size_t size, loff_t *offPtr);
void code_into_morse(void);
void output_to_led(void);

int unitLength;
module_param(unitLength, int, S_IWUSR|S_IRUSR);

static char buffer[DATA_MAX] = "";
static char coded[255];

static struct cdev cDevice;
static dev_t deviceNum;

static struct file_operations fOps = 
{
	.owner = THIS_MODULE,
	.read = device_read,
	.write = device_write
};




ssize_t device_read(struct file *filePtr, char __user *userPtr, size_t size, loff_t *offPtr)
{
	int not_copied;
	
	printk(KERN_INFO "Read function called\n");
	
	if(size > DATA_MAX)
	{
		printk(KERN_INFO "Read overflow\n");
		
		size = DATA_MAX;
	}
	
	not_copied = copy_to_user(userPtr, &buffer[0], size); 
			
	return (size - not_copied);
}

ssize_t device_write(struct file *filePtr, const char __user *userPtr, size_t size, loff_t *offPtr)
{
	int not_copied;
	
	printk(KERN_INFO "Write function called\n");
	printk(KERN_INFO "size: %d\n", size);
	
	if(size > DATA_MAX)
	{
		printk(KERN_INFO "Read overflow\n");
		
		size = DATA_MAX;
	}

	/* Clearing arrays */
	memset(buffer, 0, sizeof(buffer));
	memset(coded, 0, sizeof(coded));
					
	not_copied = copy_from_user(&buffer[0], userPtr, size);
	
	code_into_morse();
	
	/* LEDS */
	output_to_led();
	
	/* Debug code */
	
	char *ptr = &coded[0];
	
	while(*ptr != '\0')
	{
		printk(KERN_CONT "%c", *ptr);
		ptr++;
	}
		
	printk(KERN_INFO "Coding done\n");
	
	/* Debug code end */
	
	
	return (size - not_copied);
}

static int __init ModuleInit(void)
{
	int retVal = 0;
	
	printk(KERN_INFO "Module initialized.\n");
		
	retVal = alloc_chrdev_region(&deviceNum, FIRST_MINOR, DEVICE_COUNT, DEVICE_NAME);
		
	printk(KERN_INFO "Major number for char device allocated -> %d.\n", MAJOR(deviceNum));
	
	cdev_init(&cDevice, &fOps);
	
	retVal = cdev_add(&cDevice, deviceNum, MINOR_COUNT);
	
	if(retVal < 0)
	{
		printk(KERN_INFO "Adding char device to system failed\n");
		cdev_del(&cDevice);
		unregister_chrdev_region(deviceNum, DEVICE_COUNT);
		
		return retVal;
	}
	

	gpio_led_init();
	gpio_led_set(3);
	gpio_led_clear(3);
	
	return 0;
}

/* This function just changes character to uppercase if char is in rage a-z */
void toUpper(char *cPtr)
{
	if (*cPtr >= 'a' && *cPtr <= 'z')
	{
		*cPtr = *cPtr - 0x20;
	}
}

/* This function translates input array to morse code by going through input array char by char and searching its morse match */
void code_into_morse()
{
	char **morseArrayPtr = &morseCodes[0];
	char *inputPtr = &buffer[0];
	char *outputPtr = &coded[0];
	char *morseSignPtr;
	int index = 0;
	int output_index = 0;
	int ascii_offset = 0x41;
	int ascii_offset_digits = 0x30; 
	
	printk("Morse coding func called\n");
	
	/*going through input array*/
	while(*inputPtr != '\0')
	{
		/* Flipping char to uppercase */
		toUpper(inputPtr);
		
		/* Searching for letters in input array */ 
		if(*inputPtr >= 'A' && *inputPtr <= 'Z')
		{
			index = *inputPtr - ascii_offset;
			
			morseArrayPtr = &morseCodes[index];
			morseSignPtr = *morseArrayPtr;
			
			while(*morseSignPtr != '\0')
			{
				outputPtr[output_index] = *morseSignPtr;
				
				output_index++;
				morseSignPtr++;
			}
			
			/* x simbols spaces between letters */
			outputPtr[output_index] = 'x';
			output_index++;
						
		}
		else if(*inputPtr >= '0' && *inputPtr <= '9')
		{
			index = *inputPtr - ascii_offset_digits;
		
			morseArrayPtr = &morseCodesDigits[index];
			morseSignPtr = *morseArrayPtr;
			
			while(*morseSignPtr != '\0')
			{
				outputPtr[output_index] = *morseSignPtr;
				
				output_index++;
				morseSignPtr++;
			}
			
			/* x simbols spaces between letters */
			outputPtr[output_index] = 'x';
			output_index++;
		}
		else if (*inputPtr >= ' ')
		{
			/* X simbols spaces between letters */
			outputPtr[output_index] = 'X';
			output_index++;
		}
		else
		{
			/* Do nothing since char is not in range */
		}
		
		inputPtr++;
	}
	
	outputPtr[output_index] = '\0';
}

void output_to_led()
{
	char *ptr = &coded[0];
	
	while(*ptr != '\0')
	{
		switch(*ptr)
		{
			case '*':
				gpio_led_blink(3, unitLength, 0);
				break;
			case '-':
				gpio_led_blink(3, 3*unitLength, 0);
				break;
			case ' ':
				gpio_led_blink(3, 0, unitLength);
				break;
			case 'x':
				gpio_led_blink(3, 0, 3*unitLength);
				break;
			case 'X':
				gpio_led_blink(3, 0, 7*unitLength);
				break;
		}
		
		ptr++;
	}
	
}


static void __exit ModuleExit(void)
{

	cdev_del(&cDevice);
	unregister_chrdev_region(deviceNum, DEVICE_COUNT);
	printk(KERN_INFO "Module removed.\n");
}


module_init(ModuleInit);
module_exit(ModuleExit);



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Group 2");
MODULE_DESCRIPTION("This module takes input string up to 50 chars and turns them into morse code.");

