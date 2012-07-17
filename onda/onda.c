#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/device.h>
#include <linux/gpio.h>

/*
 * General-Purpose Timer
 */
#define	OMAP3530_GPT1_BASE			0x48318000
#define	OMAP3530_GPT2_BASE			0x49032000
#define	OMAP3530_GPT3_BASE			0x49034000
#define	OMAP3530_GPT4_BASE			0x49036000
#define	OMAP3530_GPT5_BASE			0x49038000
#define	OMAP3530_GPT6_BASE			0x4903A000
#define	OMAP3530_GPT7_BASE			0x4903C000
#define	OMAP3530_GPT8_BASE			0x4903E000
#define	OMAP3530_GPT9_BASE			0x49040000
#define	OMAP3530_GPT10_BASE			0x48086000
#define	OMAP3530_GPT11_BASE			0x48088000
#define	OMAP3530_GPT12_BASE			0x48304000
#define	OMAP3530_GPT1_WKUP_BASE		0x54718000
#define	OMAP3530_GPT_SIZE			0x1000

/*
 * General-Purpose Timer Registers, offset from base
 */
/* Defs for GPTIMER5 to GPTIMER8 */
#define	OMAP3530_GPT_TIOCP_CFG		0x10
#define	OMAP3530_GPT_TISTAT			0x14
#define	OMAP3530_GPT_TISR			0x18
#define	OMAP3530_GPT_TIER			0x1C
#define	OMAP3530_GPT_TWER			0x20
#define	OMAP3530_GPT_TCLR			0x24
#define	OMAP3530_GPT_TCRR			0x28
#define	OMAP3530_GPT_TLDR			0x2C
#define	OMAP3530_GPT_TTGR			0x30
#define	OMAP3530_GPT_TWPS			0x34
#define	OMAP3530_GPT_TMAR			0x38
#define	OMAP3530_GPT_TCAR1			0x3C
#define	OMAP3530_GPT_TSICR			0x40
#define	OMAP3530_GPT_TCAR2			0x44

/* Extra defs for GPTIMER1 to GPTIMER4 Register, and for GPTIMER9 to GPTIMER12 */
#define	OMAP3530_GPT_TPIR			0x048
#define	OMAP3530_GPT_TNIR			0x04C
#define	OMAP3530_GPT_TCVR			0x050
#define	OMAP3530_GPT_TOCR			0x054
#define	OMAP3530_GPT_TOWR			0x058

#define	OMAP3530_SYSCTL_BASE					0x48002000
#define	OMAP3530_SYSCTL_SIZE					0x1000

#define in32(a)		ioread32(a)
#define out32(a,b)	iowrite32(b,a)

MODULE_LICENSE("Dual BSD/GPL");

void *clock, *gpt9, *sys;

u8 polarity = 0;
u32 pincount = 0;
u32 interval = 20;
int teste = 0, teste2 = 1;
int gpio = 1;

void tasklet_function(unsigned long data);

DECLARE_TASKLET(tasklet_example, tasklet_function, 0);

/* sysfs stuff */
static DEFINE_MUTEX(sysfs_lock);

static ssize_t onda_pincount_show(struct device *dev,
                struct device_attribute *attr, char *buf);

static ssize_t onda_interval_show(struct device *dev,
                struct device_attribute *attr, char *buf);
static ssize_t onda_interval_store(struct device *dev,
                struct device_attribute *attr, const char *buf, size_t size);


static struct class_attribute onda_class_attr[] = {
	__ATTR(button_count, 0644, onda_pincount_show, NULL),
	__ATTR(onda_interval, 0644, onda_interval_show, onda_interval_store),
	__ATTR_NULL
};

static struct class onda_drv = {
	.name = "onda",
	.owner = THIS_MODULE,
	.class_attrs = &onda_class_attr,
};

static ssize_t onda_pincount_show(struct device *dev,
                struct device_attribute *attr, char *buf)
{
	ssize_t status;

	mutex_lock(&sysfs_lock);

	status = sprintf(buf, "%d\n", pincount);

	mutex_unlock(&sysfs_lock);
	return status;
}

static ssize_t onda_interval_show(struct device *dev,
                struct device_attribute *attr, char *buf)
{
	ssize_t status;

	mutex_lock(&sysfs_lock);

	status = sprintf(buf, "%d\n", interval);

	mutex_unlock(&sysfs_lock);
	return status;
}

static ssize_t onda_interval_store(struct device *dev,
                struct device_attribute *attr, const char *buf, size_t size)
{
	int value;
	if (sscanf(buf,"%d\n", &value) > 0) {
		interval = value;
	}
	return size;
}

