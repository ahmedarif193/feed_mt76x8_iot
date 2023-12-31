#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/module.h>
#include <linux/device.h> 		
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/fcntl.h>
#include <linux/spinlock.h>

#include "mt7628-pwm.h"

MODULE_LICENSE("GPL");

#define RALINK_CLK_CFG    0xB0000030 
#define RALINK_AGPIO_CFG  0xB000003C
#define RALINK_GPIOMODE   0xB0000060 

#define RALINK_PWM_BASE   0xB0005000 
#define RALINK_PWM_ENABLE    RALINK_PWM_BASE 

#define PWM_MODE_BIT      15 
#define PWM_GVAL_BIT      8 
#define PWM_IVAL_BIT      7  

enum {
	PWM_REG_CON,
	PWM_REG_GDUR = 0x0C,
	PWM_REG_WNUM = 0x28,
	PWM_REG_DWID = 0x2C,
	PWM_REG_THRE = 0x30,
	PWM_REG_SNDNUM = 0x34,
} PWM_REG_OFF;

#define PWM_NUM  4 
u32 PWM_REG[PWM_NUM] = {
	(RALINK_PWM_BASE + 0x10),    /* pwm0 base */
	(RALINK_PWM_BASE + 0x50),    /* pwm1 base */
	(RALINK_PWM_BASE + 0x90),    /* pwm2 base */
	(RALINK_PWM_BASE + 0xD0)     /* pwm3 base */
};

#define DEVICE_NAME  "mt7628-pwm"

int pwm_major;
int pwm_minor  = 0;
int pwm_device_cnt = 1;
struct cdev pwm_cdev;

static struct class  *pwm_class;
static struct device *pwm_device;

spinlock_t pwm_lock;

static void sooall_pwm_cfg(struct pwm_cfg *cfg)
{
    u32 value;
    unsigned long flags;
    u32 basereg = PWM_REG[cfg->no];

    spin_lock_irqsave(&pwm_lock, flags);
    
    // 1. Configure the PWM control register
    value = le32_to_cpu(*(volatile u32 *)(basereg + PWM_REG_CON));
    value = ((value | (1 << PWM_MODE_BIT)) & ~((1 << PWM_IVAL_BIT) | (1 << PWM_GVAL_BIT)))
            | ((cfg->idelval & 0x1) << PWM_IVAL_BIT)
            | ((cfg->guardval & 0x1) << PWM_GVAL_BIT)
            | ((cfg->clksrc == PWM_CLK_100KHZ) ? 0 : (1 << 3))
            | (cfg->clkdiv & 0x7);
    *(volatile u32 *)(basereg + PWM_REG_CON) = cpu_to_le32(value);    

    // 2. Set the guard duration value
    value = (le32_to_cpu(*(volatile u32 *)(basereg + PWM_REG_GDUR)) & ~0xffff)
            | (cfg->guarddur & 0xffff);
    *(volatile u32 *)(basereg + PWM_REG_GDUR) = cpu_to_le32(value);    

    // 3. Set the wave number value
    value = (le32_to_cpu(*(volatile u32 *)(basereg + PWM_REG_WNUM)) & ~0xffff)
            | (cfg->wavenum & 0xffff);
    *(volatile u32 *)(basereg + PWM_REG_WNUM) = cpu_to_le32(value);    

    // 4. Set the data width value
    value = (le32_to_cpu(*(volatile u32 *)(basereg + PWM_REG_DWID)) & ~0x1fff)
            | (cfg->datawidth & 0x1fff);
    *(volatile u32 *)(basereg + PWM_REG_DWID) = cpu_to_le32(value);    

    // 5. Set the threshold value
    value = (le32_to_cpu(*(volatile u32 *)(basereg + PWM_REG_THRE)) & ~0x1fff)
            | (cfg->threshold & 0x1fff);
    *(volatile u32 *)(basereg + PWM_REG_THRE) = cpu_to_le32(value);    

    spin_unlock_irqrestore(&pwm_lock, flags);
}

static void sooall_pwm_enable(int no)
{
	u32 value;
	unsigned long flags;

	printk(KERN_INFO DEVICE_NAME "enable pwm%d\n", no);

	spin_lock_irqsave(&pwm_lock, flags);
	value = le32_to_cpu(*(volatile u32 *)(RALINK_PWM_ENABLE));
	value |= (1 << no);
	*(volatile u32 *)(RALINK_PWM_ENABLE) = cpu_to_le32(value);	
	spin_unlock_irqrestore(&pwm_lock, flags);
}

static void sooall_pwm_disable(int no)
{
	u32 value;
	unsigned long flags;

	printk(KERN_INFO DEVICE_NAME "disable pwm%d\n", no);

	spin_lock_irqsave(&pwm_lock, flags);
	value = le32_to_cpu(*(volatile u32 *)(RALINK_PWM_ENABLE));
	value &= ~(1 << no);
	*(volatile u32 *)(RALINK_PWM_ENABLE) = cpu_to_le32(value);	
	spin_unlock_irqrestore(&pwm_lock, flags);
}

