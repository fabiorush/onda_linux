#include <linux/init.h>
#include <linux/module.h>
//#include <linux/kernel.h>
//#include <linux/errno.h>
#include <linux/interrupt.h>
#include <asm/io.h>
//#include "mux.h"
#include <linux/device.h>
#include <linux/gpio.h>

#define	OMAP3530_GPIO1_BASE			0x48310000
#define	OMAP3530_GPIO2_BASE			0x49050000
#define	OMAP3530_GPIO3_BASE			0x49052000
#define	OMAP3530_GPIO4_BASE			0x49054000
#define	OMAP3530_GPIO5_BASE			0x49056000
#define	OMAP3530_GPIO6_BASE			0x49058000
#define	OMAP3530_GPIO_SIZE			0x1000

#define	OMAP2420_GPIO_REVISION		0x00
#define	OMAP2420_GPIO_SYSCONFIG		0x10
#define	OMAP2420_GPIO_SYSSTATUS		0x14
#define	OMAP2420_GPIO_IRQSTATUS1	0x18
#define	OMAP2420_GPIO_IRQENABLE1	0x1C
#define	OMAP2420_GPIO_WAKEUPENABLE	0x20
#define	OMAP2420_GPIO_IRQSTATUS2	0x28
#define	OMAP2420_GPIO_IRQENABLE2	0x2C
#define	OMAP2420_GPIO_CTRL			0x30
#define	OMAP2420_GPIO_OE			0x34
#define	OMAP2420_GPIO_DATAIN		0x38
#define	OMAP2420_GPIO_DATAOUT		0x3C
#define	OMAP2420_GPIO_LEVELDETECT0	0x40
#define	OMAP2420_GPIO_LEVELDETECT1	0x44
#define	OMAP2420_GPIO_RISINGDETECT	0x48
#define	OMAP2420_GPIO_FALLINGDETECT	0x4C
#define	OMAP2420_GPIO_DEBOUNCENABLE	0x50
#define	OMAP2420_GPIO_DEBOUNCINGTIME	0x54
#define	OMAP2420_GPIO_CLEARIRQENABLE1	0x60
#define	OMAP2420_GPIO_SETIRQENABLE1	0x64
#define	OMAP2420_GPIO_CLEARIRQENABLE2	0x70
#define	OMAP2420_GPIO_SETIRQENABLE2	0x74
#define	OMAP2420_GPIO_CLEARWKUENA	0x80
#define	OMAP2420_GPIO_SETWKUENA		0x84
#define	OMAP2420_GPIO_CLEARDATAOUT	0x90
#define	OMAP2420_GPIO_SETDATAOUT	0x94

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

void *gpio5, *gpt3, *clock, *gpt9, *sys;

u8 polarity = 0;
u32 pincount = 0;
u32 interval = 10;
int teste = 0, teste2 = 1;
int gpio = 1;

//spinlock_t my_lock = SPIN_LOCK_UNLOCKED;

/* sysfs stuff */
static DEFINE_MUTEX(sysfs_lock);

static ssize_t onda_pincount_show(struct device *dev,
                struct device_attribute *attr, char *buf);

static ssize_t onda_interval_show(struct device *dev,
                struct device_attribute *attr, char *buf);
static ssize_t onda_interval_store(struct device *dev,
                struct device_attribute *attr, const char *buf, size_t size);

/*static ssize_t onda_pwm_show(struct device *dev,
                struct device_attribute *attr, char *buf);
static ssize_t onda_pwm_store(struct device *dev,
                struct device_attribute *attr, const char *buf, size_t size);*/

				
//void short_do_tasklet (unsigned long);
//DECLARE_TASKLET (short_tasklet, short_do_tasklet, 0);



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

	status = sprintf(buf, "%d\n", gpio_get_value(138));//interval);

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

/*void short_do_tasklet (unsigned long unused)
{
	gpio = gpio_get_value(138);
}*/

static irqreturn_t gpio_irq_handler_fall(int irq, void *data)
{
	//if (gpio_get_value(138) == 0) {
	if ((in32(gpio5 + OMAP2420_GPIO_DATAIN) & (1 << 10)) == 0) {
		unsigned int t;
		/* clear the pin 139*/
		out32(gpio5 + OMAP2420_GPIO_CLEARDATAOUT, (1 << 11));
		//gpio_set_value(139, 0);
		
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
		out32(gpio5 + OMAP2420_GPIO_SETDATAOUT, (1 << 11));
		//gpio_set_value(139, 1);
	}
	//gpio = (~gpio) & 1;
    //tasklet_schedule(&short_tasklet);

	return IRQ_HANDLED;
}