static irqreturn_t gpio_irq_handler(int irq, void *data)
{
	if (gpio_get_value(138) == 0) {
		unsigned int t;
		/* clear the pin 139*/
		gpio_set_value(139, 0);
		
		/* setting the initial timer counter value
		 * cada tick é 80ns */
		t = 0xffffffff - ((interval*1000)/77);
		
		out32(gpt9 + OMAP3530_GPT_TLDR, t);
		out32(gpt9 + OMAP3530_GPT_TCRR, t);
		/* starting timer with PWM */
		out32(gpt9 + OMAP3530_GPT_TCLR, 3 | (1<<12) | (1<<10)); //-- PWM

		polarity = 0;
		pincount++;
	} else {
		out32(gpt9 + OMAP3530_GPT_TCLR, (1<<12) | (1<<10) | (1<<7));
		out32(gpt9 + OMAP3530_GPT_TISR, 2);
		
		/* set the pin 139 */
		gpio_set_value(139, 1);
		tasklet_schedule(&tasklet_example);
	}

	return IRQ_HANDLED;
}

static irqreturn_t timer_irq_handler(int irq, void *data)
{
	if (polarity)
		gpio_set_value(139, 0);
	else
		gpio_set_value(139, 1);

	polarity = (~polarity) & 1;
	out32(gpt9 + OMAP3530_GPT_TISR, 2);
	
	return IRQ_HANDLED;
}

static int onda_init(void)
{
	u32 l;

	int status = class_register(&onda_drv);
	if(status < 0)
		printk("Registering Class Failed\n");

	gpt9 = ioremap(OMAP3530_GPT9_BASE, OMAP3530_GPT_SIZE);
	if (!gpt9) {
		printk(KERN_ERR "Cannot map ioport gpt9");
		return 0;
	}
	clock = ioremap(0x48005000, 8192);
	if (!clock) {
		printk(KERN_ERR "Cannot map ioport clock");
		return 0;
	}
	sys = ioremap(OMAP3530_SYSCTL_BASE, OMAP3530_SYSCTL_SIZE);
	if (!sys) {
		printk(KERN_ERR "Cannot map ioport sys");
		return 0;
	}

	
	/* enabling clocks */
	l = ioread32(clock) /*| (1<<16)*/ | (1<<4) | (1<<10);
	iowrite32(l, clock);

	l = ioread32(clock + 0x10) /*| (1<<16)*/ | (1<<4) | (1<<10);
	iowrite32(l, clock + 0x10);

	status = request_irq(45, timer_irq_handler, 0, "timer_irq_handler", NULL);
	if (status != 0) {
		printk(KERN_ALERT "Error in request_irq 45\n");
		return 0;
	}


	/* setting the PIN 139 to output */
	/* selecting mode 4 function - GPIO 139
	 * selecting pullup and mode 4 function - GPIO 138 */
#define SYS_CONF	((4 << 16) | ((1 << 8) | (1<<3) | 4))
#define SYS_MASK	~(0x10F010F)
	status = gpio_request(138, "button");
	if (status != 0) {
		printk(KERN_ALERT "gpio_request(138)\n");
		return 0;
	}
	
	status = gpio_request(139, "onda");
	if (status != 0) {
		printk(KERN_ALERT "gpio_request(139)\n");
		return 0;
	}

	/* setting mode 2 - PWM */
	l = (in32(sys + 0x174) & ~7 ) | 2;
	out32(sys + 0x174, l);

	/* setting the PIN 138 to input
	 * setting the PIN 139 to output */
	status = gpio_direction_input(138);
	if (status != 0) {
		printk(KERN_ALERT "gpio_direction_input(138)\n");
		return 0;
	}
	
	status = gpio_direction_output(139, 1);
	if (status != 0) {
		printk(KERN_ALERT "gpio_direction_output(138, 1)\n");
		return 0;
	}
	
	
	/* make sure timer has stop */
	out32(gpt9 + OMAP3530_GPT_TCLR, 0);

	/* enabling the interrupt */
	out32(gpt9 + OMAP3530_GPT_TIER, 2); //comentar se PWM
	
	/* configuring PWM */
	out32(gpt9 + OMAP3530_GPT_TCLR, (1<<12) | (1<<10) | (1<<7)); //-- PWM

	status = request_irq(gpio_to_irq(138), gpio_irq_handler, IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, "gpio_irq_handler", NULL);//&teste);
	if (status != 0) {
		printk(KERN_ALERT "Error in request_irq fall 33: %d\n", status);
		return 0;
	}

	return 0;
}
static void onda_exit(void)
{
	iowrite32(0, gpt3 + OMAP3530_GPT_TCLR);
	iowrite32(0, gpt3 + OMAP3530_GPT_TIER);
	free_irq(39, NULL);
	iounmap(gpt9);
	class_unregister(&onda_drv);
}

void tasklet_function(unsigned long data)
{
		gpio_set_value(139, 1);
}

module_init(onda_init);
module_exit(onda_exit);
