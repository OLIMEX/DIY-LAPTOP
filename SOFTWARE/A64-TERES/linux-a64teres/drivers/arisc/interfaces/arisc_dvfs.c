/*
 *  drivers/arisc/interfaces/arisc_dvfs.c
 *
 * Copyright (c) 2012 Allwinner.
 * 2012-05-01 Written by sunny (sunny@allwinnertech.com).
 * 2012-10-01 Written by superm (superm@allwinnertech.com).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "../arisc_i.h"

/*
 * set specific pll target frequency.
 * @freq:    target frequency to be set, based on KHZ;
 * @pll:     which pll will be set
 * @mode:    the attribute of message, whether syn or asyn;
 * @cb:      callback handler;
 * @cb_arg:  callback handler arguments;
 *
 * return: result, 0 - set frequency successed,
 *                !0 - set frequency failed;
 */
int arisc_dvfs_set_cpufreq(unsigned int freq, unsigned int pll, unsigned int mode, arisc_cb_t cb, void *cb_arg)
{
	int                   ret = 0;

	ARISC_INF("arisc dvfs request : %d\n", freq);
	ret = invoke_scp_fn_smc(ARM_SVC_ARISC_CPUX_DVFS_REQ, (u64)freq, (u64)pll, (u64)mode);

	if (ret) {
		if (cb == NULL) {
			ARISC_WRN("callback not install\n");
		} else {
			/* call callback function */
			ARISC_WRN("call the callback function\n");
			(*(cb))(cb_arg);
		}
	}

	return ret;
}
EXPORT_SYMBOL(arisc_dvfs_set_cpufreq);
