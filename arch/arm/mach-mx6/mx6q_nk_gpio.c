//===============just for NK9014A=============================
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/pm.h>

#include <linux/init.h>

#include <linux/nodemask.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/fsl_devices.h>

#include <linux/ata.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/regulator/consumer.h>
#include <linux/pmic_external.h>
#include <linux/pmic_status.h>
#include <linux/ipu.h>

#include <linux/gpio.h>
#include <linux/ion.h>
#include <linux/etherdevice.h>
#include <linux/power/sabresd_battery.h>
#include <linux/regulator/anatop-regulator.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>

#include <mach/common.h>
#include <mach/hardware.h>
#include <mach/mxc_dvfs.h>
#include <mach/memory.h>
#include <mach/iomux-mx6q.h>

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>


#include "devices-imx6q.h"

#include "cpu_op-mx6.h"


static iomux_v3_cfg_t mx6q_nk_gpio_pads[] = {
    MX6Q_PAD_CSI0_DAT8__GPIO_5_26,
    MX6Q_PAD_CSI0_DAT9__GPIO_5_27,
    MX6Q_PAD_CSI0_DAT10__GPIO_5_28,
    MX6Q_PAD_KEY_ROW2__GPIO_4_11,

};

#define NK_GPIO(name_gpio, no_gpio, dir_gpio, value_gpio)   \
{                                          \
    .name           = name_gpio,           \
    .gpio           = no_gpio,             \
    .direction      = dir_gpio,            \
    .value          = value_gpio,          \
    .defualt        = value_gpio,          \
}

struct nk_gpio
{
    const char* name;
    int gpio;
    int direction;
    int value;
    int defualt;
};

struct nk_gpio nk9013a_gpios[] =
{
    NK_GPIO("gpio_1", IMX_GPIO_NR(5, 26), 1, 0),
    NK_GPIO("gpio_2", IMX_GPIO_NR(5, 27), 1, 0),
    NK_GPIO("gpio_3", IMX_GPIO_NR(5, 28), 1, 0),
    NK_GPIO("gpio_4", IMX_GPIO_NR(4, 11), 0, 1),
};
struct nk_gpio_platform_data
{
    const struct nk_gpio* gpios;
    int num_gpios;
};

struct nk_gpio_platform_data nk9013a_gpio_platform_data =
{
    .gpios = nk9013a_gpios,
    .num_gpios = ARRAY_SIZE(nk9013a_gpios),
};

static struct platform_device nk9013a_gpio_device =
{
    .name = "nk-gpio",
    .id = -1,
    .dev = 
    {
        .platform_data = &nk9013a_gpio_platform_data,
    },
};

void  __init imx6q_add_device_nk_gpios(void)
{
    mxc_iomux_v3_setup_multiple_pads(mx6q_nk_gpio_pads,
        ARRAY_SIZE(mx6q_nk_gpio_pads));

    platform_device_register(&nk9013a_gpio_device);
    return;
}
//=============end of nk-gpios============================
