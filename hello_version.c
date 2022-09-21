#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include "morse_code.h"

#define FIRST_MINOR 0
#define MINOR_COUNT 1
#define DEVICE_COUNT 1
#define DEVICE_NAME "code_device"
#define DATA_MAX 50


static char buffer[DATA_MAX] = "";
static char coded[255];

static struct cdev cDevice;
static dev_t deviceNum;

ssize_t device_read(struct file *filePtr, char __user *userPtr, size_t size, loff_t *offPtr);
ssize_t device_write(struct file *filePtr, const char __user *userPtr, size_t size, loff_t *offPtr);
void code_into_morse(void);

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
	
	/* Dropping data if more data is received than it fits in buffer */
	if(size > DATA_MAX)
	{
		printk(KERN_INFO "Maximum data that can be received is %d, rest is dropped\n", DATA_MAX);
		size = DATA_MAX;
	}
	
	not_copied = copy_to_user(userPtr, &buffer[0], size); /* Check params */
	
	printk("Not copied value: %d\n", not_copied);
	code_into_morse();
	/* Read function should return 0 -> if it returns number greater than zero user space will assume that not all data is read and will call read again */
	return 15;
}

ssize_t device_write(struct file *filePtr, const char __user *userPtr, size_t size, loff_t *offPtr)
{
	int not_copied;
	
	printk(KERN_INFO "Write function called\n");
	
	buffer[0] = '\0';
	
	if(size > DATA_MAX)
	{
		printk(KERN_INFO "Maximum data that can be written is %d, rest is dropped\n", DATA_MAX);
		size = DATA_MAX;
	}
		
	not_copied = copy_from_user(&buffer[0], userPtr, size);
	
	/* Write function should return number of bytes written -> if 0 is returned it will cause infinite loop because user space assumes write call failed, so it will try again */
	return (size - not_copied);
}

static int __init ModuleInit(void)
{
	int retVal = 0;
	
	printk(KERN_INFO "Module initialized.\n");
		
	retVal = alloc_chrdev_region(&deviceNum, FIRST_MINOR, DEVICE_COUNT, DEVICE_NAME);
	
	if(retVal < 0)
	{
		printk(KERN_INFO "Char device major number allocation failed\n");
		
		return retVal;
	}
	
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
	
	
	return 0;
}


void code_into_morse()
{
	char **morseArrayPtr = &morseCodes[0];
	char *inputPtr = &buffer[0];
	char *outputPtr = &coded[0];
	char *morseSignPtr;
	int index = 0;
	int output_index = 0;
	int ascii_offset = 0x41; 
	
	printk("Morse coding func called\n");
	
	/*going through input array*/
	while(*inputPtr != '\0')
	{
		/* Searching for letters in input array */ /* Handle tolower -> ascii calculations */
		if(*inputPtr >= 'A' && *inputPtr <= 'Z')
		{
			index = *inputPtr - ascii_offset;
			
			morseArrayPtr = &morseCodes[index];
			strcpy(outputPtr, *morseArrayPtr);
		}
		
		//TODO: /* Special check for spaces and digits*/
		/*
		else if()
		{
		
		}
		*/
		inputPtr++;
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

