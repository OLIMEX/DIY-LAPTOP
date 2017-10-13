
#ifndef _EFEX_QUEUE_H_
#define _EFEX_QUEUE_H_

#include <common.h>

int efex_queue_init(void);
int efex_queue_exit(void);
int efex_queue_write_one_page( void );
int efex_queue_write_all_page( void );
int efex_save_buff_to_queue(uint flash_start, uint flash_sectors,void* buff);

#endif