static void sooall_pwm_getsndnum(struct pwm_cfg *cfg)
{
	u32 value;
	unsigned long flags;
	u32 regbase = PWM_REG[cfg->no];

	spin_lock_irqsave(&pwm_lock, flags);
	value = le32_to_cpu(*(volatile u32 *)(regbase + PWM_REG_SNDNUM));
	cfg->wavenum = value;
	spin_unlock_irqrestore(&pwm_lock, flags);
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
long sooall_pwm_ioctl(struct file *file, unsigned int req, unsigned long arg)
#else
int sooall_pwm_ioctl(struct inode *inode, struct file *file, unsigned int req, unsigned long arg)
#endif
{
	struct pwm_cfg *cfg = (struct pwm_cfg *)arg;
	
	switch (req) {
		case PWM_ENABLE:
			sooall_pwm_enable(cfg->no);
			break;
		case PWM_DISABLE:
			sooall_pwm_disable(cfg->no);
			break;
		case PWM_CONFIGURE:
			sooall_pwm_cfg(cfg);
			break;
		case PWM_GETSNDNUM:
			sooall_pwm_getsndnum(cfg);
			break;
		default:
			return -ENOIOCTLCMD;
	}
	return 0;
}

static int sooall_pwm_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int sooall_pwm_close(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations pwm_fops = {
	.owner = THIS_MODULE,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	.unlocked_ioctl = sooall_pwm_ioctl,
#else
	.ioctl = sooall_pwm_ioctl,
#endif 
	.open = sooall_pwm_open,
	.release = sooall_pwm_close,
};

static int setup_chrdev(void)
{
	dev_t dev;
	int err = 0;

	if (pwm_major) {
		dev = MKDEV(pwm_major, 0);
		err = register_chrdev_region(dev, pwm_device_cnt, DEVICE_NAME);
	} else {
		err = alloc_chrdev_region(&dev, 0, pwm_device_cnt, DEVICE_NAME);
		pwm_major = MAJOR(dev);
	}
	
	if (err < 0) {
		printk(KERN_ERR DEVICE_NAME "get device number failed\n");
		return -1;
	}
	
	cdev_init(&pwm_cdev, &pwm_fops);
	pwm_cdev.owner = THIS_MODULE;
	err = cdev_add(&pwm_cdev, dev, pwm_device_cnt);
	if (err < 0) {
		printk(KERN_ERR DEVICE_NAME "cdev_add failed\n");
		unregister_chrdev_region(dev, pwm_device_cnt);
		return -1;
	}
	return 0;
}

static void clean_chrdev(void)
{
	dev_t dev = MKDEV(pwm_major, 0);

	cdev_del(&pwm_cdev);
	unregister_chrdev_region(dev, pwm_device_cnt);
}

static void setup_gpio(void)
{
	u32 value;
	int i;

	/* enable the pwm clk */
	value = le32_to_cpu(*(volatile u32 *)(RALINK_CLK_CFG));
	value |= (1 << 31);
	*(volatile u32 *)(RALINK_CLK_CFG) = cpu_to_le32(value);	
	
	/* set the agpio cfg of ephy_gpio_aio_en */
	value = le32_to_cpu(*(volatile u32 *)(RALINK_AGPIO_CFG));
	value |= (0xF << 17);
	*(volatile u32 *)(RALINK_AGPIO_CFG) = cpu_to_le32(value);	

	/* set the pwm  mode */
	value = le32_to_cpu(*(volatile u32 *)(RALINK_GPIOMODE));
	value &= ~(3 << 28 | 3 << 30);
	*(volatile u32 *)(RALINK_GPIOMODE) = cpu_to_le32(value);	
	
	/* disable all the pwm */
	for (i = 0; i < PWM_NUM; i++) {
		sooall_pwm_disable(i);	
	}
}

static int sooall_pwm_init(void)
{
	int ret;

	spin_lock_init(&pwm_lock);
	
	ret = setup_chrdev();
	if (ret < 0) 
		return -1;

	pwm_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (!pwm_class) {
		printk(KERN_ERR DEVICE_NAME "class_create failed\n");	
		goto dev_clean;
	}

	pwm_device = device_create(pwm_class, NULL, MKDEV(pwm_major, pwm_minor), NULL, "sooall_pwm");
	if (!pwm_device) {
		printk(KERN_ERR DEVICE_NAME "device_create failed\n");	
		goto class_clean;
	}
		
	setup_gpio();	
		
	printk(KERN_INFO "sooall pwm init success\n"); 
	return 0;

class_clean:
	class_destroy(pwm_class);	
dev_clean:
	clean_chrdev();
	return -1;
}

static void sooall_pwm_exit(void)
{
	device_destroy(pwm_class, MKDEV(pwm_major, pwm_minor));
	class_destroy(pwm_class);	
	clean_chrdev();
	printk(KERN_INFO "sooall pwm exit\n");
}

module_init(sooall_pwm_init);  
module_exit(sooall_pwm_exit);
