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


/* 有线性表的特性:分为链式队列与顺序队列
	顺序队列:用一段地址连续的存储单元存储数据元素，定义两个游标:指向队头
	的游标(front)、指向队尾的游标(rear),如果front == rear队列为空,如果
	(rear + 1) % MAXSIZE == front队列满(此为循环队列),如普通队列rear==MAXSIZE队列满
*/

#ifndef __QUEUE_H__
#define __QUEUE_H__


#define QUEUE_MAX_BUFFER_SIZE    64  /* max buffer count of queue  */


typedef struct
{
	char *data;
	uint  len;
}
queue_data;

typedef struct
{
	queue_data element[QUEUE_MAX_BUFFER_SIZE];
	int front;                     //head buffer of the queue
	int rear;                      //tail buffer of the queue
    int size;                      //the bytes of each buffer int the queue
    int count;                     //the total count of buffers in the queue
    void *base_addr;
}queue;

int  initqueue(queue *q, int each_size, int buffer_count);  //init queue

int  destroyqueue(queue *q);         //destroy queue

void resetqueue(queue *q);

int isqueueempty(queue *q);

int isqueuefull(queue *q);

int inqueue_query(queue *q, queue_data *qdata);

int inqueue_ex(queue *q);

int outqueue_query(queue *q, queue_data *qdata, queue_data *next_qdata);

int outqueue_ex(queue *q);



#endif //__QUEUE_H__


