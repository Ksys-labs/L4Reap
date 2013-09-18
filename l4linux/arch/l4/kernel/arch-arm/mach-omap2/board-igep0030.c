/*
 * Copyright (C) 2009 Integration Software and Electronic Engineering.
 *
 * Modified from mach-omap2/board-generic.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/i2c/twl.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>
#include <linux/spi/spi.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/mmc/host.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/flash.h>
#include <asm/mach/map.h>

#include <plat/board.h>
#include "common.h"
#include <video/omapdss.h>
#include <video/omap-panel-generic-dpi.h>
#include <video/omap-panel-tfp410.h>
#include <plat/gpmc.h>
#include <mach/hardware.h>
#include <plat/nand.h>
#include <plat/mcspi.h>
#include <plat/mux.h>
#include <plat/usb.h>
#include <plat/omap34xx.h>

#include "mux.h"
#include "sdram-micron-mt46h32m32lf-6.h"
#include "hsmmc.h"
#include "common-board-devices.h"
#include "powerdomain.h"
#include "clockdomain.h"
#include "clock3xxx.h"

#define GPIO_LED_D210_RED	16
#define GPIO_WIFI_NPD		138
#define GPIO_WIFI_NRESET	139
#define GPIO_BT_NRESET		137


#define IGEP2_SMSC911X_CS       5
#define IGEP2_SMSC911X_GPIO     176
#define IGEP2_GPIO_USBH_NRESET  24
#define IGEP2_GPIO_LED0_GREEN   26
#define IGEP2_GPIO_LED0_RED     27
#define IGEP2_GPIO_LED1_RED     28
#define IGEP2_GPIO_DVI_PUP      170

#define IGEP2_RB_GPIO_WIFI_NPD     94
#define IGEP2_RB_GPIO_WIFI_NRESET  95
#define IGEP2_RB_GPIO_BT_NRESET    137
#define IGEP2_RC_GPIO_WIFI_NPD     138
#define IGEP2_RC_GPIO_WIFI_NRESET  139
#define IGEP2_RC_GPIO_BT_NRESET    137

#define IGEP3_GPIO_LED0_GREEN	54
#define IGEP3_GPIO_LED0_RED	53
#define IGEP3_GPIO_LED1_RED	16
#define IGEP3_GPIO_USBH_NRESET  183

#define IGEP00X0_BUDDY_MAX_STRLEN	16
#define IGEP00X0_BUDDY_NONE		0x00
#define IGEP00X0_BUDDY_IGEP0022		0x01
#define IGEP00X0_BUDDY_BASE0010		0x02
#define IGEP00X0_BUDDY_ILMS0015		0x03

#define IGEP00X0_BUDDY_HWREV_A		(1 << 0)
#define IGEP00X0_BUDDY_HWREV_B		(1 << 1)

#define IGEP00X0_BUDDY_OPT_MODEM	(1 << 0)

#define TWL_IGEP00X0_REGULATOR_VMMC1	(1 << 0)
#define TWL_IGEP00X0_REGULATOR_VIO	(1 << 1)

#define IGEP00X0_SYSBOOT_MASK           0x1f
#define IGEP00X0_SYSBOOT_NAND           0x0f
#define IGEP00X0_SYSBOOT_ONENAND        0x10


#define IGEP_SYSBOOT_MASK           0x1f
#define IGEP_SYSBOOT_NAND           0x0f
#define IGEP_SYSBOOT_ONENAND        0x10

/*
 * IGEP2 Hardware Revision Table
 *
 *  --------------------------------------------------------------------------
 * | Id. | Hw Rev.            | HW0 (28) | WIFI_NPD | WIFI_NRESET | BT_NRESET |
 *  --------------------------------------------------------------------------
 * |  0  | B                  |   high   |  gpio94  |   gpio95    |     -     |
 * |  0  | B/C (B-compatible) |   high   |  gpio94  |   gpio95    |  gpio137  |
 * |  1  | C                  |   low    |  gpio138 |   gpio139   |  gpio137  |
 *  --------------------------------------------------------------------------
 */

#define IGEP2_BOARD_HWREV_B	0
#define IGEP2_BOARD_HWREV_C	1
#define IGEP3_BOARD_HWREV	2

