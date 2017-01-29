/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
/* TODO: can we just include all these headers whether needed or not? */
#if defined(CONFIG_CMD_BEDBUG)
#include <bedbug/type.h>
#endif
#ifdef CONFIG_HAS_DATAFLASH
#include <dataflash.h>
#endif
#include <dm.h>
#include <environment.h>
#include <fdtdec.h>
#if defined(CONFIG_CMD_IDE)
#include <ide.h>
#endif
#include <initcall.h>
#ifdef CONFIG_PS2KBD
#include <keyboard.h>
#endif
#if defined(CONFIG_CMD_KGDB)
#include <kgdb.h>
#endif
#include <logbuff.h>
#include <malloc.h>
#ifdef CONFIG_BITBANGMII
#include <miiphy.h>
#endif
#include <mmc.h>
#include <nand.h>
#include <onenand_uboot.h>
#include <scsi.h>
#include <serial.h>
#include <spi.h>
#include <stdio_dev.h>
#include <trace.h>
#include <watchdog.h>
#ifdef CONFIG_ADDR_MAP
#include <asm/mmu.h>
#endif
#include <asm/sections.h>
#ifdef CONFIG_X86
#include <asm/init_helpers.h>
#endif
#include <dm/root.h>
#include <linux/compiler.h>
#include <linux/err.h>
#include <fdt_support.h>

#if defined(CONFIG_ALLWINNER)
#include <private_uboot.h>
#include <sunxi_board.h>
#include <sys_partition.h>
#include <sys_config.h>
#include <smc.h>
#include <securestorage.h>
#include <arisc.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

ulong monitor_flash_len;

int __board_flash_wp_on(void)
{
	/*
	 * Most flashes can't be detected when write protection is enabled,
	 * so provide a way to let U-Boot gracefully ignore write protected
	 * devices.
	 */
	return 0;
}

int board_flash_wp_on(void)
	__attribute__ ((weak, alias("__board_flash_wp_on")));

void __cpu_secondary_init_r(void)
{
}

void cpu_secondary_init_r(void)
	__attribute__ ((weak, alias("__cpu_secondary_init_r")));

static int initr_secondary_cpu(void)
{
	/*
	 * after non-volatile devices & environment is setup and cpu code have
	 * another round to deal with any initialization that might require
	 * full access to the environment or loading of some image (firmware)
	 * from a non-volatile device
	 */
	/* TODO: maybe define this for all archs? */
	cpu_secondary_init_r();

	return 0;
}

static int initr_trace(void)
{
#ifdef CONFIG_TRACE
	trace_init(gd->trace_buff, CONFIG_TRACE_BUFFER_SIZE);
#endif

	return 0;
}

static int initr_reloc(void)
{
	set_working_fdt_addr((void*)gd->fdt_blob);
	
	gd->flags |= GD_FLG_RELOC;	/* tell others: relocation done */
	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_R, "board_init_r");
	return 0;
}

#ifdef CONFIG_ARM
/*
 * Some of these functions are needed purely because the functions they
 * call return void. If we change them to return 0, these stubs can go away.
 */
static int initr_caches(void)
{
	/* Enable caches */
	enable_caches();
	return 0;
}
#endif

__weak int fixup_cpu(void)
{
	return 0;
}

static int initr_reloc_global_data(void)
{
#ifdef __ARM__
	
	//ulong head_align_size = get_spare_head_size();
	monitor_flash_len = _end - __image_copy_start;// + head_align_size;
	
#elif !defined(CONFIG_SANDBOX)
	monitor_flash_len = (ulong)&__init_end - gd->relocaddr;
#endif
#if defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
	/*
	 * The gd->cpu pointer is set to an address in flash before relocation.
	 * We need to update it to point to the same CPU entry in RAM.
	 * TODO: why not just add gd->reloc_ofs?
	 */
	gd->arch.cpu += gd->relocaddr - CONFIG_SYS_MONITOR_BASE;

	/*
	 * If we didn't know the cpu mask & # cores, we can save them of
	 * now rather than 'computing' them constantly
	 */
	fixup_cpu();
#endif
#ifdef CONFIG_SYS_EXTRA_ENV_RELOC
	/*
	 * Some systems need to relocate the env_addr pointer early because the
	 * location it points to will get invalidated before env_relocate is
	 * called.  One example is on systems that might use a L2 or L3 cache
	 * in SRAM mode and initialize that cache from SRAM mode back to being
	 * a cache in cpu_init_r.
	 */
	gd->env_addr += gd->relocaddr - CONFIG_SYS_MONITOR_BASE;
#endif
	return 0;
}

