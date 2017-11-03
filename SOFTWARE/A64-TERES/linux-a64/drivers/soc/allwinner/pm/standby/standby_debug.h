#ifndef __STANDBY_DEBUG_
#define __STANDBY_DEBUG_

#define STANDBY_START	               (0x00)
#define RESUME0_START		       (0x20)
#define RESUME1_START		       (0x40)

#define SHUTDOWN_FLOW		       (0x10)
#define OTA_FLOW		       (0x30)

void mem_status_init_nommu(void);
void mem_status_clear(void);
void mem_status_exit(void);
void save_irq_status(volatile __u32 val);
void save_mem_status(volatile __u32 val);
__u32 get_mem_status(void);
void parse_status_code(__u32 code, __u32 index);
void show_mem_status(void);

#endif /* __STANDBY_DEBUG_ */