#define IGEP3_RD_GPIO_LED_D440_GREEN	54
#define IGEP3_RD_GPIO_LED_D440_RED	53
#define IGEP3_RD_GPIO_USBH_NRESET	183
#define IGEP3_RE_GPIO_USBH_NRESET	54

/*
 * IGEP3 Hardware Revision
 *
 * Revision D is only assembled with OMAP35x
 * Revision E is only assembled with DM37xx
 *
 */

#define IGEP3_BOARD_HWREV_D	0xD
#define IGEP3_BOARD_HWREV_E	0xE

static u8 hwrev;

static void __init igep3_get_revision(void)
{
	if (cpu_is_omap3630()) {
		pr_info("IGEP: Hardware Rev. E\n");
		hwrev = IGEP3_BOARD_HWREV_E;
	} else {
		pr_info("IGEP: Hardware Rev. D\n");
		hwrev = IGEP3_BOARD_HWREV_D;
	}
}


#if defined(CONFIG_MTD_ONENAND_OMAP2) ||		\
	defined(CONFIG_MTD_ONENAND_OMAP2_MODULE) ||	\
	defined(CONFIG_MTD_NAND_OMAP2) ||		\
	defined(CONFIG_MTD_NAND_OMAP2_MODULE)

#define ONENAND_MAP             0x20000000

/* NAND04GR4E1A ( x2 Flash built-in COMBO POP MEMORY )
 * Since the device is equipped with two DataRAMs, and two-plane NAND
 * Flash memory array, these two component enables simultaneous program
 * of 4KiB. Plane1 has only even blocks such as block0, block2, block4
 * while Plane2 has only odd blocks such as block1, block3, block5.
 * So MTD regards it as 4KiB page size and 256KiB block size 64*(2*2048)
 */

static struct mtd_partition igep_flash_partitions[] = {
	{
		.name           = "X-Loader",
		.offset         = 0,
		.size           = 2 * (64*(2*2048))
	},
	{
		.name           = "U-Boot",
		.offset         = MTDPART_OFS_APPEND,
		.size           = 6 * (64*(2*2048)),
	},
	{
		.name           = "Environment",
		.offset         = MTDPART_OFS_APPEND,
		.size           = 2 * (64*(2*2048)),
	},
	{
		.name           = "Kernel",
		.offset         = MTDPART_OFS_APPEND,
		.size           = 12 * (64*(2*2048)),
	},
	{
		.name           = "File System",
		.offset         = MTDPART_OFS_APPEND,
		.size           = MTDPART_SIZ_FULL,
	},
};

static inline u32 igep_get_sysboot_value(void)
{
	return omap_ctrl_readl(OMAP343X_CONTROL_STATUS) & IGEP_SYSBOOT_MASK;
}

static void __init igep_flash_init(void)
{
	u32 mux;
	mux = igep_get_sysboot_value();

	if (mux == IGEP_SYSBOOT_NAND) {
		pr_info("IGEP: initializing NAND memory device\n");
		board_nand_init(igep_flash_partitions,
				ARRAY_SIZE(igep_flash_partitions),
				0, NAND_BUSWIDTH_16);
	} else if (mux == IGEP_SYSBOOT_ONENAND) {
		pr_info("IGEP: initializing OneNAND memory device\n");
		board_onenand_init(igep_flash_partitions,
				   ARRAY_SIZE(igep_flash_partitions), 0);
	} else {
		pr_err("IGEP: Flash: unsupported sysboot sequence found\n");
	}
}

#else
static void __init igep_flash_init(void) {}
#endif

#if defined(CONFIG_SMSC911X) || defined(CONFIG_SMSC911X_MODULE)

#include <linux/smsc911x.h>
#include <plat/gpmc-smsc911x.h>

static struct omap_smsc911x_platform_data smsc911x_cfg = {
	.cs             = IGEP2_SMSC911X_CS,
	.gpio_irq       = IGEP2_SMSC911X_GPIO,
	.gpio_reset     = -EINVAL,
	.flags		= SMSC911X_USE_32BIT | SMSC911X_SAVE_MAC_ADDRESS,
};

static inline void __init igep2_init_smsc911x(void)
{
	gpmc_smsc911x_init(&smsc911x_cfg);
}

#else
static inline void __init igep2_init_smsc911x(void) { }
#endif

static struct regulator_consumer_supply igep_vmmc1_supply[] = {
	REGULATOR_SUPPLY("vmmc", "omap_hsmmc.0"),
};