//static irq_handler_t irq_handler(int irq, void *dev_id, struct pt_regs *regs)
static irqreturn_t timer_irq_handler(int irq, void *data)
{
	if (polarity)
		//gpio_set_value(139, 0);
		out32(gpio5 + OMAP2420_GPIO_CLEARDATAOUT, (1 << 11));
	else
		//gpio_set_value(139, 1);
		out32(gpio5 + OMAP2420_GPIO_SETDATAOUT, (1 << 11));

	polarity = (~polarity) & 1;
	//out32(gpt3 + OMAP3530_GPT_TISR, 2);
	out32(gpt9 + OMAP3530_GPT_TISR, 2);
	
	return IRQ_HANDLED;
}

static int onda_init(void)
{
	u32 l;

	int status = class_register(&onda_drv);
	if(status < 0)
		printk("Registering Class Failed\n");

	gpio5 = ioremap(OMAP3530_GPIO5_BASE, OMAP3530_GPIO_SIZE);
	if (!gpio5) {
		printk(KERN_ERR "Cannot map ioport gpio5");
		return 0;
	}
	//gpt3 = ioport_map(OMAP3530_GPT3_BASE, OMAP3530_GPT_SIZE);
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

	//gpio5 = ioremap(OMAP3530_GPIO5_BASE, OMAP3530_GPIO_SIZE);

	
	printk(KERN_ALERT "ioport_map2\n");

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

	//status = request_irq(33, gpio_irq_handler, 0, "gpio_irq_handler", NULL);
	/*status = request_irq(33, gpio_irq_handler, IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, "gpio_irq_handler", NULL);
	if (status != 0) {
		printk(KERN_ALERT "Error in request_irq 33: %d\n", status);
		return 0;
	}*/

	printk(KERN_ALERT "request_irq\n");

	/* setting the PIN 139 to output */
	/* selecting mode 4 function - GPIO 139
	 * selecting pullup and mode 4 function - GPIO 138 */
#define SYS_CONF	((4 << 16) | ((1 << 8) | (1<<3) | 4))
#define SYS_MASK	~(0x10F010F)
	/*l = (in32(sys + 0x168) &  SYS_MASK) | SYS_CONF;
	//l = (in32(sys + 0x168) & ~(7<<16) ) | (4 << 16);
	//out32(sys + 0x168, ((1<<3 | 4) << 16) | (1<<3) | 4);
	out32(sys + 0x168, l); */
	//omap_mux_init_gpio(138, OMAP_PIN_INPUT_PULLUP);
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
	/*l = (in32(gpio5 + OMAP2420_GPIO_OE) & ~(1 << 11)) | 1 << 10;
	out32(gpio5 + OMAP2420_GPIO_OE, l);*/
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
	
	/* enabling interrupt on both levels on GPIO 139 */
	/*out32(gpio5 + OMAP2420_GPIO_RISINGDETECT, l << 10);
	out32(gpio5 + OMAP2420_GPIO_FALLINGDETECT, l << 10);
	out32(gpio5 + OMAP2420_GPIO_SETIRQENABLE1, l << 10);*/

	
	/* make sure timer has stop */
	//out32(gpt3 + OMAP3530_GPT_TCLR, 0);
	out32(gpt9 + OMAP3530_GPT_TCLR, 0);

	/* enabling the interrupt */
	out32(gpt9 + OMAP3530_GPT_TIER, 2); //comentar se PWM
	
	/* configuring PWM */
	out32(gpt9 + OMAP3530_GPT_TCLR, (1<<12) | (1<<10) | (1<<7)); //-- PWM

	status = request_irq(gpio_to_irq(138), gpio_irq_handler_fall, IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, "gpio_irq_handler_fall", NULL);//&teste);
	if (status != 0) {
		printk(KERN_ALERT "Error in request_irq fall 33: %d\n", status);
		return 0;
	}
	/*status = request_irq(gpio_to_irq(138), gpio_irq_handler_rise, IRQF_SHARED|IRQF_TRIGGER_HIGH, "gpio_irq_handler_rise", &teste2);
	if (status != 0) {
		printk(KERN_ALERT "Error in request_irq rise 33: %d\n", status);
		return 0;
	}*/
	//out32(gpio5 + OMAP2420_GPIO_SETDATAOUT, (1 << 11));

	printk(KERN_ALERT "Hello world\n");

	return 0;
}
static void onda_exit(void)
{
	iowrite32(0, gpt3 + OMAP3530_GPT_TCLR);
	iowrite32(0, gpt3 + OMAP3530_GPT_TIER);
	free_irq(39, NULL);
	//iowrite32((1 << 11), gpio5 + OMAP2420_GPIO_SETDATAOUT);
	//printk(KERN_ALERT "Timer when removing the kernel: %x\n", ioread32(gpt3 + OMAP3530_GPT_TCRR) & 0xff);
	//iounmap(gpio5);
	iounmap(gpt9);
	class_unregister(&onda_drv);
	printk(KERN_ALERT "Goodbye, cruel world\n");
}
module_init(onda_init);
module_exit(onda_exit);