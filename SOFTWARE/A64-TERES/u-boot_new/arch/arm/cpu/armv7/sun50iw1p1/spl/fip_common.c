#include <linux/types.h>
#include <config.h>
//#include "bl_common.h"
#include <private_uboot.h>
#include <asm/io.h>

//index for merge uboot with bl31,bl32
/* EL3 Runtime Firmware BL31 */
#define BL31_IMAGE_NAME			0
/* Trusted OS*/
#define BL32_IMAGE_NAME			1
/* Scp*/
#define SCP_IMAGE_NAME			2



//**********************************************************************
extern int printf(const char *fmt, ...);
extern void * memcpy(void * dest,const void *src,size_t count);
extern void * memset(void * s, int c, size_t count);

int sunxi_deassert_arisc(void)
{
	printf("set arisc reset to de-assert state\n");
	{
		volatile unsigned long value;
		value = readl(SUNXI_RCPUCFG_BASE + 0x0);
		value &= ~1;
		writel(value, SUNXI_RCPUCFG_BASE + 0x0);
		value = readl(SUNXI_RCPUCFG_BASE + 0x0);
		value |= 1;
		writel(value, SUNXI_RCPUCFG_BASE + 0x0);
	}

	return 0;
}


int load_image(u32 image_index,u32 image_base)
{
	struct spare_boot_head_t* header;
	u32 offset;
	u32 image_size;

	/* Obtain a reference to the image by querying the platform layer */
	header = (struct spare_boot_head_t* )CONFIG_SYS_TEXT_BASE;
	offset = header->boot_ext[image_index].data[0];
	image_size = header->boot_ext[image_index].data[1];

	if(0 == offset || 0 == image_size)
	{
		//printf("index %d for file not found\n", image_index);
		return -1;
	}
	if(image_index == SCP_IMAGE_NAME)
	{
		memcpy((void*)SCP_SRAM_BASE,(void*)(CONFIG_SYS_TEXT_BASE+offset), CONFIG_SYS_SRAMA2_SIZE);
		memcpy((void*)SCP_DRAM_BASE,(void*)(CONFIG_SYS_TEXT_BASE+offset+0x18000), SCP_DRAM_SIZE);
	}
	else
	{
		memcpy((void*)image_base,(void*)(CONFIG_SYS_TEXT_BASE+offset), image_size);
	}
	printf("Loading file %d at address 0x%x,size 0x%x success\n",image_index, image_base,image_size);
	return 0;
}



/*******************************************************************************
 * The only thing to do in BL2 is to load further images and pass control to
 * BL3-1. The memory occupied by BL2 will be reclaimed by BL3-x stages. BL2 runs
 * entirely in S-EL1.
 ******************************************************************************/
int load_fip(int *use_monitor)
{
	int e;
	struct spare_boot_head_t* header;

	header = (struct spare_boot_head_t* )CONFIG_SYS_TEXT_BASE;

	printf("boot0: %s\n", "start load other image");
	printf("boot0: Loading BL3-1\n");
	e = load_image(BL31_IMAGE_NAME,BL31_BASE);
	if (e)
	{
		printf("Failed to load BL3-1\n");
	}
	else
	{
		*use_monitor = 1;
		header->boot_data.secureos_exist = 1;
	}

	printf("boot0: Loading scp\n");
	e = load_image(SCP_IMAGE_NAME,SCP_SRAM_BASE);
	if (e)
	{
		printf("Failed to load SCP\n");
		return -1;
	}
	else
	{
		sunxi_deassert_arisc();
	}

	/*
	printf("boot0: Loading BL3-2\n");
	e = load_image(BL32_IMAGE_NAME,BL32_BASE);
	if (e) {
		ERROR("Failed to load BL3-2 (%d)\n", e);
		return -1;
	}
	printf("boot0: Loading BL3-3\n");
	e = load_image(BL33_IMAGE_NAME,BL33_BASE);
	if (e) {
		ERROR("Failed to load BL3-3 (%d)\n", e);
		return -1;
	}
	*/
	/* Flush the params to be passed to memory */
	//bl2_plat_flush_bl31_params();

	return 0;
}


#if 0
#include "fip_type.h"
#include <config.h>
#include "bl_common.h"
#include <private_uboot.h>

//index for merge uboot with bl31,bl32
/* EL3 Runtime Firmware BL31 */
#define BL31_IMAGE_NAME			0
/* Trusted OS*/
#define BL32_IMAGE_NAME			1



#define UBOOT_BASE		0x4A000000
#define DRAM_BASE		0x40000000