/* VMMC1 for OMAP VDD_MMC1 (i/o) and MMC1 card */
static struct regulator_init_data igep_vmmc1 = {
	.constraints = {
		.min_uV			= 1850000,
		.max_uV			= 3150000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = ARRAY_SIZE(igep_vmmc1_supply),
	.consumer_supplies      = igep_vmmc1_supply,
};

static struct regulator_consumer_supply igep_vio_supply[] = {
	REGULATOR_SUPPLY("vmmc_aux", "omap_hsmmc.1"),
};

static struct regulator_init_data igep_vio = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.apply_uV		= 1,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = ARRAY_SIZE(igep_vio_supply),
	.consumer_supplies      = igep_vio_supply,
};

static struct regulator_consumer_supply igep_vmmc2_supply[] = {
	REGULATOR_SUPPLY("vmmc", "omap_hsmmc.1"),
};

static struct regulator_init_data igep_vmmc2 = {
	.constraints		= {
		.valid_modes_mask	= REGULATOR_MODE_NORMAL,
		.always_on		= 1,
	},
	.num_consumer_supplies	= ARRAY_SIZE(igep_vmmc2_supply),
	.consumer_supplies	= igep_vmmc2_supply,
};

static struct fixed_voltage_config igep_vwlan = {
	.supply_name		= "vwlan",
	.microvolts		= 3300000,
	.gpio			= -EINVAL,
	.enabled_at_boot	= 1,
	.init_data		= &igep_vmmc2,
};

static struct platform_device igep_vwlan_device = {
	.name		= "reg-fixed-voltage",
	.id		= 0,
	.dev = {
		.platform_data	= &igep_vwlan,
	},
};

static struct omap2_hsmmc_info mmc[] = {
	{
		.mmc		= 1,
		.caps		= MMC_CAP_4_BIT_DATA,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= -EINVAL,
	},
	{
		.mmc		= 2,
		.caps		= MMC_CAP_4_BIT_DATA,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= -EINVAL,
		.transceiver	= true,
		.ocr_mask	= 0x00100000,	/* 3.3V */
	},
	{}	/* Terminator */
};



#if defined(CONFIG_LEDS_GPIO) || defined(CONFIG_LEDS_GPIO_MODULE)
#include <linux/leds.h>

static struct gpio_led igep_gpio_leds[] = {
	[0] = {
		.name			= "gpio-led:red:d0",
		.default_trigger	= "default-off"
	},
	[1] = {
		.name			= "gpio-led:green:d0",
		.default_trigger	= "default-off",
	},
	[2] = {
		.name			= "gpio-led:red:d1",
		.default_trigger	= "default-off",
	},
	[3] = {
		.name			= "gpio-led:green:d1",
		.default_trigger	= "heartbeat",
		.gpio			= -EINVAL, /* gets replaced */
		.active_low		= 1,
	},
};

static struct gpio_led_platform_data igep_led_pdata = {
	.leds           = igep_gpio_leds,
	.num_leds       = ARRAY_SIZE(igep_gpio_leds),
};

static struct platform_device igep_led_device = {
	 .name   = "leds-gpio",
	 .id     = -1,
	 .dev    = {
		 .platform_data  =  &igep_led_pdata,
	},
};

static void __init igep_leds_init(void)
{
	if (machine_is_igep0020()) {
		igep_gpio_leds[0].gpio = IGEP2_GPIO_LED0_RED;
		igep_gpio_leds[1].gpio = IGEP2_GPIO_LED0_GREEN;
		igep_gpio_leds[2].gpio = IGEP2_GPIO_LED1_RED;
	} else {
		igep_gpio_leds[0].gpio = IGEP3_GPIO_LED0_RED;
		igep_gpio_leds[1].gpio = IGEP3_GPIO_LED0_GREEN;
		igep_gpio_leds[2].gpio = IGEP3_GPIO_LED1_RED;
	}

	platform_device_register(&igep_led_device);
}

#else
static struct gpio igep_gpio_leds[] __initdata = {
	{ -EINVAL,	GPIOF_OUT_INIT_LOW, "gpio-led:red:d0"   },
	{ -EINVAL,	GPIOF_OUT_INIT_LOW, "gpio-led:green:d0" },
	{ -EINVAL,	GPIOF_OUT_INIT_LOW, "gpio-led:red:d1"   },
};

