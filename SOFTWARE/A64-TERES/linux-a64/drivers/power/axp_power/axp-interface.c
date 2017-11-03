#ifndef _AXP_SYS_H
#define _AXP_SYS_H

#include <linux/mfd/axp-mfd.h>

#if defined(CONFIG_AW_AXP81X)
#elif defined(CONFIG_AW_AXP)
s32 axp_usbcur(aw_charge_type type){ return 0; };
s32 axp_usbvol(aw_charge_type type){ return 0; };
s32 axp_usb_det(void){ return 0; };
s32 axp_powerkey_get(void){ return 0; };
void axp_powerkey_set(int value){};
u64 axp_read_power_sply(void){ return 0; };
s32 axp_read_bat_cap(void){return 100;};
s32 axp_read_ac_chg(void){ return 0; };
/* axp15 not define this function */
void axp_reg_debug(s32 reg, s32 len, u8 *val){return;};
#endif
#endif