//bl31(monitor area)
#define PLAT_TRUSTED_MONITOR_BASE	0x7e000000     /*for test on fpga*/
#define PLAT_TRUSTED_MONITOR_SIZE	0x00100000	 /* 1MB */

//bl32(trusted os area)
#define PLAT_TRUSTED_DRAM_BASE	0x7f000000      /*0x40000000+0x40000000-0x02000000*/
#define PLAT_TRUSTED_DRAM_SIZE	0x01000000	/* 16 MB */

//shared memory for monitor
#define PLAT_SHARED_RAM_BASE	PLAT_TRUSTED_DRAM_BASE
#define PLAT_SHARED_RAM_SIZE	0x1000

//bl31 base addr
#define BL31_BASE	PLAT_TRUSTED_MONITOR_BASE

/*bl32  run on trusted dram*/
#define BL32_BASE	(PLAT_TRUSTED_DRAM_BASE + PLAT_SHARED_RAM_SIZE)

/*bl33 base addr*/
#define BL33_BASE	UBOOT_BASE



/*******************************************************************************
 *  Shared Data
 ******************************************************************************/
/* Entrypoint mailboxes */
#define MBOX_BASE		PLAT_SHARED_RAM_BASE
#define MBOX_SIZE		0x200

/* Base address where parameters to BL31 are stored */
#define PARAMS_BASE		(MBOX_BASE + MBOX_SIZE)


//************************************arch macro********************************

#define MODE_RW_SHIFT		0x4
#define MODE_RW_MASK		0x1
#define MODE_RW_64			0x0
#define MODE_RW_32			0x1

#define MODE_EL_SHIFT		0x2
#define MODE_EL_MASK		0x3
#define MODE_EL3		0x3
#define MODE_EL2		0x2
#define MODE_EL1		0x1
#define MODE_EL0		0x0

#define MODE_SP_SHIFT		0x0
#define MODE_SP_MASK		0x1
#define MODE_SP_EL0		0x0
#define MODE_SP_ELX		0x1

/* CPSR/SPSR definitions */
#define DAIF_FIQ_BIT		(1 << 0)
#define DAIF_IRQ_BIT		(1 << 1)
#define DAIF_ABT_BIT		(1 << 2)
#define DAIF_DBG_BIT		(1 << 3)
#define SPSR_DAIF_SHIFT		6
#define SPSR_DAIF_MASK		0xf

#define SPSR_AIF_SHIFT		6
#define SPSR_AIF_MASK		0x7

#define SPSR_E_SHIFT		9
#define SPSR_E_MASK			0x1
#define SPSR_E_LITTLE		0x0
#define SPSR_E_BIG			0x1

#define SPSR_T_SHIFT		5
#define SPSR_T_MASK			0x1
#define SPSR_T_ARM			0x0
#define SPSR_T_THUMB		0x1


#define DISABLE_ALL_EXCEPTIONS \
		(DAIF_FIQ_BIT | DAIF_IRQ_BIT | DAIF_ABT_BIT | DAIF_DBG_BIT)

#define SPSR_64(el, sp, daif)				\
		(MODE_RW_64 << MODE_RW_SHIFT |			\
		((el) & MODE_EL_MASK) << MODE_EL_SHIFT |	\
		((sp) & MODE_SP_MASK) << MODE_SP_SHIFT |	\
		((daif) & SPSR_DAIF_MASK) << SPSR_DAIF_SHIFT)


#define MODE32_SHIFT		0
#define MODE32_MASK		0xf
#define MODE32_usr		0x0
#define MODE32_fiq		0x1
#define MODE32_irq		0x2
#define MODE32_svc		0x3
#define MODE32_mon		0x6
#define MODE32_abt		0x7
#define MODE32_hyp		0xa
#define MODE32_und		0xb
#define MODE32_sys		0xf

#define GET_RW(mode)		(((mode) >> MODE_RW_SHIFT) & MODE_RW_MASK)
#define GET_EL(mode)		(((mode) >> MODE_EL_SHIFT) & MODE_EL_MASK)
#define GET_SP(mode)		(((mode) >> MODE_SP_SHIFT) & MODE_SP_MASK)
#define GET_M32(mode)		(((mode) >> MODE32_SHIFT) & MODE32_MASK)


#define SPSR_64(el, sp, daif)				\
	(MODE_RW_64 << MODE_RW_SHIFT |			\
	((el) & MODE_EL_MASK) << MODE_EL_SHIFT |	\
	((sp) & MODE_SP_MASK) << MODE_SP_SHIFT |	\
	((daif) & SPSR_DAIF_MASK) << SPSR_DAIF_SHIFT)

