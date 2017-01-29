#include <linux/ion.h>	//for all "ion api"
#include <linux/ion_sunxi.h>	//for import global variable "sunxi_ion_client_create"
#include <linux/dma-mapping.h>	//just include"PAGE_SIZE" macro


/*your data set may look like this*/
struct ion_facade
{
	struct ion_client *client;
	struct ion_handle *handle;
	ion_phys_addr_t phyical_address;
	void* virtual_address;
	size_t address_length;
};

#define ION_KERNEL_USER_ERR(str)	pr_err("%s failed!! %s %s %d" , #str , __FILE__ , __func__ , __LINE__ )

/* 
do alloc  , 
	you alloc ion  memory function may looks like it
*/
static int your_module_alloc(struct ion_facade *ionf , int len)
{
	int ret;
/*here, create a client for your module.*/
	ionf->client = sunxi_ion_client_create("define-your-module-name");
	if (IS_ERR(ionf->client))
	{
		ION_KERNEL_USER_ERR( ion_client_create );
		goto err_client;
	}
	
/*here ,you chose CMA heap , no need to cache.*/
	ionf->handle = ion_alloc( ionf->client, len, PAGE_SIZE,  
		ION_HEAP_TYPE_CARVEOUT|ION_HEAP_TYPE_DMA_MASK , 0 );
	if (IS_ERR(ionf->handle))
	{
		ION_KERNEL_USER_ERR( ion_alloc );
		goto err_alloc;
	}	 
/*here, we map to virtual address for kernel.*/
	ionf->virtual_address = ion_map_kernel( ionf->client, ionf->handle);
	if (IS_ERR(ionf->virtual_address))
	{
		ION_KERNEL_USER_ERR( ion_map_kernel );
		goto err_map_kernel;
	}
/*here , we get the phyical address for special usage.*/
	ret = ion_phys( ionf->client,  ionf->handle, &ionf->phyical_address , &ionf->address_length );
	if( ret )
	{
		ION_KERNEL_USER_ERR( ion_phys );
		goto err_phys;
	}
	
	return 0;

/*got to err process*/
err_phys:	
	ion_unmap_kernel( ionf->client,  ionf->handle);
err_map_kernel:
	ion_free( ionf->client , ionf->handle );
err_alloc:
	ion_client_destroy(ionf->client);
err_client:

	return -1;

}

/* 
do free  , 
you free ion  memory function may looks like it
*/
static void your_module_free(struct ion_facade *ionf)
{
/*check */
	if ( IS_ERR(ionf->client) || IS_ERR(ionf->handle) || IS_ERR(ionf->virtual_address))
		return ;
/*do reverse functions*/
	ion_unmap_kernel( ionf->client,  ionf->handle);
	ion_free( ionf->client , ionf->handle );
	ion_client_destroy(ionf->client);

/*clear your state here*/
	ionf->client = (struct ion_client *)0;
	ionf->handle = (struct ion_handle *)0;
	ionf->phyical_address = 0;
	ionf->virtual_address = (void*)0;
	ionf->address_length = 0;
}

/*test kernel ion apis*/
#if 1
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>


#define M(x)	(x<<20)
static int test_len = M(4);
module_param( test_len , int,0644);

static struct ion_facade my_ion_param;

static int __init ion_use_demo_init(void)
{
	return your_module_alloc(&my_ion_param , test_len);
}

static void __exit ion_use_demo_exit(void)
{
	return your_module_free(&my_ion_param);
}

module_init(ion_use_demo_init);
module_exit(ion_use_demo_exit);
MODULE_LICENSE("GPL");
#endif
