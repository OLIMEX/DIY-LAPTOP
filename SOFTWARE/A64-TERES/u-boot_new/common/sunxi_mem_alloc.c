/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>

struct alloc_struct_t
{
    ulong address;                      //申请内存的地址
    ulong size;                         //分配的内存大小，用户实际得到的内存大小
    ulong o_size;                       //用户申请的内存大小
    struct alloc_struct_t *next;
};

#define SUNXI_BYTE_ALIGN(x)            ( ( (x + 31) >> 5) << 5)             /* alloc based on 1k byte */

static struct alloc_struct_t boot_heap_head, boot_heap_tail;
/*
*********************************************************************************************************
*                       CREATE HEAP
*
* Description: create heap.
*
* Aguments   : pHeapHead    heap start address.
*              nHeapSize    heap size.
*
* Returns    : EPDK_OK/EPDK_FAIL.
*********************************************************************************************************
*/

void mem_noncache_malloc_init(uint noncache_start, uint noncache_size)
{
    boot_heap_head.size    = boot_heap_tail.size = 0;
    boot_heap_head.address = noncache_start;
    boot_heap_tail.address = noncache_start + noncache_size;
    boot_heap_head.next    = &boot_heap_tail;
    boot_heap_tail.next    = 0;

    return ;
}

/*
*********************************************************************************************************
*                       MALLOC BUFFER FROM HEAP
*
* Description: malloc a buffer from heap.
*
* Aguments   : num_bytes    the size of the buffer need malloc;
*
* Returns    : the po__s32er to buffer has malloc.
*********************************************************************************************************
*/
void *malloc_noncache(uint num_bytes)
{
    struct alloc_struct_t *ptr, *newptr;
    __u32  actual_bytes;

    if (!num_bytes) return NULL;

    actual_bytes = SUNXI_BYTE_ALIGN(num_bytes);    /* translate the byte count to size of long type       */

    ptr = &boot_heap_head;                      /* scan from the boot_heap_head of the heap            */

    while (ptr && ptr->next)                    /* look for enough memory for alloc                    */
    {
        if (ptr->next->address >= (ptr->address + ptr->size +                                          \
                2 * sizeof(struct alloc_struct_t) + actual_bytes))
        {
            break;
        }
                                                /* find enough memory to alloc                         */
        ptr = ptr->next;
    }

    if (!ptr->next)
    {
        return 0;                   /* it has reached the boot_heap_tail of the heap now              */
    }

    newptr = (struct alloc_struct_t *)(ptr->address + ptr->size);
                                                /* create a new node for the memory block             */
    if (!newptr)
    {
        return 0;                               /* create the node failed, can't manage the block     */
    }

    /* set the memory block chain, insert the node to the chain */
    newptr->address = ptr->address + ptr->size + sizeof(struct alloc_struct_t);
    newptr->size    = actual_bytes;
    newptr->o_size  = num_bytes;
    newptr->next    = ptr->next;
    ptr->next       = newptr;

    return (void *)newptr->address;
}
/*
*********************************************************************************************************
*                       FREE BUFFER TO HEAP
*
* Description: free buffer to heap
*
* Aguments   : p    the po__s32er to the buffer which need be free.
*
* Returns    : none
*********************************************************************************************************
*/
void  free_noncache(void *p)
{
    struct alloc_struct_t *ptr, *prev;

	if( p == NULL )
		return;

    ptr = &boot_heap_head;                /* look for the node which po__s32 this memory block                     */
    while (ptr && ptr->next)
    {
        if (ptr->next->address == (ulong)p)
            break;              /* find the node which need to be release                              */
        ptr = ptr->next;
    }

	prev = ptr;
	ptr = ptr->next;

    if (!ptr) return;           /* the node is heap boot_heap_tail                                               */

    prev->next = ptr->next;     /* delete the node which need be released from the memory block chain  */

    return ;
}