static inline void igep_leds_init(void)
{
	int i;

	if (machine_is_igep0020()) {
		igep_gpio_leds[0].gpio = IGEP2_GPIO_LED0_RED;
		igep_gpio_leds[1].gpio = IGEP2_GPIO_LED0_GREEN;
		igep_gpio_leds[2].gpio = IGEP2_GPIO_LED1_RED;
	} else {
		igep_gpio_leds[0].gpio = IGEP3_GPIO_LED0_RED;
		igep_gpio_leds[1].gpio = IGEP3_GPIO_LED0_GREEN;
		igep_gpio_leds[2].gpio = IGEP3_GPIO_LED1_RED;
	}

	if (gpio_request_array(igep_gpio_leds, ARRAY_SIZE(igep_gpio_leds))) {
		pr_warning("IGEP v2: Could not obtain leds gpios\n");
		return;
	}

	for (i = 0; i < ARRAY_SIZE(igep_gpio_leds); i++)
		gpio_export(igep_gpio_leds[i].gpio, 0);
}
#endif

static struct gpio igep2_twl_gpios[] = {
	{ -EINVAL, GPIOF_IN,		"GPIO_EHCI_NOC"  },
	{ -EINVAL, GPIOF_OUT_INIT_LOW,	"GPIO_USBH_CPEN" },
};

static int igep_twl_gpio_setup(struct device *dev,
		unsigned gpio, unsigned ngpio)
{
	int ret;

	/* gpio + 0 is "mmc0_cd" (input/IRQ) */
	mmc[0].gpio_cd = gpio + 0;
	omap_hsmmc_late_init(mmc);

	/* TWL4030_GPIO_MAX + 1 == ledB (out, active low LED) */
#if !defined(CONFIG_LEDS_GPIO) && !defined(CONFIG_LEDS_GPIO_MODULE)
	ret = gpio_request_one(gpio + TWL4030_GPIO_MAX + 1, GPIOF_OUT_INIT_HIGH,
			       "gpio-led:green:d1");
	if (ret == 0)
		gpio_export(gpio + TWL4030_GPIO_MAX + 1, 0);
	else
		pr_warning("IGEP: Could not obtain gpio GPIO_LED1_GREEN\n");
#else
	igep_gpio_leds[3].gpio = gpio + TWL4030_GPIO_MAX + 1;
#endif

	if (machine_is_igep0030())
		return 0;

	/*
	 * REVISIT: need ehci-omap hooks for external VBUS
	 * power switch and overcurrent detect
	 */
	igep2_twl_gpios[0].gpio = gpio + 1;

	/* TWL4030_GPIO_MAX + 0 == ledA, GPIO_USBH_CPEN (out, active low) */
	igep2_twl_gpios[1].gpio = gpio + TWL4030_GPIO_MAX;

	ret = gpio_request_array(igep2_twl_gpios, ARRAY_SIZE(igep2_twl_gpios));
	if (ret < 0)
		pr_err("IGEP2: Could not obtain gpio for USBH_CPEN");

	return 0;
};

static struct twl4030_gpio_platform_data igep_twl4030_gpio_pdata = {
	.gpio_base	= OMAP_MAX_GPIO_LINES,
	.irq_base	= TWL4030_GPIO_IRQ_BASE,
	.irq_end	= TWL4030_GPIO_IRQ_END,
	.use_leds	= true,
	.setup		= igep_twl_gpio_setup,
};

static struct tfp410_platform_data dvi_panel = {
	.i2c_bus_num		= 3,
	.power_down_gpio	= IGEP2_GPIO_DVI_PUP,
};

static struct omap_dss_device igep2_dvi_device = {
	.type			= OMAP_DISPLAY_TYPE_DPI,
	.name			= "dvi",
	.driver_name		= "tfp410",
	.data			= &dvi_panel,
	.phy.dpi.data_lines	= 24,
};

static struct omap_dss_device *igep2_dss_devices[] = {
	&igep2_dvi_device
};

static struct omap_dss_board_info igep2_dss_data = {
	.num_devices	= ARRAY_SIZE(igep2_dss_devices),
	.devices	= igep2_dss_devices,
	.default_device	= &igep2_dvi_device,
};

