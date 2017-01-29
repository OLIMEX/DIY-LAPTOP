#include "nand_blk.h"
#include "nand_dev.h"

/*****************************************************************************/

extern struct nand_blk_ops mytr;
extern struct _nand_info* p_nand_info;
extern unsigned int flush_cache_num;

extern int  init_blklayer(void);
extern int init_blklayer_for_dragonboard(void);
extern void   exit_blklayer(void);
extern void set_cache_level(struct _nand_info*nand_info,unsigned short cache_level);
extern void set_capacity_level(struct _nand_info*nand_info,unsigned short capacity_level);
extern __u32 nand_wait_rb_mode(void);
extern __u32 nand_wait_dma_mode(void);
extern void do_nand_interrupt(unsigned int no);
extern void print_nftl_zone(void * zone);
extern int NAND_get_storagetype(void);
extern int NAND_Get_Dragonboard_Flag(void);

int test_mbr(uchar* data);
extern int NAND_Print_DBG(const char *fmt, ...);
extern __u32 NAND_GetMaxChannelCnt(void);

#define BLK_ERR_MSG_ON

static unsigned int channel0 = 0;

void  * NDFC0_BASE_ADDR = NULL;
void  * NDFC1_BASE_ADDR = NULL;
struct device *ndfc_dev;
struct platform_device *plat_dev_nand = NULL;
__u32 exit_probe_flag = 0;




/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
spinlock_t     nand_int_lock;

