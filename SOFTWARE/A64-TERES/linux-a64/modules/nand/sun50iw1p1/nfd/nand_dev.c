
#define _NAND_DEV_C_

#include "nand_blk.h"
#include "nand_dev.h"


#define NAND_SCHEDULE_TIMEOUT  (HZ >> 3)
#define NFTL_SCHEDULE_TIMEOUT  (HZ >> 2)
#define NFTL_FLUSH_DATA_TIME	 1
#define WEAR_LEVELING 1

unsigned int flush_cache_num = 32;
static int dev_num = 0;

unsigned long nand_active_time = 0;

struct nand_kobject* s_nand_kobj;
struct _nand_info* p_nand_info = NULL;
extern struct nand_blk_ops mytr;
extern struct kobj_type ktype;

extern struct _nand_partition* build_nand_partition(struct _nand_phy_partition* phy_partition);
extern void add_nftl_blk_list(struct _nftl_blk*head,struct _nftl_blk *nftl_blk);
extern struct _nftl_blk* del_last_nftl_blk(struct _nftl_blk*head);
extern int add_nand_blktrans_dev(struct nand_blk_dev *dev);
extern int add_nand_blktrans_dev_for_dragonboard(struct nand_blk_dev *dev);
extern int del_nand_blktrans_dev(struct nand_blk_dev *dev);
extern struct _nand_disk* get_disk_from_phy_partition(struct _nand_phy_partition* phy_partition);
extern uint16 get_partitionNO(struct _nand_phy_partition* phy_partition);
extern int nftl_exit(struct _nftl_blk *nftl_blk);
extern int NAND_Print_DBG(const char *fmt, ...);
extern struct _nftl_blk* get_nftl_need_read_claim(struct _nftl_blk* start_blk);
extern void set_nftl_read_claim_complete(struct _nftl_blk* nftl_blk);
extern unsigned int get_blk_logic_page_num(struct _nftl_blk *nftl_blk);
extern  int read_reclaim(void *zone,uchar*buf,uint32 start_page,uint32 total_pages);


