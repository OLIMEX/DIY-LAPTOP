/*
 * Based on arch/arm64/kernel/chipid-sunxi.c
 *
 * Copyright (C) 2015 Allwinnertech Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/sunxi-smc.h>

#ifdef CONFIG_ARM
/*cmd to call ATF service*/
#define ARM_SVC_EFUSE_PROBE_SECURE_ENABLE    (0x8000fe03)
#define ARM_SVC_READ_SEC_REG                 (0x8000ff05)
#define ARM_SVC_WRITE_SEC_REG                (0x8000ff06)
#endif

#ifdef CONFIG_ARM64
/*cmd to call ATF service*/
#define ARM_SVC_EFUSE_PROBE_SECURE_ENABLE    (0xc000fe03)
#define ARM_SVC_READ_SEC_REG                 (0xC000ff05)
#define ARM_SVC_WRITE_SEC_REG                (0xC000ff06)
#endif

/*interface for smc */
int sunxi_smc_readl(phys_addr_t addr)
{
	return invoke_smc_fn(ARM_SVC_READ_SEC_REG, addr, 0, 0);
}

int sunxi_smc_writel(u32 value, phys_addr_t addr)
{
	return invoke_smc_fn(ARM_SVC_WRITE_SEC_REG, addr, value, 0);
}

int sunxi_smc_probe_secure(void)
{
	return invoke_smc_fn(ARM_SVC_EFUSE_PROBE_SECURE_ENABLE,
			0, 0, 0);
}
