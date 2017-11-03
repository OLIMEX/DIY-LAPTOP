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
#include <config.h>
#include <common.h>
#include <malloc.h>
#include <asm/arch/queue.h>
/*
************************************************************************************************************
*
*                                             function
*
*    name          :  initqueue
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int initqueue(queue *q, int each_size, int buffer_count)
{
	int i;

    if(!q)
    {
    	printf("initqueue: queue element is null\n");

    	return -1;
    }
	memset(q, 0, sizeof(queue));
    q->count = buffer_count;    //多使用一个buffer
    q->size  = each_size;
    q->base_addr = (void *)malloc(buffer_count * each_size * 2);
    if(!q->base_addr)
    {
    	printf("create buffer memory failed\n");

    	return -2;
    }
    q->front = q->rear = 0;
    //init buffer queue
    q->element[0].data = (char *)q->base_addr + each_size;
    for(i=1;i<buffer_count;i++)
    {
    	q->element[i].data = q->element[i-1].data + each_size * 2;
    	q->element[i].len  = 0;
    }

    return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :  destroyqueue
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int destroyqueue(queue *q)
{
    if(!q)
    {
    	printf("destroyqueue: queue element is null\n");

    	return -1;
    }
	free(q->base_addr);

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :  resetqueue
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
void resetqueue(queue *q)
{
	int i;

	q->front = q->rear = 0;
	q->element[0].data = (char *)q->base_addr + q->size;
    for(i=1;i<q->count;i++)
    {
    	q->element[i].data = q->element[i-1].data + q->size * 2;
    	q->element[i].len  = 0;
    }
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :  enqueue
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
void enqueue(queue *q, int element)
{
	;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :  outqueue
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
void outqueue(queue *q, int *element)
{
	;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :  isqueueempty
*
*    parmeters     :
*
*    return        :
*
*    note          :  采用保留一个buffer不用的办法，当头尾指针指向同一个buffer时，表示queue为空
*
*
************************************************************************************************************
*/
int isqueueempty(queue *q)
{
	return (q->front == q->rear)? 1:0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :  isqueueempty
*
*    parmeters     :
*
*    return        :
*
*    note          :  采用保留一个buffer不用的办法，当头尾指针增加1即等于头指针时，表示queue为满
*
*
************************************************************************************************************
*/
int isqueuefull(queue *q)
{
	int tmp_rear;

	tmp_rear = q->rear + 1;
	if(tmp_rear > q->count)
	{
		tmp_rear = 0;
	}
	return (q->front == tmp_rear)? 1:0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :  inqueue_query
*
*    parmeters     :
*
*    return        :
*
*    note          :  尝试从队列中取出一个buffer，写入数据到buffer中
*                     当取出成功，rear指针不变
*
************************************************************************************************************
*/
int inqueue_query(queue *q, queue_data *qdata)
{
	int   tmp_rear;

	tmp_rear = q->rear + 1;
	if(tmp_rear > q->count)
	{
		tmp_rear = 0;
	}
	if(q->front == tmp_rear)   //满足时，queue满
	{
		//无法使用rear处buffer
		return -1;
	}

    qdata = &q->element[q->rear];

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :  inqueue_ex
*
*    parmeters     :
*
*    return        :
*
*    note          :  把pickqueue_ex归还到buffer队列中，rear指针加1
*
*
*
************************************************************************************************************
*/
int inqueue_ex(queue *q)
{
	int tmp_rear;

	tmp_rear = q->rear + 1;
	if(tmp_rear > q->count)
	{
		tmp_rear = 0;
	}
	if(q->front == tmp_rear)   //队列已满
	{
		//无法使用rear处buffer
		return -1;
	}
	q->rear = tmp_rear;

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :  outqueue_query
*
*    parmeters     :
*
*    return        :
*
*    note          :  查询当前front处buffer是否可用，当可用时取出，front指针不变
*
*
************************************************************************************************************
*/
int outqueue_query(queue *q, queue_data *qdata, queue_data *next_qdata)
{
	int next;

	if(q->front == q->rear)   //满足时，queue空
	{
		//无法使用front处buffer
		return -1;
	}
	next = q->front + 1;
	if(next > q->count)
	{
		next = 0;
	}
    qdata  = &q->element[q->front];
    next_qdata = &q->element[next];

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :  outqueue
*
*    parmeters     :
*
*    return        :
*
*    note          : 把queryqueue_ex归还到buffer队列中，front指针加1
*
*
************************************************************************************************************
*/
int outqueue_ex(queue *q)
{
	int tmp_front;

	if(q->front == q->rear)   //满足时，queue空，无法退出当前buffer
	{
		return -1;
	}
	tmp_front = q->front + 1;
	if(tmp_front > q->count)
	{
		tmp_front = 0;
	}
	q->front = tmp_front;

    return 0;
}

