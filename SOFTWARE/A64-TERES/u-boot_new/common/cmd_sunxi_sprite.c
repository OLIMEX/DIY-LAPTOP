
#include <common.h>
#include <config.h>
#include <command.h>

DECLARE_GLOBAL_DATA_PTR;


extern int sunxi_usb_dev_register(uint dev_name);
extern void sunxi_usb_main_loop(int mode);
extern int sunxi_card_sprite_main(int workmode, char *name);
extern int sprite_form_sysrecovery(void);

extern int sprite_led_init(void);
extern int sprite_led_exit(int status);

int do_sprite_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;
	printf("work mode=0x%x\n", uboot_spare_head.boot_data.work_mode);
	if(uboot_spare_head.boot_data.work_mode == WORK_MODE_USB_PRODUCT)
	{
		printf("run usb efex\n");
		if(sunxi_usb_dev_register(2))
		{
			printf("sunxi usb test: invalid usb device\n");
		}
		sunxi_usb_main_loop(2500);
	}
	else if(uboot_spare_head.boot_data.work_mode == WORK_MODE_CARD_PRODUCT)
	{
		printf("run card sprite\n");
		sprite_led_init();
		ret = sunxi_card_sprite_main(0, NULL);
		sprite_led_exit(ret);
		return ret;
	}
	else if(uboot_spare_head.boot_data.work_mode == WORK_MODE_USB_DEBUG)
	{
		unsigned int val;

		printf("run usb debug\n");
		if(sunxi_usb_dev_register(2))
		{
			printf("sunxi usb test: invalid usb device\n");
		}

		asm("mrc p15, 0, %0, c1, c0, 0	@ get CR" : "=r" (val) : : "cc");
		val &= ~(1<<2);
		asm volatile("mcr p15, 0, %0, c1, c0, 0	@ set CR" : : "r" (val) : "cc");

		sunxi_usb_main_loop(0);
	}
	else if(uboot_spare_head.boot_data.work_mode == WORK_MODE_SPRITE_RECOVERY)
	{
		printf("run sprite recovery\n");
		sprite_led_init();
		ret = sprite_form_sysrecovery();
		sprite_led_exit(ret);
		return ret;
	}
	else
	{
		printf("others\n");
	}

	return 0;
}

U_BOOT_CMD(
	sprite_test, 2, 0, do_sprite_test,
	"do a sprite test",
	"NULL"
);



int do_fastboot_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("run usb fastboot\n");
	if(sunxi_usb_dev_register(3))
	{
		printf("sunxi usb test: invalid usb device\n");
	}
	sunxi_usb_main_loop(0);

	return 0;
}


U_BOOT_CMD(
	fastboot_test, 2, 0, do_fastboot_test,
	"do a sprite test",
	"NULL"
);

int do_mass_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("run usb mass\n");
	if(sunxi_usb_dev_register(1))
	{
		printf("sunxi usb test: invalid usb device\n");
	}
	sunxi_usb_main_loop(0);

	return 0;
}


U_BOOT_CMD(
	mass_test, 2, 0, do_mass_test,
	"do a usb mass test",
	"NULL"
);

int do_efex_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("run usb efex_test\n");
	if(sunxi_usb_dev_register(5))
	{
		printf("sunxi usb test: invalid usb device\n");
	}
	sunxi_usb_main_loop(2500);

	return 0;
}
U_BOOT_CMD(
	efex_test, 2, 0, do_efex_test,
	"do a usb efex test",
	"NULL"
);