static int initr_serial(void)
{
	serial_initialize();
	return 0;
}




extern void mem_noncache_malloc_init(uint noncache_start, uint noncache_size);
static int initr_malloc(void)
{
	ulong malloc_start;

	malloc_start = gd->relocaddr - TOTAL_MALLOC_LEN;
	mem_malloc_init((ulong)map_sysmem(malloc_start, TOTAL_MALLOC_LEN),
			TOTAL_MALLOC_LEN);
	debug("malloc start addr is %lx, size %dk \n",malloc_start,TOTAL_MALLOC_LEN>>10 );

#ifdef  CONFIG_NONCACHE_MEMORY
{
	mem_noncache_malloc_init(gd->malloc_noncache_start, CONFIG_NONCACHE_MEMORY_SIZE);
	debug("no cache malloc start addr is %lx, size %dk \n",gd->malloc_noncache_start,TOTAL_MALLOC_LEN>>10 );
}
#endif
	
	return 0;
}

__weak int power_init_board(void)
{
#ifdef CONFIG_SUNXI_AXP
	extern int axp_reinit(void);
	axp_reinit();
#endif
	return 0;
}

static int initr_announce(void)
{
	debug("Now running in RAM - U-Boot at: %08lx\n", gd->relocaddr);
	return 0;
}



#ifdef CONFIG_GENERIC_MMC
int initr_mmc(void)
{
	puts("MMC:   ");
	mmc_initialize(gd->bd);
	return 0;
}
#endif


/*
 * Tell if it's OK to load the environment early in boot.
 *
 * If CONFIG_OF_CONFIG is defined, we'll check with the FDT to see
 * if this is OK (defaulting to saying it's OK).
 *
 * NOTE: Loading the environment early can be a bad idea if security is
 *       important, since no verification is done on the environment.
 *
 * @return 0 if environment should not be loaded, !=0 if it is ok to load
 */
static int should_load_env(void)
{
#ifdef CONFIG_OF_CONTROL
	return fdtdec_get_config_int(gd->fdt_blob, "load-environment", 1);
#elif defined CONFIG_DELAY_ENVIRONMENT
	return 0;
#else
	return 1;
#endif
}

static int initr_env(void)
{
	/* initialize environment */
	if (should_load_env())
		env_relocate();
	else
		set_default_env(NULL);

	/* Initialize from environment */
	load_addr = getenv_ulong("loadaddr", 16, load_addr);
	return 0;
}

static int initr_jumptable(void)
{
	jumptable_init();
	return 0;
}


/* enable exceptions */
#ifdef CONFIG_ARM
static int initr_enable_interrupts(void)
{
	enable_interrupts();
	return 0;
}
#endif


static int run_main_loop(void)
{
	sunxi_tick_printf("inter uboot shell\n");
	/* main_loop() can return to retry autoboot, if so just run it again */
	for (;;)
		main_loop();
	return 0;
}

extern int script_init(void);
extern int PowerCheck(void);
extern int sunxi_keydata_burn_by_usb(void);

static int initr_sunxi_base(void)
{
	board_late_init();
	return 0;
}

static int sunxi_burn_key(void)
{

#ifndef CONFIG_SUNXI_SPINOR_PLATFORM
	sunxi_keydata_burn_by_usb();
#endif
	return 0;
}


#ifdef  CONFIG_SUNXI_DISPLAY
static int initr_sunxi_display(void)
{

	int workmode = uboot_spare_head.boot_data.work_mode;
	if(!((workmode == WORK_MODE_BOOT) ||
		(workmode == WORK_MODE_CARD_PRODUCT) ||
		(workmode == WORK_MODE_SPRITE_RECOVERY)))
	{
		return 0;
	}
	tick_printf("start\n");
	drv_disp_init();

#ifdef CONFIG_SUNXI_HDCP_IN_SECURESTORAGE
{
	int hdcpkey_enable=0;
	int ret = 0;
	ret = script_parser_fetch("hdmi_para", "hdmi_hdcp_enable", &hdcpkey_enable, 1);
	if((ret) || (hdcpkey_enable != 1))
	{
		board_display_device_open();
		board_display_layer_request();
	}
	//here: write key to hardware
	if(hdcpkey_enable==1)
	{
		char buffer[4096];
		int data_len;
		int ret0;

		memset(buffer, 0, 4096);
		ret0 = sunxi_secure_storage_init();
		if(ret0)
		{
			printf("sunxi init secure storage failed\n");
		}
		else
		{
			ret0 = sunxi_secure_storage_read("hdcpkey", buffer, 4096, &data_len);
			if(ret0)
			{
				printf("probe hdcp key failed\n");
			}
			else
			{
				ret0 = smc_aes_bssk_decrypt_to_keysram(buffer, data_len);
				if(ret0)
				{
					printf("push hdcp key failed\n");
				}
				else
				{
					board_display_device_open();
					board_display_layer_request();
				}
			}
		}
	}
}
#else
	board_display_device_open();
	board_display_layer_request();
#endif
	tick_printf("end\n");
	return 0;
}
#endif

