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
#define DATA_OUTPUT_MAX 501

/* IOCTL commands */
#define SET_LED _IOW(235,'0',int*)
#define SET_UNIT _IOW(235,'1',int*)

/* Function prototypes */
int driver_open(struct inode *device_file, struct file *instance);
int driver_close(struct inode *device_file, struct file *instance);
ssize_t device_read(struct file *filePtr, char __user *userPtr, size_t size, loff_t *offPtr);
ssize_t device_write(struct file *filePtr, const char __user *userPtr, size_t size, loff_t *offPtr);
long device_unlocked_ioctl 	(struct file *filePtr, unsigned int cmd, unsigned long arg);
void code_into_morse(void);
void output_to_led(void);

/* Module params */
int unitLength = 100;
int ledUsed = 0; /* 0 for RED, 1 for GREEN*/
module_param(unitLength, int, S_IWUSR|S_IRUSR);
module_param(ledUsed, int, S_IWUSR|S_IRUSR);

/* Global variables */
static char buffer[DATA_MAX] = "";
static char coded[DATA_OUTPUT_MAX];
int coded_array_size = 0;
int LED_ID = GPIO_LED_RED;

static struct cdev cDevice;
static dev_t deviceNum;

static struct class *dev_class;

static struct file_operations fOps = 
{
	.owner = THIS_MODULE,
	.open=driver_open,
	.release=driver_close,
	.read = device_read,
	.write = device_write,
	.unlocked_ioctl = device_unlocked_ioctl
};

long device_unlocked_ioctl (struct file *f, unsigned int cmd, unsigned long arg)
{
    int value;
    int ret;
    
	pr_info("unlocked_ioctl triggered\n");
    
    /* Copy new value from user */
    ret = copy_from_user(&value, (int*)arg, sizeof(value));
	switch(cmd) {
        case SET_LED:
            /* Only 0 and 1 allowed */
            if((value != 0) && (value != 1))
            {
                return -EINVAL;
            }
            
            /* Set new LED */
            LED_ID = (value == 0) ? GPIO_LED_RED : GPIO_LED_GREEN;
            
            break;
        case SET_UNIT:
            /* Only positive numbers allowed */
            if(value < 0)
            {
                return -EINVAL;
            }
            
            /* Set new unit value */
            unitLength = value;

            break;
            
        default:
            /* Unsupported cmd */
            pr_info("Command not supported\n");
            return -EINVAL;
            break;
	}
	return 0;
}

int driver_open(struct inode *device_file, struct file *instance)
{
	printk(KERN_INFO "Device opened\n");
	
	return 0;
}

int driver_close(struct inode *device_file, struct file *instance)
{
	printk(KERN_INFO "Device closed\n");
	
	return 0;
}

ssize_t device_read(struct file *filePtr, char __user *userPtr, size_t size, loff_t *offPtr)
{
	int not_copied;
	ssize_t ret;
	char terminator = '\0';
	
	printk(KERN_INFO "Read function called\n");
	
	/* Size in smaller than coded array */
	if(size < coded_array_size)
	{
		printk(KERN_INFO "Partialy read\n");
		not_copied = copy_to_user(userPtr, &coded[0], size-1);
	 	not_copied += copy_to_user(userPtr+(size-1), &terminator, 1);
	 	*offPtr += size;
	 	ret = size;
	}
	else 
	{
		/* Size is greater than maximal coded array */
		if(size > DATA_OUTPUT_MAX)
		{
			size = DATA_OUTPUT_MAX;
			printk(KERN_INFO "Read overflow\n");
		}
	
		not_copied = copy_to_user(userPtr, &coded[0], coded_array_size); 
		ret = coded_array_size;
	}
	
	return ret;
}

ssize_t device_write(struct file *filePtr, const char __user *userPtr, size_t size, loff_t *offPtr)
{
	int not_copied;
	
	printk(KERN_INFO "Write function called\n");
	printk(KERN_INFO "size: %d\n", size);
	
	if(size > DATA_MAX)
	{
		printk(KERN_INFO "Write overflow\n");
		
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
	int err;
	
	printk(KERN_INFO "Module initialized.\n");
		
	/* Register Char Dev Region */
	if(alloc_chrdev_region(&deviceNum, FIRST_MINOR, DEVICE_COUNT, DEVICE_NAME))
	{
	    printk(KERN_INFO "Registering char device to system failed\n");
	    err=-ENODEV;
		goto err_exit;
	}
	printk(KERN_INFO "Major number for char device allocated -> %d.\n", MAJOR(deviceNum));
	
	/* Alloc Char Dev */
	cdev_init(&cDevice, &fOps);
		
	/* Add Char Dev */
	if(cdev_add(&cDevice, deviceNum, MINOR_COUNT))
	{
		printk(KERN_INFO "Adding char device to system failed\n");
    	err=-ENODEV;
		goto err_unregister_dev;
	}
	
	/* Create device class */
    if((dev_class = class_create(THIS_MODULE,"morse_class")) == NULL){
        pr_err("Cannot create the struct class for device\n");
        goto err_destroy_char_driver;
    }
	
	/* Create device */
    if((device_create(dev_class, NULL, deviceNum, NULL, "morse_device")) == NULL){
            pr_err("Cannot create the Device\n");
            goto err_destroy_class;
    }
    printk(KERN_INFO "Char device created under /dev/morse_device [%d, %d]\n", MAJOR(deviceNum), MINOR(deviceNum));
		
	/* Parse */
	if(ledUsed == 0)
	{
		LED_ID = GPIO_LED_RED;
	}
	else if(ledUsed == 1)
	{
		LED_ID = GPIO_LED_GREEN;
	}
	else
	{
		/* Only 0 and 1 are valid options, defaulting to RED */
	}

    /* Init GPIO */
    gpio_led_init();
	gpio_led_clear(LED_ID);

    return 0;

    err_destroy_class:
        class_destroy(dev_class);
    
    err_destroy_char_driver:
        cdev_del(&cDevice);
        
	err_unregister_dev:
	    unregister_chrdev_region(deviceNum, DEVICE_COUNT);
		
	err_exit:
	    return err;
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
		else if ((*inputPtr == ' ') && (outputPtr[output_index - 1] == 'x'))
		{
			/* X simbols spaces between letters */
			outputPtr[output_index -1] = 'X';
		}
		else
		{
			/* Do nothing since char is not in range */
		}
		
		inputPtr++;
	}
	
	outputPtr[output_index] = '\0';
	
	coded_array_size = output_index;
}

void output_to_led()
{
	char *ptr = &coded[0];
		
	while(*ptr != '\0')
	{
		switch(*ptr)
		{
			case '*':
				gpio_led_blink(LED_ID, unitLength, 0);
				break;
			case '-':
				gpio_led_blink(LED_ID, 3*unitLength, 0);
				break;
			case ' ':
				gpio_led_blink(LED_ID, 0, unitLength);
				break;
			case 'x':
				gpio_led_blink(LED_ID, 0, 3*unitLength);
				break;
			case 'X':
				gpio_led_blink(LED_ID, 0, 7*unitLength);
				break;
		}
		
		ptr++;
	}	
}


static void __exit ModuleExit(void)
{	
	device_destroy(dev_class,deviceNum);
	class_destroy(dev_class);
	
	cdev_del(&cDevice);
	unregister_chrdev_region(deviceNum, DEVICE_COUNT);
	
	printk(KERN_INFO "Module removed.\n");
}


module_init(ModuleInit);
module_exit(ModuleExit);



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Group 2");
MODULE_DESCRIPTION("This module takes input string up to 50 chars and turns them into morse code.");

