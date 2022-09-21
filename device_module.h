ssize_t device_read(struct file *filePtr, char __user *userPtr, size_t size, loff_t *offPtr);
ssize_t device_write(struct file *filePtr, const char __user *userPtr, size_t size, loff_t *offPtr);
void code_into_morse(void);