#define SPSR_MODE32(mode, isa, endian, aif)		\
	(MODE_RW_32 << MODE_RW_SHIFT |			\
	((mode) & MODE32_MASK) << MODE32_SHIFT |	\
	((isa) & SPSR_T_MASK) << SPSR_T_SHIFT |		\
	((endian) & SPSR_E_MASK) << SPSR_E_SHIFT |	\
	((aif) & SPSR_AIF_MASK) << SPSR_AIF_SHIFT)



//**********************************************************************
extern int printf(const char *fmt, ...);
extern void * memcpy(void * dest,const void *src,size_t count);
extern void * memset(void * s, int c, size_t count);

#define ERROR(...)	printf("ERROR:   " __VA_ARGS__)
#define WARN(...)	printf("WARN:   " __VA_ARGS__)
#define INFO(...)	printf("INFO:   " __VA_ARGS__)
#define NULL ((void *)0)
#define assert(x) { if(!(x)) {while(*(volatile uint32_t*)(0x4a000000) != 0x1122);}}


static bl31_params_t *bl2_to_bl31_params;
static entry_point_info_t *bl31_ep_info;


/*******************************************************************************
 * Before calling this function BL31 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL31 and set SPSR and security state.
 * On FVP we are only setting the security state, entrypoint
 ******************************************************************************/
void bl2_plat_set_bl31_ep_info(image_info_t *bl31_image_info,
					entry_point_info_t *bl31_ep_info)
{
	SET_SECURITY_STATE(bl31_ep_info->h.attr, SECURE);
	bl31_ep_info->spsr = SPSR_64(MODE_EL3, MODE_SP_ELX,
					DISABLE_ALL_EXCEPTIONS);
}


/*******************************************************************************
 * Before calling this function BL32 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL32 and set SPSR and security state.
 * On FVP we are only setting the security state, entrypoint
 ******************************************************************************/
void bl2_plat_set_bl32_ep_info(image_info_t *bl32_image_info,
					entry_point_info_t *bl32_ep_info)
{
	SET_SECURITY_STATE(bl32_ep_info->h.attr, SECURE);
	bl32_ep_info->spsr = 0;
}

/*******************************************************************************
 * Before calling this function BL33 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL33 and set SPSR and security state.
 * On FVP we are only setting the security state, entrypoint
 ******************************************************************************/
void bl2_plat_set_bl33_ep_info(image_info_t *image,
					entry_point_info_t *bl33_ep_info)
{
	
	
	//when use AA64 uboot
	//SET_SECURITY_STATE(bl33_ep_info->h.attr, NON_SECURE);
	//bl33_ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	
	//when use AA32 uboot
	SET_SECURITY_STATE(bl33_ep_info->h.attr, NON_SECURE);
	bl33_ep_info->spsr = SPSR_MODE32(MODE32_svc,SPSR_T_ARM,SPSR_E_LITTLE,DISABLE_ALL_EXCEPTIONS);
	INFO("bl33 spsr is 0x%x\n", bl33_ep_info->spsr);
}


/*******************************************************************************
 * This function assigns a pointer to the memory that the platform has kept
 * aside to pass platform specific and trusted firmware related information
 * to BL31. This memory is allocated by allocating memory to
 * bl2_to_bl31_params_mem_t structure which is a superset of all the
 * structure whose information is passed to BL31
 * NOTE: This function should be called only once and should be done
 * before generating params to BL31
 ******************************************************************************/
bl31_params_t *bl2_plat_get_bl31_params(void)
{
	bl2_to_bl31_params_mem_t *bl31_params_mem;

	/*
	 * Allocate the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	bl31_params_mem = (bl2_to_bl31_params_mem_t *)PARAMS_BASE;
	memset((void *)PARAMS_BASE, 0, sizeof(bl2_to_bl31_params_mem_t));

	/* Assign memory for TF related information */
	bl2_to_bl31_params = &bl31_params_mem->bl31_params;
	SET_PARAM_HEAD(bl2_to_bl31_params, PARAM_BL31, VERSION_1, 0);

	/* Fill BL31 related information */
	bl31_ep_info = &bl31_params_mem->bl31_ep_info;
	bl2_to_bl31_params->bl31_image_info = &bl31_params_mem->bl31_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl31_image_info, PARAM_IMAGE_BINARY,
						VERSION_1, 0);

	/* Fill BL32 related information if it exists */
	if (BL32_BASE) {
		bl2_to_bl31_params->bl32_ep_info =
					&bl31_params_mem->bl32_ep_info;
		SET_PARAM_HEAD(bl2_to_bl31_params->bl32_ep_info,
					PARAM_EP, VERSION_1, 0);
		bl2_to_bl31_params->bl32_image_info =
					&bl31_params_mem->bl32_image_info;
		SET_PARAM_HEAD(bl2_to_bl31_params->bl32_image_info,
					PARAM_IMAGE_BINARY,
					VERSION_1, 0);
	}

	/* Fill BL33 related information */
	bl2_to_bl31_params->bl33_ep_info = &bl31_params_mem->bl33_ep_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl33_ep_info,
					PARAM_EP, VERSION_1, 0);
	bl2_to_bl31_params->bl33_image_info = &bl31_params_mem->bl33_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl33_image_info, PARAM_IMAGE_BINARY,
					VERSION_1, 0);

	return bl2_to_bl31_params;
}

