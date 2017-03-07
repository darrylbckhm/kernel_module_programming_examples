#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h> //for put_user

//prototypes instead of header file
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "chardev" //Dev name as appears in /proc/devices
#define BUF_LEN 80 // Max length of message from device

static int Major;
static int Device_Open = 0;

static char msg[BUF_LEN];
static char *msg_Ptr;

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

int init_module(void)
{
	Major = register_chrdev(0, DEVICE_NAME, &fops);

	if(Major < 0){
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
		return Major;
	}

	printk(KERN_INFO "I was assigned major number %d. To talk to \n", Major);
	printk(KERN_INFO "the driver, create a dev file with\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");

	return SUCCESS;
}

// Called when a process trues to open the device file, like 'cat /dev/mycharfile'
static int device_open(struct inode *inode, struct file *file)
{
	static int counter = 0;

	if(Device_Open)
		return -EBUSY;

	Device_Open++;
	sprintf(msg, "I already told you %d times Hello world!\n", counter++);
	msg_Ptr = msg;
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

// Called when a process closes the device file
static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;
	//Decrement the usage count or you may never be able to get rid of the module
	module_put(THIS_MODULE);

	return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
	int bytes_read = 0;
	
	if(*msg_Ptr == 0)
		return 0;

	while(length && *msg_Ptr){
		put_user(*(msg_Ptr++), buffer++);
		length--;
		bytes_read++;
	}

	return bytes_read;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
	printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
	return -EINVAL;
}