static struct platform_device *igep_devices[] __initdata = {
	&igep_vwlan_device,
};

static int igep2_keymap[] = {
	KEY(0, 0, KEY_LEFT),
	KEY(0, 1, KEY_RIGHT),
	KEY(0, 2, KEY_A),
	KEY(0, 3, KEY_B),
	KEY(1, 0, KEY_DOWN),
	KEY(1, 1, KEY_UP),
	KEY(1, 2, KEY_E),
	KEY(1, 3, KEY_F),
	KEY(2, 0, KEY_ENTER),
	KEY(2, 1, KEY_I),
	KEY(2, 2, KEY_J),
	KEY(2, 3, KEY_K),
	KEY(3, 0, KEY_M),
	KEY(3, 1, KEY_N),
	KEY(3, 2, KEY_O),
	KEY(3, 3, KEY_P)
};

static struct matrix_keymap_data igep2_keymap_data = {
	.keymap			= igep2_keymap,
	.keymap_size		= ARRAY_SIZE(igep2_keymap),
};

static struct twl4030_keypad_data igep2_keypad_pdata = {
	.keymap_data	= &igep2_keymap_data,
	.rows		= 4,
	.cols		= 4,
	.rep		= 1,
};

static struct twl4030_platform_data igep_twldata = {
	/* platform_data for children goes here */
	.gpio		= &igep_twl4030_gpio_pdata,
	.vmmc1          = &igep_vmmc1,
	.vio		= &igep_vio,
};

static struct i2c_board_info __initdata igep2_i2c3_boardinfo[] = {
	{
		I2C_BOARD_INFO("eeprom", 0x50),
	},
};

static void __init igep_i2c_init(void)
{
	int ret;

	omap3_pmic_get_config(&igep_twldata, TWL_COMMON_PDATA_USB,
			      TWL_COMMON_REGULATOR_VPLL2);
	igep_twldata.vpll2->constraints.apply_uV = true;
	igep_twldata.vpll2->constraints.name = "VDVI";

	if (machine_is_igep0020()) {
		/*
		 * Bus 3 is attached to the DVI port where devices like the
		 * pico DLP projector don't work reliably with 400kHz
		 */
		ret = omap_register_i2c_bus(3, 100, igep2_i2c3_boardinfo,
					    ARRAY_SIZE(igep2_i2c3_boardinfo));
		if (ret)
			pr_warning("IGEP2: Could not register I2C3 bus (%d)\n", ret);

		igep_twldata.keypad	= &igep2_keypad_pdata;
		/* Get common pmic data */
		omap3_pmic_get_config(&igep_twldata, TWL_COMMON_PDATA_AUDIO, 0);
	}

	omap3_pmic_init("twl4030", &igep_twldata);
}

static const struct usbhs_omap_board_data igep2_usbhs_bdata __initconst = {
	.port_mode[0] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[1] = OMAP_USBHS_PORT_MODE_UNUSED,
	.port_mode[2] = OMAP_USBHS_PORT_MODE_UNUSED,

	.phy_reset = true,
	.reset_gpio_port[0] = IGEP2_GPIO_USBH_NRESET,
	.reset_gpio_port[1] = -EINVAL,
	.reset_gpio_port[2] = -EINVAL,
};

static const struct usbhs_omap_board_data igep3_usbhs_bdata __initconst = {
	.port_mode[0] = OMAP_USBHS_PORT_MODE_UNUSED,
	.port_mode[1] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[2] = OMAP_USBHS_PORT_MODE_UNUSED,

	.phy_reset = true,
	.reset_gpio_port[0] = -EINVAL,
	.reset_gpio_port[1] = IGEP3_GPIO_USBH_NRESET,
	.reset_gpio_port[2] = -EINVAL,
};

#ifdef CONFIG_OMAP_MUX
static struct omap_board_mux board_mux[] __initdata = {
	{ .reg_offset = OMAP_MUX_TERMINATOR },
};
#endif

#if defined(CONFIG_LIBERTAS_SDIO) || defined(CONFIG_LIBERTAS_SDIO_MODULE) || \
	defined(CONFIG_LIBERTAS_THINFIRM) || defined(CONFIG_LIBERTAS_THINFIRM_SDIO)