#if 0
int load_image(meminfo_t *mem_layout,
	       const char *image_name,
	       uint64_t image_base,
	       image_info_t *image_data,
	       entry_point_info_t *entry_point_info)
{
	fip_toc_header_t *header;
	fip_toc_entry_t *fip_file_entry;
	uint32_t  bufer_offset = 0;
	uuid_t file_uuid;
	int found_file = 0;
	const uuid_t uuid_null = {0};

	//assert(mem_layout != NULL);
	assert(image_name != NULL);
	assert(image_data != NULL);
	assert(image_data->h.version >= VERSION_1);

	/* Obtain a reference to the image by querying the platform layer */
	header = (fip_toc_header_t*)FIP_LOAD_ADDR;

	if (!is_valid_header(header))
	{
		printf("Firmware Image Package header check failed.\n");
		return -1;
	}
	else
	{
		printf("FIP header looks OK.\n");
	}
	bufer_offset = sizeof(fip_toc_header_t);
	fip_file_entry = (fip_toc_entry_t *)(FIP_LOAD_ADDR+bufer_offset);

	if(file_to_uuid(image_name, &file_uuid))
	{
		printf("file %s is not config in name_uuid table\n", image_name);
		return -1;
	}

	do {
		if (compare_uuids(&fip_file_entry->uuid,
				  &file_uuid) == 0)
		{
			found_file = 1;
			break;
		}
		fip_file_entry++;
	}while (compare_uuids(&fip_file_entry->uuid, &uuid_null) != 0);


	if(!found_file)
	{
		printf("uuid for file %s not found\n", image_name);
		return -1;
	}

	memcpy((void*)image_base,(void*)(FIP_LOAD_ADDR+fip_file_entry->offset_address), fip_file_entry->size);
	image_data->image_base = image_base;
	image_data->image_size = fip_file_entry->size;

	if (entry_point_info != NULL)
		entry_point_info->pc = image_base;

	printf("Loading file '%s' at address 0x%x,size 0x%x sucess\n",
		image_name, (uint32_t)image_base,(uint32_t)image_data->image_size);
	return 0;
}
#endif
int load_image(meminfo_t *mem_layout,
	       uint32_t image_index,
	       uint32_t image_base,
	       image_info_t *image_data,
	       entry_point_info_t *entry_point_info)
{
	struct spare_boot_head_t* header;
	
	uint32_t offset;
	uint32_t image_size;

	assert(image_data != NULL);
	assert(image_data->h.version >= VERSION_1);

	/* Obtain a reference to the image by querying the platform layer */
	header = (struct spare_boot_head_t* )UBOOT_BASE;
	offset = header->boot_ext[image_index].data[0];
	image_size = header->boot_ext[image_index].data[1];

	if(0 == offset || 0 == image_size)
	{
		printf("index %d for file not found\n", image_index);
		return -1;
	}

	memcpy((void*)image_base,(void*)(UBOOT_BASE+offset), image_size);
	image_data->image_base = image_base;
	image_data->image_size = image_size;

	if (entry_point_info != NULL)
		entry_point_info->pc = image_base;

	INFO("Loading file %d at address 0x%x,size 0x%x success\n",
		image_index, (uint32_t)image_base,(uint32_t)image_data->image_size);
	return 0;
}


/*******************************************************************************
 * Load the BL3-1 image.
 * The bl2_to_bl31_params and bl31_ep_info params will be updated with the
 * relevant BL3-1 information.
 * Return 0 on success, a negative error code otherwise.
 ******************************************************************************/