static irqreturn_t nand_interrupt(int irq, void *channel)
{
    unsigned int no;
    unsigned long iflags;

    //nand_dbg_err("nand_interrupt_ch0!\n");
    spin_lock_irqsave(&nand_int_lock, iflags);


    no = *((unsigned int*)channel);

    do_nand_interrupt(no);

    spin_unlock_irqrestore(&nand_int_lock, iflags);

    return IRQ_HANDLED;
}
#if 0
/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
static int nand_early_suspend(void)
{
    struct _nftl_blk *nftl_blk;
    struct nand_blk_ops *tr = &mytr;

    nftl_blk = tr->nftl_blk_head.nftl_blk_next;

    nand_dbg_err("nand_early_suspend\n");
    while(nftl_blk != NULL)
    {
        nand_dbg_err("nand\n");
        mutex_lock(nftl_blk->blk_lock);
        nftl_blk->flush_write_cache(nftl_blk,0xffff);
        mutex_unlock(nftl_blk->blk_lock);
        nftl_blk = nftl_blk->nftl_blk_next;
    }
    return 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
static int nand_early_resume(void)
{
    nand_dbg_err("nand_early_resume\n");
    return 0;
}
#endif
/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
extern int nand_clean_zone_table(void *p);
static int nand_suspend(struct platform_device *plat_dev, pm_message_t state)
{
#if 0
	if(NORMAL_STANDBY== standby_type)
    {
        nand_dbg_err("[NAND] nand_suspend normal\n");

        NandHwNormalStandby();
    }
    else if(SUPER_STANDBY == standby_type)
    {
        nand_dbg_err("[NAND] nand_suspend super\n");
        NandHwSuperStandby();
    }

    nand_dbg_err("[NAND] nand_suspend ok \n");
#else
	nand_dbg_err("[NAND] nand_suspend\n");
    NandHwSuperStandby();
	nand_dbg_err("[NAND] nand_suspend ok \n");
#endif    
	return 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
extern int nand_find_zone_table(void *p);
static int nand_resume(struct platform_device *plat_dev)
{
#if 0
    if(NORMAL_STANDBY== standby_type){
        nand_dbg_err("[NAND] nand_resume normal\n");
        NandHwNormalResume();
    }else if(SUPER_STANDBY == standby_type){
        nand_dbg_err("[NAND] nand_resume super\n");
        NandHwSuperResume();
    }

    nand_dbg_err("[NAND] nand_resume ok \n");  
#else
	nand_dbg_err("[NAND] nand_resume\n");
    NandHwSuperResume();
	nand_dbg_err("[NAND] nand_resume ok \n");
#endif     
    return 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
u64 nand_dma_mask = DMA_BIT_MASK(32);
static int nand_probe(struct platform_device *plat_dev)
{
	__u32 irq;
	char * dev_name = "nand_dev";

	plat_dev_nand = plat_dev;
	ndfc_dev = &plat_dev->dev;

	plat_dev->dev.dma_mask = &nand_dma_mask;
	plat_dev->dev.coherent_dma_mask = DMA_BIT_MASK(32);

    spin_lock_init(&nand_int_lock);

    if((nand_wait_rb_mode() != 0) || (nand_wait_dma_mode() != 0))
	{
		nand_dbg_err("nand interrupt request\n");
		
		irq = irq_of_parse_and_map(ndfc_dev->of_node, 0);
        if (request_irq(irq, nand_interrupt, IRQF_DISABLED, dev_name, &channel0))
        {
            nand_dbg_err("nand interrupte ch0 irqno: %d register error\n", irq);
            return -EAGAIN;
        }
    }

	if(NAND_GetMaxChannelCnt() == 1)
	{
			NDFC0_BASE_ADDR = (void *)of_iomap(ndfc_dev->of_node, 0);
			nand_dbg_err("NDFC0_BASE_ADDR %p\n",NDFC0_BASE_ADDR);
			if (!NDFC0_BASE_ADDR)
			{		
				nand_dbg_err("Failed to map NDFC0 IO space\n");		
				return -EAGAIN;			
			}
	}
	else if(NAND_GetMaxChannelCnt() == 2)
	{
			NDFC0_BASE_ADDR = (void *)of_iomap(ndfc_dev->of_node, 0);
			if (!NDFC0_BASE_ADDR)
			{		
				nand_dbg_err("Failed to map NDFC0 IO space\n");		
				return -EAGAIN;			
			}
			NDFC1_BASE_ADDR = (void *)of_iomap(ndfc_dev->of_node, 1);
			if (!NDFC1_BASE_ADDR)
			{		
				nand_dbg_err("Failed to map NDFC1 IO space\n");		
				return -EAGAIN;			
			}
	}
	
	exit_probe_flag = 1;

    nand_dbg_inf("nand_probe\n");
    return 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
static int nand_remove(struct platform_device *plat_dev)
{
    nand_dbg_inf("nand_remove\n");
    return 0;
}

static void nand_release_dev(struct device *dev)
{
    nand_dbg_inf("nand_release dev\n");
    return ;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
uint32 shutdown_flush_write_cache(void)
{
    struct _nftl_blk *nftl_blk;
    struct nand_blk_ops *tr = &mytr;

    nftl_blk = tr->nftl_blk_head.nftl_blk_next;

    while(nftl_blk != NULL)
    {
        nand_dbg_err("shutdown_flush_write_cache\n");
        mutex_lock(nftl_blk->blk_lock);
        nftl_blk->flush_write_cache(nftl_blk,0xffff);

        print_nftl_zone(nftl_blk->nftl_zone);

        nftl_blk = nftl_blk->nftl_blk_next;
        //mutex_unlock(nftl_blk->blk_lock);
    }
    return 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
void nand_shutdown(struct platform_device *plat_dev)
{
    struct nand_blk_dev *dev;
    struct nand_blk_ops *tr = &mytr;

    nand_dbg_err("[NAND]shutdown first\n");
    list_for_each_entry(dev, &tr->devs, list){
        while(blk_fetch_request(dev->rq) != NULL){
            nand_dbg_err("nand_shutdown wait dev %d\n",dev->devnum);
            set_current_state(TASK_INTERRUPTIBLE);
            schedule_timeout(HZ>>3);
        }
    }

    nand_dbg_err("[NAND]shutdown second\n");
    list_for_each_entry(dev, &tr->devs, list){
        while(blk_fetch_request(dev->rq) != NULL){
            nand_dbg_err("nand_shutdown wait dev %d\n",dev->devnum);
            set_current_state(TASK_INTERRUPTIBLE);
            schedule_timeout(HZ>>3);
        }
    }

    shutdown_flush_write_cache();
    NandHwShutDown();
    nand_dbg_err("[NAND]shutdown end\n");
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/

static const struct of_device_id of_nand_id = { .compatible = "allwinner,sun8iw10-nand", };

static struct platform_driver nand_driver = {
    .probe = nand_probe,
    .remove = nand_remove,
    .shutdown =  nand_shutdown,
    .suspend = nand_suspend,
    .resume = nand_resume,
    .driver = {
        .name = "sw_nand",
        .owner = THIS_MODULE,
        .of_match_table = &of_nand_id,
    }
};


static struct resource flash_resource = {
	.start		= 0,
	.end		= 1,
	.flags		= 0x1,
};

static struct platform_device nand_device = {
	.name		= "sw_nand",
	.id		= 33,
	.resource	= &flash_resource,
	.num_resources	= 1,
	.dev        =  {
		.release =  nand_release_dev,
	}
};



/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int __init nand_init(void)
{
	int ret;
//    int nand0_used_flag;
    int nand_cache_level = 0;
    int nand_capacity_level = 0;
    int nand_flush_cache_num = 8;
//    script_item_value_type_e  type;
 //   char * dev_name = "nand_dev";
	//int storage_type =0;	
	int dragonboard_flag = 0;
	uchar *data = kmalloc(0x400,GFP_KERNEL);

	exit_probe_flag = 0;

	platform_driver_register(&nand_driver);

	if(exit_probe_flag == 0)
	{
		nand_dbg_err("Failed to insmod nand!!!\n");
		return 0;
	}

	ret = of_property_read_u32(ndfc_dev->of_node, "nand0_cache_level", &nand_cache_level);
	if (ret) 
	{
		nand_dbg_err("Failed to get nand0_cache_level\n");
		nand_cache_level = 0;
	}
	else
	{
		if(nand_cache_level == 0x55aaaa55)
		{
			nand_dbg_err("nand0_cache_level is no used\n");
			nand_cache_level = 0;
		}
	}
	
	ret = of_property_read_u32(ndfc_dev->of_node, "nand0_flush_cache_num", &nand_flush_cache_num);
	if (ret) 
	{
		nand_dbg_err("Failed to get nand_flush_cache_num\n");
		nand_flush_cache_num = 8;
	}
	else
	{
		if(nand_flush_cache_num == 0x55aaaa55)
		{
			nand_dbg_err("nand_flush_cache_num is no used\n");
			nand_flush_cache_num = 8;
		}
	}
	
	ret = of_property_read_u32(ndfc_dev->of_node, "nand0_capacity_level", &nand_capacity_level);
	if (ret) 
	{
		nand_dbg_err("Failed to get nand_capacity_level\n");
		nand_capacity_level = 0;
	}
	else
	{
		if(nand_capacity_level == 0x55aaaa55)
		{
			nand_dbg_err("nand_capacity_level is no used\n");
			nand_capacity_level = 0;
		}
	}
	
#if 0
    //get card_line
    type = script_get_item("nand0_para", "nand0_used", &nand0_used_flag);
    if(SCIRPT_ITEM_VALUE_TYPE_INT != type)
    {
        nand_dbg_err("nand type err! %d",type);
    }
    nand_dbg_err("nand init start, nand0_used_flag is %d\n", nand0_used_flag.val);

    nand_cache_level.val = 0;
    type = script_get_item("nand0_para", "nand_cache_level", &nand_cache_level);
    if(SCIRPT_ITEM_VALUE_TYPE_INT != type)
    {
        NAND_Print_DBG("nand_cache_level err! %d",type);
        nand_cache_level.val = 0;
    }

    nand_flush_cache_num.val = 8;
    type = script_get_item("nand0_para", "nand_flush_cache_num", &nand_flush_cache_num);
    if(SCIRPT_ITEM_VALUE_TYPE_INT != type)
    {
        //nand_dbg_err("nand_flush_cache_num err! %d",type);
        nand_flush_cache_num.val = 8;
    }

    nand_capacity_level.val = 0;
    type = script_get_item("nand0_para", "nand_capacity_level", &nand_capacity_level);
    if(SCIRPT_ITEM_VALUE_TYPE_INT != type)
    {
        NAND_Print_DBG("nand_capacity_level err! %d\n",type);
        nand_capacity_level.val = 0;
    }

    flush_cache_num = nand_flush_cache_num.val;

    //nand_dbg_err("flush_cache_num ! %d",flush_cache_num);

    if(nand0_used_flag.val == 0)
    {
        nand_dbg_err("nand driver is disabled \n");
        return 0;
    }

#endif

	flush_cache_num = nand_flush_cache_num;

    //storage_type = NAND_get_storagetype();
    dragonboard_flag = NAND_Get_Dragonboard_Flag();

	if(0 == dragonboard_flag)
    {
	    nand_dbg_err("nand init start\n");

	    p_nand_info = NandHwInit();
		if(p_nand_info == NULL){
		    kfree(data);
			return EAGAIN;
		}

	    set_cache_level(p_nand_info,nand_cache_level);
	    set_capacity_level(p_nand_info,nand_capacity_level);
		ret = nand_info_init(p_nand_info,0,8,NULL);
		kfree(data);
	    if(ret != 0)
	    {
	        nand_dbg_err("nand_info_init error \n");
	        return ret;
	    }

	    init_blklayer();
    }
	else
	{
		nand_dbg_err("dragonboard_flag=%d,run nand test for dragonboard\n",dragonboard_flag);
		init_blklayer_for_dragonboard();
	}

    nand_dbg_err("nand init end \n");
    return 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
void __exit nand_exit(void)
{
#if 0
	script_item_u   nand0_used_flag;
    script_item_value_type_e  type;

//	platform_driver_unregister(&nand_driver);

    type = script_get_item("nand0_para", "nand0_used", &nand0_used_flag);
    if(SCIRPT_ITEM_VALUE_TYPE_INT != type)
    nand_dbg_err("nand type err!");
    nand_dbg_err("nand0_used_flag is %d\n", nand0_used_flag.val);

    if(nand0_used_flag.val == 0)
    {
        nand_dbg_err("nand driver is disabled \n");
    }
#endif
    exit_blklayer();

	platform_device_unregister(&nand_device);

#if 0
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&early_suspend);
#endif
#endif
//  kobject_del(&kobj);
//  kobject_put(&kobj);
}

//module_init(nand_init);
//module_exit(nand_exit);
MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("nand flash groups");
MODULE_DESCRIPTION ("Generic NAND flash driver code");