void __init igep00x0_wifi_bt_init(int npd, int wifi_nreset, int bt_nreset,
		int bt_enable)
{
	omap_mux_init_gpio(npd, OMAP_PIN_OUTPUT);
	omap_mux_init_gpio(wifi_nreset, OMAP_PIN_OUTPUT);
	omap_mux_init_gpio(bt_nreset, OMAP_PIN_OUTPUT);

	/* Set GPIO's for  W-LAN + Bluetooth combo module */
	if ((gpio_request(npd, "WIFI NPD") == 0) &&
	    (gpio_direction_output(npd, 1) == 0))
		gpio_export(npd, 0);
	else
		pr_warning("IGEP: Could not obtain gpio WIFI NPD\n");

	if ((gpio_request(wifi_nreset, "WIFI NRESET") == 0) &&
	    (gpio_direction_output(wifi_nreset, 1) == 0)) {
		gpio_export(wifi_nreset, 0);
		gpio_set_value(wifi_nreset, 0);
		udelay(10);
		gpio_set_value(wifi_nreset, 1);
	} else
		pr_warning("IGEP: Could not obtain gpio WIFI NRESET\n");

	if ((gpio_request(bt_nreset, "BT NRESET") == 0) &&
	    (gpio_direction_output(bt_nreset, bt_enable) == 0))
		gpio_export(bt_nreset, 0);
	else
		pr_warning("IGEP: Could not obtain gpio BT NRESET\n");
}

#else
void __init igep00x0_wifi_bt_init(int npd, int wifi_nreset, int bt_nreset,
			int bt_enable) { }
#endif

static int twl4030_gpio_setup(struct device *dev,
		unsigned gpio, unsigned ngpio)
{
#if 0
	/* gpio + 0 is "mmc0_cd" (input/IRQ) */
	mmc[0].gpio_cd = gpio + 0;
	omap2_hsmmc_init(mmc);

	/* REVISIT: need ehci-omap hooks for external VBUS
	 * power switch and overcurrent detect
	 */
	gpio_request(gpio + 1, "EHCI NOC");
	gpio_direction_input(gpio + 1);

	if (hwrev == IGEP3_BOARD_HWREV_D) {
		gpio_led_data[0].gpio = IGEP3_RD_GPIO_LED_D440_RED;
		gpio_led_data[1].gpio = IGEP3_RD_GPIO_LED_D440_GREEN;
	} else {
		/* Hardware Rev. E */
		/* TWL4030_GPIO_MAX + 0 == ledA (out, active low LED) */
		gpio_led_data[0].gpio = gpio + TWL4030_GPIO_MAX + 0;
		/* gpio + 13 == ledsync (out, active low LED) */
		gpio_led_data[1].gpio = gpio + 13;
	}

	/* TWL4030_GPIO_MAX + 1 == ledB (out, active low LED) */
	gpio_led_data[3].gpio = gpio + TWL4030_GPIO_MAX + 1;

	/* Register led devices */
	platform_device_register(&gpio_led_device);
#endif
	return 0;
};


static struct twl4030_gpio_platform_data twl4030_gpio_pdata = {
	.gpio_base	= OMAP_MAX_GPIO_LINES,
	.irq_base	= TWL4030_GPIO_IRQ_BASE,
	.irq_end	= TWL4030_GPIO_IRQ_END,
	.use_leds	= true,
	.setup		= twl4030_gpio_setup,
};

static struct twl4030_platform_data twl4030_pdata = {
	/* platform_data for children goes here */
	.gpio		= &twl4030_gpio_pdata,
};



/* TWL4030 power init scripts */
static struct twl4030_ins wrst_seq[] __initdata = {
/*
 * Reset twl4030.
 * Reset VDD1 regulator.
 * Reset VDD2 regulator.
 * Reset VPLL1 regulator.
 * Enable sysclk output.
 * Reenable twl4030.
 */
        {MSG_SINGULAR(DEV_GRP_NULL, 0x1b, RES_STATE_OFF), 2},
        {MSG_SINGULAR(DEV_GRP_P1, 0xf, RES_STATE_WRST), 15},
        {MSG_SINGULAR(DEV_GRP_P1, 0x10, RES_STATE_WRST), 15},
        {MSG_SINGULAR(DEV_GRP_P1, 0x7, RES_STATE_WRST), 0x60},
        {MSG_SINGULAR(DEV_GRP_P1, 0x19, RES_STATE_ACTIVE), 2},
        {MSG_SINGULAR(DEV_GRP_NULL, 0x1b, RES_STATE_ACTIVE), 2},
};