int _dev_nand_read(struct _nand_dev *nand_dev,__u32 start_sector,__u32 len,unsigned char *buf);
int _dev_nand_write(struct _nand_dev *nand_dev,__u32 start_sector,__u32 len,unsigned char *buf);
int _dev_nand_discard(struct _nand_dev *nand_dev,__u32 start_sector,__u32 len);
int _dev_flush_write_cache(struct _nand_dev *nand_dev,__u32 num);
int _dev_flush_sector_write_cache(struct _nand_dev *nand_dev, __u32 num);
int dev_initialize(struct _nand_dev *nand_dev,struct _nftl_blk *nftl_blk,__u32 offset,__u32 size);
int nand_flush(struct nand_blk_dev *dev);
int add_nand(struct nand_blk_ops *tr, struct _nand_phy_partition* phy_partition);
int remove_nand(struct nand_blk_ops *tr);
unsigned int nand_read_reclaim(struct _nftl_blk *nftl_blk,unsigned char *buf);

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int nand_thread(void *arg)
{
    unsigned long time;
    struct nand_blk_ops* tr = (struct nand_blk_ops*)arg;

    unsigned int start_time = 480*10;
    unsigned char * temp_buf = kmalloc(64*1024, GFP_KERNEL);

    while (!kthread_should_stop())
    {
        if(start_time != 0)
        {
            start_time--;
            goto  nand_thread_exit;
        }

        time = jiffies;
        if (time_after(time, nand_active_time+HZ))
        {
            nand_read_reclaim(tr->nftl_blk_head.nftl_blk_next,temp_buf);
        }

nand_thread_exit:
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(NAND_SCHEDULE_TIMEOUT);
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
/*
static int nftl_thread(void *arg)
{
    struct _nftl_blk *nftl_blk = arg;
    unsigned long time;

    nftl_blk->time_flush = NFTL_FLUSH_DATA_TIME * HZ;
    nftl_blk->time_flush = HZ;

    while (!kthread_should_stop()) {

        mutex_lock(nftl_blk->blk_lock);

        if(nftl_get_zone_write_cache_nums(nftl_blk->nftl_zone) > 0){
            time = jiffies;
           if (time_after(time,nftl_blk->time + nftl_blk->time_flush+HZ*4)){
                nftl_blk->flush_write_cache(nftl_blk,16);
           }
        }

//#if  SUPPORT_WEAR_LEVELING
//        if(do_static_wear_leveling(nftl_blk->nftl_zone) != 0){
//            nand_dbg_err("nftl_thread do_static_wear_leveling error!\n");
//        }
//#endif

        if(garbage_collect(nftl_blk->nftl_zone) != 0){
            nand_dbg_err("nftl_thread garbage_collect error!\n");
        }

        if(do_prio_gc(nftl_blk->nftl_zone) != 0){
            nand_dbg_err("nftl_thread do_prio_gc error!\n");
        }

        mutex_unlock(nftl_blk->blk_lock);
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(NFTL_SCHEDULE_TIMEOUT);
    }

    nftl_blk->nftl_thread = (void*)NULL;
    return 0;
}
*/

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
static int nftl_thread(void *arg)
{
    struct _nftl_blk *nftl_blk = arg;
    unsigned long time;

	unsigned long swl_time = jiffies;
	int first_miss_swl = 0;
	int need_swl = 0;
	int swl_done = 0;
    nftl_blk->time_flush = 2*HZ;

    while (!kthread_should_stop())
    {
        mutex_lock(nftl_blk->blk_lock);

        time = jiffies;
        if(nftl_get_zone_write_cache_nums(nftl_blk->nftl_zone) > 64)
        {
           //if (time_after(time,nftl_blk->time + nftl_blk->time_flush))
           {
                nftl_blk->flush_write_cache(nftl_blk,16);
           }
        }
//        else if(nftl_get_zone_write_cache_nums(nftl_blk->nftl_zone) > 200)
//        {
//           time = jiffies;
//           if (time_after(time,nftl_blk->time + nftl_blk->time_flush)){
//                nftl_blk->flush_write_cache(nftl_blk,6);
//           }
//        }
//        else if(nftl_get_zone_write_cache_nums(nftl_blk->nftl_zone) > 100)
//        {
//            time = jiffies;
//           if (time_after(time,nftl_blk->time + nftl_blk->time_flush)){
//                nftl_blk->flush_write_cache(nftl_blk,8);
//           }
//        }
        else
        {
           if (time_after((unsigned long)time,(unsigned long)(nftl_blk->time + nftl_blk->time_flush)) != 0)
           {
                nftl_blk->flush_write_cache(nftl_blk,flush_cache_num);
           }
        }

#if WEAR_LEVELING
		/**
        if (time_after(time,(HZ<<4)+nftl_blk->time))
        {
            if(do_static_wear_leveling(nftl_blk->nftl_zone) != 0)
            {
                nand_dbg_err("nftl_thread do_static_wear_leveling error!\n");
            }
        }
        */

        /** add static WL:
         * we do the static WL when nftl ops is idle over 4s;
         * if missing the chance over 64s, we will do it mandatorily!!!
         * if we have done the static WL successfully, the time interval
         * to next static WL must be over 64s.
		 */
		time = jiffies;
		if (swl_done) {
			if (time_after(time, swl_time + (HZ << 6))) {
        		//nand_dbg_err("[ND]swl: over time(64s) after last swl done\n");
        		need_swl = 1;	/* next static WL is over 64s */
			}
		} else if (time_after(time, (unsigned long)(nftl_blk->ops_time + (HZ << 2)))) {
			//nand_dbg_err("[ND]swl: nftl ops is idle over 4s\n");
			need_swl = 1;	/* nftl ops is idle over 4s */
		} else if (first_miss_swl) {
			if (time_after(time, swl_time + (HZ << 6))) {
        		//nand_dbg_err("[ND]swl: over time(64s) after missing swl\n");
        		need_swl = 1;	/* next static WL is over 64s */
			}
        } else {
			first_miss_swl = 1;
			swl_time = jiffies;
			//nand_dbg_err("[ND]swl: miss swl set time\n");
        }

    	if (need_swl) {
			first_miss_swl = 0;
			need_swl = 0;

            if(!(swl_done = do_static_wear_leveling(nftl_blk->nftl_zone))){
                //nand_dbg_err("[ND]swl: nftl_thread swl ok!\n");
				swl_done = 1;
				swl_time = jiffies;
				//nand_dbg_err("[ND]swl: done swl set time\n");
            } else {
            	//nand_dbg_err("[ND]swl: nftl_thread swl fail(%i)!\n", swl_done);
				swl_done = 0;
            }
        }
#endif

        if(garbage_collect(nftl_blk->nftl_zone) != 0)
        {
            nand_dbg_err("nftl_thread garbage_collect error!\n");
        }

        if(do_prio_gc(nftl_blk->nftl_zone) != 0)
        {
            nand_dbg_err("nftl_thread do_prio_gc error!\n");
        }

        mutex_unlock(nftl_blk->blk_lock);

        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(NFTL_SCHEDULE_TIMEOUT);
    }

    nftl_blk->nftl_thread = (void*)NULL;
    return 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
void add_nand_dev_list(struct _nand_dev*head,struct _nand_dev *nand_dev)
{
    struct _nand_dev * p = head;

    nand_dev->nand_dev_next = NULL;
    while(p->nand_dev_next != NULL)
    {
        p = p->nand_dev_next;
    }
    p->nand_dev_next = nand_dev;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
struct _nand_dev* del_last_nand_dev(struct _nand_dev*head)
{
    struct _nand_dev *nand_dev = NULL;
    struct _nand_dev * p = head;
    while(p->nand_dev_next != NULL)
    {
        nand_dev = p->nand_dev_next;
        if(nand_dev->nand_dev_next == NULL)
        {
            p->nand_dev_next = NULL;
            return nand_dev;
        }
        p = p->nand_dev_next;
    }
    return NULL;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int add_nand(struct nand_blk_ops *tr, struct _nand_phy_partition* phy_partition)
{
	int i;
    __u32 cur_offset = 0;
    struct _nftl_blk *nftl_blk;
    struct _nand_dev *nand_dev;
	struct _nand_disk* disk;
	struct _nand_disk* head_disk;
	//struct nand_kobject* nand_kobj;
	uint16 PartitionNO;

    PartitionNO = get_partitionNO(phy_partition);

    nftl_blk = kmalloc(sizeof(struct _nftl_blk), GFP_KERNEL);
    if (!nftl_blk)
	{
	    nand_dbg_err("init kmalloc fail 3!\n");
		return 1;
	}
    nftl_blk->nand = build_nand_partition(phy_partition);

    if (nftl_initialize(nftl_blk,PartitionNO))
	{
        nand_dbg_err("nftl_initialize failed\n");
        return 1;
    }

    nftl_blk->blk_lock = kmalloc(sizeof(struct mutex), GFP_KERNEL);
    if (!nftl_blk->blk_lock)
	{
	    nand_dbg_err("init kmalloc fail 2!\n");
		return 1;
	}
    mutex_init(nftl_blk->blk_lock);

	nftl_blk->nftl_thread = kthread_run(nftl_thread, nftl_blk, "%sd", "nftl");
    if (IS_ERR(nftl_blk->nftl_thread))
    {
        nand_dbg_err("init kthread_run fail!\n");
        return 1;
    }

    add_nftl_blk_list(&tr->nftl_blk_head,nftl_blk);

    s_nand_kobj = kzalloc(sizeof(struct nand_kobject), GFP_KERNEL);
    if (!s_nand_kobj)
	{
	    nand_dbg_err("init kmalloc fail 1!\n");
		return 1;
	}
	s_nand_kobj->nftl_blk = nftl_blk;
    if(kobject_init_and_add(&s_nand_kobj->kobj,&ktype,NULL,"nand_driver%d",PartitionNO) != 0 ) {
	    nand_dbg_err("init nand sysfs fail!\n");
		return 1;
	}

	disk = get_disk_from_phy_partition(phy_partition);
	for(i=0;i<MAX_PART_COUNT_PER_FTL;i++)
    {
        //nand_dbg_err("disk->name %s\n",(char *)(disk->name));
        //nand_dbg_err("disk->type %x\n",disk[i].type);
        //nand_dbg_err("disk->size %x\n",disk[i].size);
    }

    head_disk = get_disk_from_phy_partition(phy_partition);
	for(i=0;i<MAX_PART_COUNT_PER_FTL;i++)
	{
		disk = head_disk + i;
		if(disk->type == 0xffffffff)
		{
		    break;
		}

        nand_dev = kmalloc(sizeof(struct _nand_dev), GFP_KERNEL);
        if (!nand_dev)
		{
		    nand_dbg_err("init kmalloc fail!\n");
            return 1;
		}

		add_nand_dev_list(&tr->nand_dev_head,nand_dev);

        nand_dev->nbd.nandr = &mytr;

        if(dev_initialize(nand_dev,nftl_blk,cur_offset,disk->size) != 0)
		{
            //nand_dbg_err("dev_initialize failed\n");
            return 1;
        }

        nand_dev->nbd.size = (unsigned int)nand_dev->size;
        nand_dev->nbd.priv = (void*)nand_dev;

//        if(disk->size == 0xffffffff)
//        {
//            nand_dbg_err("user size1: %d!\n",nand_dev->nbd.size);
//            nand_dev->nbd.size += nand_dev->nbd.size >> 2;
//            nand_dbg_err("user size2: %d!\n",nand_dev->nbd.size);
//        }

		memcpy(nand_dev->nbd.name,disk->name,strlen(disk->name)+1);
        memcpy(nand_dev->name,disk->name,strlen(disk->name)+1);
        NAND_Print_DBG("nand_dev add %s\n",nand_dev->name);

        if((PartitionNO == 0) && (i==0))
        {
            dev_num = -1;
        }
        else
        {
            dev_num++;
            nand_dev->nbd.devnum = dev_num;
            if (add_nand_blktrans_dev(&nand_dev->nbd))
		    {
                nand_dbg_err("nftl add blk disk dev failed\n");
                return 1;
            }
        }

		cur_offset += disk->size;
	}
    return 0;
}

int add_nand_for_dragonboard_test(struct nand_blk_ops *tr)
{
    struct _nand_dev *nand_dev;

    nand_dev = kmalloc(sizeof(struct _nand_dev), GFP_KERNEL);
    if (!nand_dev)
	{
	    nand_dbg_err("init kmalloc fail!\n");
        return 1;
	}

	add_nand_dev_list(&tr->nand_dev_head,nand_dev);

    nand_dev->nbd.nandr = &mytr;

    nand_dev->nbd.size = 1024*4096;
    nand_dev->nbd.priv = (void*)nand_dev;
	
    dev_num = 0;
    nand_dev->nbd.devnum = dev_num;
	printk("befor add nand blktrans dev\n");
	if (add_nand_blktrans_dev_for_dragonboard(&nand_dev->nbd))
    {
        nand_dbg_err("nftl add blk disk dev failed\n");
        return 1;
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
int remove_nand(struct nand_blk_ops *tr)
{
//	struct nand_kobject* nand_kobj;
    struct _nftl_blk *nftl_blk;
    struct _nand_dev *nand_dev;

    nand_dbg_err("remove_nand\n");

    nftl_blk = &tr->nftl_blk_head;
    nand_dev = &tr->nand_dev_head;

    nand_dev = del_last_nand_dev(&tr->nand_dev_head);
    while(nand_dev != NULL)
    {
        nand_flush(&nand_dev->nbd);
        del_nand_blktrans_dev(&nand_dev->nbd);
        kfree(nand_dev);
        nand_dev = del_last_nand_dev(&tr->nand_dev_head);
    }

    nftl_blk = del_last_nftl_blk(&tr->nftl_blk_head);
    while(nftl_blk != NULL)
    {
        if(nftl_blk->nftl_thread!=NULL)
        {
            kthread_stop(nftl_blk->nftl_thread);
            nftl_blk->nftl_thread=NULL;
        }

//        nand_kobj = container_of(&nftl_blk, struct nand_kobject, nftl_blk);
//        kobject_del(&nand_kobj->kobj);
//        kobject_put(&nand_kobj->kobj);

        kfree(nftl_blk->blk_lock);
        nftl_exit(nftl_blk);
        kfree(nftl_blk);
        nftl_blk = del_last_nftl_blk(&tr->nftl_blk_head);
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
int nand_flush(struct nand_blk_dev *dev)
{
    int error = 0;
    struct _nand_dev *nand_dev = (struct _nand_dev *)(dev->priv);

    //error = nand_dev->flush_sector_write_cache(nand_dev,0);
    error = nand_dev->flush_write_cache(nand_dev,0xffff);

    return error;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int dev_initialize(struct _nand_dev *nand_dev,struct _nftl_blk *nftl_blk,__u32 offset,__u32 size)
{
    __u32 offset_t,size_t;

    offset_t = offset;
    size_t = size;

	nand_dev->nftl_blk = nftl_blk;

	if(offset_t < nftl_blk->nftl_logic_size)
	{
	    nand_dev->offset = offset_t;
	}
	else
	{
	    //nand_dbg_err("dev_initialize %x  %x \n",offset_t,nftl_blk->nftl_logic_size);
	    return 1;
	}

	if((nand_dev->offset + size_t) <= nftl_blk->nftl_logic_size)
	{
	    nand_dev->size = size_t;
	}
	else
	{
	    nand_dev->size = nftl_blk->nftl_logic_size - nand_dev->offset;
	}

	nand_dev->read_data = _dev_nand_read;
	nand_dev->write_data = _dev_nand_write;
	nand_dev->flush_write_cache = _dev_flush_write_cache;
	nand_dev->flush_sector_write_cache = _dev_flush_sector_write_cache;
	nand_dev->discard = _dev_nand_discard;

	//nand_dbg_err("dev_initialize 0x%x \n",nand_dev->size);

    return 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int _dev_nand_read(struct _nand_dev *nand_dev,__u32 start_sector,__u32 len,unsigned char *buf)
{
    int ret;
    struct _nftl_blk *nftl_blk = nand_dev->nftl_blk;

    nand_active_time = jiffies;

    mutex_lock(nftl_blk->blk_lock);

    if(start_sector+len >nand_dev->size)
    {
        ret = 0xfffff;
        nand_dbg_err("_dev_nand_read over size 0x%x 0x%x\n",start_sector,nand_dev->size);
        while(--ret);
        ret = -1;
        goto _dev_nand_read_end;

    }

    ret =  nand_dev->nftl_blk->read_data(nand_dev->nftl_blk,start_sector + nand_dev->offset,len,buf);

_dev_nand_read_end:

    nand_active_time = jiffies;

    mutex_unlock(nftl_blk->blk_lock);

    return ret;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int _dev_nand_write(struct _nand_dev *nand_dev,__u32 start_sector,__u32 len,unsigned char *buf)
{
    __u32 ret;
    struct _nftl_blk *nftl_blk = nand_dev->nftl_blk;

    nand_active_time = jiffies;

    mutex_lock(nftl_blk->blk_lock);

    if(start_sector+len >nand_dev->size)
    {
        ret = 0xffffff;
        nand_dbg_err("_dev_nand_write over size 0x%x 0x%x\n",start_sector,nand_dev->size);
        while(--ret);
        ret = -1;
        goto _dev_nand_write_end;
    }

    ret = nand_dev->nftl_blk->write_data(nand_dev->nftl_blk,start_sector + nand_dev->offset,len,buf);

_dev_nand_write_end:

    nftl_blk->time = jiffies;
    nand_active_time = jiffies;

    mutex_unlock(nftl_blk->blk_lock);

    return ret;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int _dev_nand_discard(struct _nand_dev *nand_dev,__u32 start_sector,__u32 len)
{
    __u32 ret = 0;
    struct _nftl_blk *nftl_blk = nand_dev->nftl_blk;

    mutex_lock(nftl_blk->blk_lock);

    nand_dbg_err("==========nand_discard========== %d,%d\n",start_sector,len);

    ret = nand_dev->nftl_blk->discard(nand_dev->nftl_blk,start_sector + nand_dev->offset,len);

    nand_active_time = jiffies;

    mutex_unlock(nftl_blk->blk_lock);

    return ret;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int _dev_flush_write_cache(struct _nand_dev *nand_dev, __u32 num)
{
    __u32 ret;
    struct _nftl_blk *nftl_blk = nand_dev->nftl_blk;

    nand_active_time = jiffies;

    mutex_lock(nftl_blk->blk_lock);

    ret = nand_dev->nftl_blk->flush_write_cache(nand_dev->nftl_blk,num);

    //nftl_blk->time = jiffies;
    nand_active_time = jiffies;

    mutex_unlock(nftl_blk->blk_lock);

    return ret;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int _dev_flush_sector_write_cache(struct _nand_dev *nand_dev, __u32 num)
{
    __u32 ret;
    struct _nftl_blk *nftl_blk = nand_dev->nftl_blk;

    mutex_lock(nftl_blk->blk_lock);

    ret = nand_dev->nftl_blk->flush_sector_write_cache(nand_dev->nftl_blk,num);

    //nftl_blk->time = jiffies;

    mutex_unlock(nftl_blk->blk_lock);

    return ret;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
struct _nand_dev * _get_nand_dev_by_name(char * name)
{
    int len;
    struct _nand_dev * p;

    //nand_dbg_err("_get_nand_dev_by_name: %s \n",name);

    len = strlen(name)+1;

    if(len > 16)
    {
        len = 16;
    }

    for(p=&mytr.nand_dev_head; p!=NULL; p = p->nand_dev_next)
    {
        //nand_dbg_err("  %s %d\n",p->name,len);
        if( memcmp(p->name,name,len) == 0)
        {
            return p;
        }
    }
    return NULL;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int _dev_nand_read2(char * name,unsigned int start_sector,unsigned int len,unsigned char *buf)
{
    int ret;
    struct _nand_dev *nand_dev  = _get_nand_dev_by_name(name);

    if(nand_dev == NULL)
    {
        return -1;
    }

    if(start_sector+len > nand_dev->size)
    {
        nand_dbg_err("_dev_nand_read over size 0x%x 0x%x\n",start_sector,nand_dev->size);
        return -1;
    }

    //nand_dbg_err("_dev_nand_read2 %s \n",name);

    ret =  nand_dev->nftl_blk->read_data(nand_dev->nftl_blk,start_sector + nand_dev->offset,len,buf);
    nand_active_time = jiffies;

    return ret;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int _dev_nand_write2(char * name,unsigned int start_sector,unsigned int len,unsigned char *buf)
{
    int ret;
    struct _nand_dev *nand_dev = _get_nand_dev_by_name(name);

    if(nand_dev == NULL)
    {
        return -1;
    }

    if(start_sector+len > nand_dev->size)
    {
        nand_dbg_err("_dev_nand_write over size 0x%x 0x%x\n",start_sector,nand_dev->size);
        return -1;
    }

   //nand_dbg_err("_dev_nand_write2 %s \n",name);

    ret = nand_dev->nftl_blk->write_data(nand_dev->nftl_blk,start_sector + nand_dev->offset,len,buf);
    nand_active_time = jiffies;

    return ret;
}




/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
static uint32 st_start_page = 0;
uint32 nand_read_reclaim(struct _nftl_blk *start_blk,unsigned char *buf)
{
    uint32 total_pages,ret;
    struct _nftl_blk *nftl_blk;

    nftl_blk = get_nftl_need_read_claim(start_blk);
    if(nftl_blk == NULL)
    {
        return 0;
    }

    total_pages = get_blk_logic_page_num(nftl_blk);

    mutex_lock(nftl_blk->blk_lock);

    ret = read_reclaim(nftl_blk->nftl_zone,buf,st_start_page,total_pages);
    if(ret == 0xffffffff)
    {
        st_start_page = 0;
        set_nftl_read_claim_complete(nftl_blk);
    }
    else
    {
        st_start_page = ret;
    }

    mutex_unlock(nftl_blk->blk_lock);

    return 0;
}