static int load_bl31(bl31_params_t *bl2_to_bl31_params,
		     entry_point_info_t *bl31_ep_info)
{
	meminfo_t *bl2_tzram_layout;
	int e;

	INFO("BL2: Loading BL3-1\n");
	assert(bl2_to_bl31_params != NULL);
	assert(bl31_ep_info != NULL);

	/* Find out how much free trusted ram remains after BL2 load */
	bl2_tzram_layout = NULL;//bl2_plat_sec_mem_layout();

	/* Set the X0 parameter to BL3-1 */
	bl31_ep_info->args.arg0 = (unsigned long)bl2_to_bl31_params;

	/* Load the BL3-1 image */
	e = load_image(bl2_tzram_layout,
		       BL31_IMAGE_NAME,
		       BL31_BASE,
		       bl2_to_bl31_params->bl31_image_info,
		       bl31_ep_info);

	if (e == 0)
		bl2_plat_set_bl31_ep_info(bl2_to_bl31_params->bl31_image_info,
					  bl31_ep_info);

	return e;
}




/*******************************************************************************
 * Load the BL3-2 image if there's one.
 * The bl2_to_bl31_params param will be updated with the relevant BL3-2
 * information.
 * If a platform does not want to attempt to load BL3-2 image it must leave
 * BL32_BASE undefined.
 * Return 0 on success or if there's no BL3-2 image to load, a negative error
 * code otherwise.
 ******************************************************************************/
static int load_bl32(bl31_params_t *bl2_to_bl31_params)
{
	int e = 0;
#ifdef BL32_BASE
	//meminfo_t bl32_mem_info;

	INFO("BL2: Loading BL3-2\n");
	assert(bl2_to_bl31_params != NULL);

	/*
	 * It is up to the platform to specify where BL3-2 should be loaded if
	 * it exists. It could create space in the secure sram or point to a
	 * completely different memory.
	 */
	e = load_image(NULL,
		       BL32_IMAGE_NAME,
		       BL32_BASE,
		       bl2_to_bl31_params->bl32_image_info,
		       bl2_to_bl31_params->bl32_ep_info);

	if (e == 0) {
		bl2_plat_set_bl32_ep_info(
			bl2_to_bl31_params->bl32_image_info,
			bl2_to_bl31_params->bl32_ep_info);
	}
#endif /* BL32_BASE */

	return e;
}

/*******************************************************************************
 * Load the BL3-3 image.
 * The bl2_to_bl31_params param will be updated with the relevant BL3-3
 * information.
 * Return 0 on success, a negative error code otherwise.
 ******************************************************************************/
static int load_bl33(bl31_params_t *bl2_to_bl31_params)
{
	
	INFO("BL2: Loading BL3-3\n");
	assert(bl2_to_bl31_params != NULL);

	//bl33 is in place ,just set para
	bl2_to_bl31_params->bl33_image_info->image_base = BL33_BASE;
	bl2_to_bl31_params->bl33_image_info->image_size = 1;//value is uboot size,don't care here

	if (bl2_to_bl31_params->bl33_ep_info != NULL)
		bl2_to_bl31_params->bl33_ep_info->pc = BL33_BASE;

	
	bl2_plat_set_bl33_ep_info(bl2_to_bl31_params->bl33_image_info,
					  bl2_to_bl31_params->bl33_ep_info);

	return 0;
}

/*******************************************************************************
 * This function returns a pointer to the shared memory that the platform
 * has kept to point to entry point information of BL31 to BL2
 ******************************************************************************/
struct entry_point_info *bl2_plat_get_bl31_ep_info(void)
{
	return bl31_ep_info;
}



/*******************************************************************************
 * The only thing to do in BL2 is to load further images and pass control to
 * BL3-1. The memory occupied by BL2 will be reclaimed by BL3-x stages. BL2 runs
 * entirely in S-EL1.
 ******************************************************************************/
int bl2_main(void)
{
	bl31_params_t *bl2_to_bl31_params;
	entry_point_info_t *bl31_ep_info;
	int e;

	INFO("BL2: %s\n", "start load other image");

	/*
	 * Get a pointer to the memory the platform has set aside to pass
	 * information to BL3-1.
	 */
	bl2_to_bl31_params = bl2_plat_get_bl31_params();
	bl31_ep_info = bl2_plat_get_bl31_ep_info();

	e = load_bl31(bl2_to_bl31_params, bl31_ep_info);
	if (e) {
		ERROR("Failed to load BL3-1 (%d)\n", e);
		return -1;
	}

	e = load_bl32(bl2_to_bl31_params);
	if (e)
		WARN("Failed to load BL3-2 (%d)\n", e);

	e = load_bl33(bl2_to_bl31_params);
	if (e) {
		ERROR("Failed to load BL3-3 (%d)\n", e);
		return -1;

	}
	/* Flush the params to be passed to memory */
	//bl2_plat_flush_bl31_params();

	return 0;
}
#endif