static struct twl4030_script wrst_script __initdata = {
        .script = wrst_seq,
        .size   = ARRAY_SIZE(wrst_seq),
        .flags  = TWL4030_WRST_SCRIPT,
};

static struct twl4030_script *twl4030_scripts[] __initdata = {
        &wrst_script,
};

static struct twl4030_resconfig twl4030_rconfig[] = {
        { .resource = RES_HFCLKOUT, .devgroup = DEV_GRP_P3, .type = -1,
                .type2 = -1 },
        { .resource = RES_VDD1, .devgroup = DEV_GRP_P1, .type = -1,
                .type2 = -1 },
        { .resource = RES_VDD2, .devgroup = DEV_GRP_P1, .type = -1,
                .type2 = -1 },
        { 0, 0},
};


static struct twl4030_power_data igep00x0_twl4030_power_data = {
        .scripts        = twl4030_scripts,
        .num            = ARRAY_SIZE(twl4030_scripts),
        .resource_config = twl4030_rconfig,
};

static struct regulator_consumer_supply vmmc1_supply =
	REGULATOR_SUPPLY("vmmc", "mmci-omap-hs.0");

static struct regulator_consumer_supply vio_supply =
	REGULATOR_SUPPLY("hsusb0", "ehci-omap.0");

static struct regulator_consumer_supply vdd33_supplies[] = {
	REGULATOR_SUPPLY("vmmc", "mmci-omap-hs.1"),
};

/* VMMC1 for OMAP VDD_MMC1 (i/o) and MMC1 card */
struct regulator_init_data igep00x0_vmmc1_idata = {
	.constraints = {
		.min_uV			= 1850000,
		.max_uV			= 3150000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &vmmc1_supply,
};

struct regulator_init_data igep00x0_vio_idata = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 1850000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &vio_supply,
};

static struct regulator_init_data vdd33_data = {
	.constraints		= {
		.valid_modes_mask	= REGULATOR_MODE_NORMAL,
		.always_on		= 1,
	},
	.num_consumer_supplies	= ARRAY_SIZE(vdd33_supplies),
	.consumer_supplies	= vdd33_supplies,
};

static struct fixed_voltage_config vdd33_voltage_config = {
	.supply_name		= "VDD 3V3",
	.microvolts		= 3300000,
	.gpio			= -EINVAL,
	.enabled_at_boot	= 1,
	.init_data		= &vdd33_data,
};

struct platform_device igep00x0_vdd33_device = {
	.name		= "reg-fixed-voltage",
	.id		= 0,
	.dev = {
		.platform_data	= &vdd33_voltage_config,
	},
};

static struct regulator_consumer_supply dummy_supplies[] = {
	REGULATOR_SUPPLY("vddvario", "smsc911x.0"),
	REGULATOR_SUPPLY("vdd33a", "smsc911x.0"),
};

void __init igep00x0_pmic_get_config(struct twl4030_platform_data *pmic_data,
				  u32 pdata_flags, u32 regulators_flags)
{
	if (!pmic_data->irq_base)
		pmic_data->irq_base = TWL4030_IRQ_BASE;
	if (!pmic_data->irq_end)
		pmic_data->irq_end = TWL4030_IRQ_END;

	/* Common platform data configurations */

	/* Common regulator configurations */
	if (regulators_flags & TWL_IGEP00X0_REGULATOR_VMMC1 && !pmic_data->vmmc1)
		pmic_data->vmmc1 = &igep00x0_vmmc1_idata;

	if (regulators_flags &  TWL_IGEP00X0_REGULATOR_VIO && !pmic_data->vio)
		pmic_data->vio = &igep00x0_vio_idata;

	/* add generic power scripts */
	if (!pmic_data->power)
		pmic_data->power = &igep00x0_twl4030_power_data;
}


