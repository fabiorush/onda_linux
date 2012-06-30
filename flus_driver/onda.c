#include <linux/init.h>
#include <linux/module.h>
//#include <linux/kernel.h>
//#include <linux/errno.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/delay.h>

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

MODULE_LICENSE("Dual BSD/GPL");

void *gpio5, *gpt3, *clock;
u8 b = 0;

//spinlock_t my_lock = SPIN_LOCK_UNLOCKED;

//static irq_handler_t irq_handler(int irq, void *dev_id, struct pt_regs *regs)
static irqreturn_t irq_handler(int irq, void *data)
{
	if (b)
		iowrite32((1 << 11), gpio5 + OMAP2420_GPIO_CLEARDATAOUT);
	else
		iowrite32((1 << 11), gpio5 + OMAP2420_GPIO_SETDATAOUT);

	b = (~b) & 1;
	iowrite32(2, gpt3 + OMAP3530_GPT_TISR);
	
	return IRQ_HANDLED;
}

static int onda_init(void)
{
	u32 l;
	gpio5 = ioport_map(OMAP3530_GPIO5_BASE, OMAP3530_GPIO_SIZE);
	//gpio5 = ioremap(OMAP3530_GPIO5_BASE, OMAP3530_GPIO_SIZE);
	gpt3 = ioport_map(OMAP3530_GPT3_BASE, OMAP3530_GPT_SIZE);
	clock = ioport_map(0x48005000, 8192);
	
	/* enabling clocks */
	l = ioread32(clock) | (1<<16) | (1<<4);
	iowrite32(l, clock);

	l = ioread32(clock + 0x10) | (1<<16) | (1<<4);
	iowrite32(l, clock + 0x10);

	request_irq(39,   /* The number of the keyboard IRQ on PCs */ 
		irq_handler, /* our handler */
		0, 
		"onda_irq_handler", NULL);

	/* setting the PIN 139 to output */
	l = ioread32(gpio5 + OMAP2420_GPIO_OE)  & ~(1 << 11);
	iowrite32(l, gpio5 + OMAP2420_GPIO_OE);

	/* setting the PIN 139 to output */
//	out32(gpio5 + OMAP2420_GPIO_SETDATAOUT, (1 << 11));
	local_irq_disable();
	iowrite32((1 << 11), gpio5 + OMAP2420_GPIO_CLEARDATAOUT);
	
	//for (l=5000;l;l--);
	//ndelay(300);
	iowrite32((1 << 11), gpio5 + OMAP2420_GPIO_SETDATAOUT);
	//ndelay(300);
	iowrite32((1 << 11), gpio5 + OMAP2420_GPIO_CLEARDATAOUT);
	//ndelay(300);
	iowrite32((1 << 11), gpio5 + OMAP2420_GPIO_SETDATAOUT);
	//ndelay(300);
	iowrite32((1 << 11), gpio5 + OMAP2420_GPIO_CLEARDATAOUT);
	//ndelay(300);
	iowrite32((1 << 11), gpio5 + OMAP2420_GPIO_SETDATAOUT);
	//ndelay(300);
	iowrite32((1 << 11), gpio5 + OMAP2420_GPIO_CLEARDATAOUT);
	//ndelay(300);
	iowrite32((1 << 11), gpio5 + OMAP2420_GPIO_SETDATAOUT);
	//ndelay(300);
	iowrite32((1 << 11), gpio5 + OMAP2420_GPIO_CLEARDATAOUT);
	//ndelay(300);
	iowrite32((1 << 11), gpio5 + OMAP2420_GPIO_SETDATAOUT);
	//ndelay(300);
	iowrite32((1 << 11), gpio5 + OMAP2420_GPIO_CLEARDATAOUT);
	local_irq_enable();

	return 0;
	/* stopping timer */
	iowrite32(0, gpt3 + OMAP3530_GPT_TCLR);

	//sleep(1);
	/* setting the initial timer counter value
	 * cada tick é 80ns */
	iowrite32(0xffffff80, gpt3 + OMAP3530_GPT_TLDR);
	iowrite32(0xffffff80, gpt3 + OMAP3530_GPT_TCRR);

	/* enabling the interrupt */
	iowrite32(2, gpt3 + OMAP3530_GPT_TIER);

	/* starting timer */
	iowrite32(3, gpt3 + OMAP3530_GPT_TCLR);

	printk(KERN_ALERT "Hello world\n");
	printk(KERN_ALERT "CM_FCLKEN_PER: %x\n", ioread32(clock));
	printk(KERN_ALERT "CM_ICLKEN_PER: %x\n", ioread32(clock + 0x10));
	//printk(KERN_ALERT "Timer when loading the kernel: %x\n", ioread32(gpt3 + OMAP3530_GPT_TCRR) & 0xff);
	return 0;
}
static void onda_exit(void)
{
	iowrite32(0, gpt3 + OMAP3530_GPT_TCLR);
	iowrite32(0, gpt3 + OMAP3530_GPT_TIER);
	free_irq(39, NULL);
	iowrite32((1 << 11), gpio5 + OMAP2420_GPIO_SETDATAOUT);
	//printk(KERN_ALERT "Timer when removing the kernel: %x\n", ioread32(gpt3 + OMAP3530_GPT_TCRR) & 0xff);
	iounmap(gpio5);
	iounmap(gpt3);
	printk(KERN_ALERT "Goodbye, cruel world\n");
}
module_init(onda_init);
module_exit(onda_exit);