static int initr_sunxi_flash(void)
{
	int ret = 0;

#ifndef CONFIG_ARCH_HOMELET	//for pad used
	ret = sunxi_flash_handle_init();
	if(!ret)
	{
		sunxi_partition_init();
	}
#endif
	return ret;

}

/*
 * Over time we hope to remove these functions with code fragments and
 * stub funtcions, and instead call the relevant function directly.
 *
 * We also hope to remove most of the driver-related init and do it if/when
 * the driver is later used.
 *
 * TODO: perhaps reset the watchdog in the initcall function after each call?
 */
init_fnc_t init_sequence_r[] = {
	initr_trace,
	initr_reloc,
	/* TODO: could x86/PPC have this also perhaps? */
#ifdef CONFIG_ARM
	initr_caches,
	board_init,	/* Setup chipselects */
#endif
	/*
	 * TODO: printing of the clock inforamtion of the board is now
	 * implemented as part of bdinfo command. Currently only support for
	 * davinci SOC's is added. Remove this check once all the board
	 * implement this.
	 */
	initr_reloc_global_data,
	initr_serial,
	initr_announce,
	initr_malloc,
	script_init,
	bootstage_relocate,
	power_init_board,
	initr_secondary_cpu,
	stdio_init,
	initr_jumptable,
	console_init_r,		/* fully init console as a device */
	interrupt_init,
#if defined(CONFIG_ARM) || defined(CONFIG_x86)
	initr_enable_interrupts,
#endif
#ifdef CONFIG_SUNXI
	//sunxi_arisc_probe,   //call this func here for optimize boot time
	#ifdef  CONFIG_SUNXI_DISPLAY
	initr_sunxi_display,
	#endif
	initr_sunxi_flash,
	sunxi_burn_key,
	initr_env,
	initr_sunxi_base,
	//sunxi_arisc_wait_ready,  // must call this func here,because will call power_check later.
	PowerCheck,
#endif
	run_main_loop,
};

extern ulong _reset_vec;
extern ulong _undefined_instruction;
extern ulong _software_interrupt;
extern ulong _prefetch_abort;
extern ulong _data_abort;
extern ulong _not_used;
extern ulong _irq;
extern ulong _fiq;

#ifndef CONFIG_ARM64
#ifdef CONFIG_USE_IRQ
void intr_vector_fix(void)
{
	int i = 0;
	ulong * vec_array[8] = {0};
	vec_array[i++] = &_reset_vec;
	vec_array[i++] = &_undefined_instruction;
	vec_array[i++] = &_software_interrupt;
	vec_array[i++] = &_prefetch_abort;
	vec_array[i++] = &_data_abort;
	vec_array[i++] = &_not_used;
	vec_array[i++] = &_irq;
	vec_array[i++] = &_fiq;

	for(i =0; i < 8; i++)
	{
		debug("before fix: vector %d, addr %lx ,value 0x%lx\n", i ,(ulong)vec_array[i], *(vec_array[i]));
		*(vec_array[i]) +=  (gd->reloc_off);
		debug("after  fix: vector %d, addr %lx ,value 0x%lx\n", i ,(ulong)vec_array[i], *(vec_array[i]));
	}
}
#endif
#endif

void board_init_r(gd_t *new_gd, ulong dest_addr)
{

#ifdef CONFIG_NEEDS_MANUAL_RELOC
	int i;
#endif

#ifndef CONFIG_X86
	gd = new_gd;
#endif

#ifdef CONFIG_NEEDS_MANUAL_RELOC
	for (i = 0; i < ARRAY_SIZE(init_sequence_r); i++)
		init_sequence_r[i] += gd->reloc_off;
#endif

#ifndef CONFIG_ARM64
#ifdef CONFIG_USE_IRQ
	intr_vector_fix();
#endif
#endif
	if (initcall_run_list(init_sequence_r))
		hang();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}