void  igep_init(void)
{
	regulator_register_fixed(1, dummy_supplies, ARRAY_SIZE(dummy_supplies));
	omap3_mux_init(board_mux, OMAP_PACKAGE_CBB);

	/* Get IGEP3 hardware revision */
	igep3_get_revision();

	omap_hsmmc_init(mmc);

	/* Register I2C busses and drivers */
	igep_i2c_init();
	platform_add_devices(igep_devices, ARRAY_SIZE(igep_devices));
#if 0
	omap_serial_init();
	omap_sdrc_init(m65kxxxxam_sdrc_params,
				  m65kxxxxam_sdrc_params);
#endif
	usb_musb_init(NULL);
#if 0
	igep_flash_init();
#endif
	igep_leds_init();

	/* Add twl4030 common data */
	omap3_pmic_get_config(&twl4030_pdata, TWL_COMMON_PDATA_USB |
			TWL_COMMON_PDATA_AUDIO | TWL_COMMON_PDATA_MADC,
			TWL_COMMON_REGULATOR_VPLL2);

	igep00x0_pmic_get_config(&twl4030_pdata, 0,
			TWL_IGEP00X0_REGULATOR_VMMC1 |
			TWL_IGEP00X0_REGULATOR_VIO);

	omap_pmic_init(1, 2600, "twl4030", INT_34XX_SYS_NIRQ, &twl4030_pdata);


	platform_device_register(&igep00x0_vdd33_device);
	/*
	 * WLAN-BT combo module from MuRata which has a Marvell WLAN
	 * (88W8686) + CSR Bluetooth chipset. Uses SDIO interface.
	 */
	igep00x0_wifi_bt_init(GPIO_WIFI_NPD, GPIO_WIFI_NRESET,
			GPIO_BT_NRESET, 0);

	if (machine_is_igep0020()) {
		omap_display_init(&igep2_dss_data);
		igep2_init_smsc911x();
		usbhs_init(&igep2_usbhs_bdata);
	} else {
		usbhs_init(&igep3_usbhs_bdata);
	}
}

extern int omap_hwmod_setup_all(void);
extern int omap2_common_pm_init(void);

static struct omap_globals omap3_globals = {
	.class	= OMAP343X_CLASS,
	.tap	= OMAP2_L4_IO_ADDRESS(0x4830A000),
	.sdrc	= OMAP343X_SDRC_BASE,
	.sms	= OMAP343X_SMS_BASE,
	.ctrl	= OMAP343X_CTRL_BASE,
	.prm	= OMAP3430_PRM_BASE,
	.cm	= OMAP3430_CM_BASE,
};

static void omap2_set_globals_3xxx_aux(void)
{
	omap2_set_globals_control(&omap3_globals);
	omap2_set_globals_prcm(&omap3_globals);
}

static 	void omap2_init_common_infrastructure_aux(void)
{
	if (cpu_is_omap34xx()) {
		omap3xxx_voltagedomains_init();
		omap3xxx_powerdomains_init();
		omap3xxx_clockdomains_init();
		omap3xxx_hwmod_init();
	} else {
		pr_err("Could not init hwmod data - unknown SoC\n");
	}

	/* ...... */

	if (cpu_is_omap34xx())
		omap3xxx_clk_init();
	else
		pr_err("Could not init clock framework - unknown SoC\n");
}

void igep_init_early(void)
{
	omap2_set_globals_3xxx_aux();
	omap3_init_irq();
	extern void omap2_gpio_init(void);
	omap2_gpio_init();
	omap2_init_common_infrastructure_aux();
	omap_hwmod_setup_all();
	omap2_gpio_init();
	omap2_common_pm_init();
}

#if 0
MACHINE_START(IGEP0020, "IGEP v2 board")
	.atag_offset	= 0x100,
	.reserve	= omap_reserve,
	.map_io		= omap3_map_io,
	.init_early	= omap35xx_init_early,
	.init_irq	= omap3_init_irq,
	.handle_irq	= omap3_intc_handle_irq,
	.init_machine	= igep_init,
	.init_late	= omap35xx_init_late,
	.timer		= &omap3_timer,
	.restart	= omap_prcm_restart,
MACHINE_END

MACHINE_START(IGEP0030, "IGEP OMAP3 module")
	.atag_offset	= 0x100,
	.reserve	= omap_reserve,
	.map_io		= omap3_map_io,
	.init_early	= omap35xx_init_early,
	.init_irq	= omap3_init_irq,
	.handle_irq	= omap3_intc_handle_irq,
	.init_machine	= igep_init,
	.init_late	= omap35xx_init_late,
	.timer		= &omap3_timer,
	.restart	= omap_prcm_restart,
MACHINE_END

#endif
