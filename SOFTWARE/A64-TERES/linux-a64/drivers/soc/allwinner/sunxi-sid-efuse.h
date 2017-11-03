/*
 * The key define in SUNXI Efuse.
 *
 * Copyright (C) 2016 Allwinner.
 *
 * Matteo <duanmintao@allwinnertech.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef __SUNXI_SID_EFUSE_H__
#define __SUNXI_SID_EFUSE_H__

#include <linux/sunxi-sid.h>

struct efuse_key_map {
	char name[SUNXI_KEY_NAME_LEN];
	int offset;
	int size;	 /* the number of key in bits */
	int read_flag_shift;
	int burn_flag_shift;
	int public;
	int reserve[3];
};

#ifdef CONFIG_ARCH_SUN8IW10

#define EFUSE_IS_PUBLIC
#define EFUSE_HAS_NO_RW_PROTECT

static struct efuse_key_map key_maps[] = {
/* Name                  Offset Size ReadFlag BurnFlag Public Reserve */
{EFUSE_CHIPID_NAME,      0x0,   128, -1,      -1,      1,     {0} },
{EFUSE_THM_SENSOR_NAME,  0x10,  32,  -1,      -1,      1,     {0} },
{"",                     0,     0,   0,       0,       0,     {0} },
};

#elif defined(CONFIG_ARCH_SUN8IW11)

#define EFUSE_IS_PUBLIC

static struct efuse_key_map key_maps[] = {
/* Name                  Offset Size ReadFlag BurnFlag Public Reserve */
{EFUSE_CHIPID_NAME,      0x0,   128, -1,      -1,      1,     {0} },
{EFUSE_IN_NAME,          0x10,  256, 23,      11,      0,     {0} },
{EFUSE_SSK_NAME,         0x30,  128, 22,      10,      0,     {0} },
{EFUSE_THM_SENSOR_NAME,  0x40,  32,  -1,      -1,      1,     {0} },
{EFUSE_FT_ZONE_NAME,     0x44,  64,  -1,      -1,      1,     {0} },
{EFUSE_TV_OUT_NAME,      0x4C,  128, -1,      -1,      0,     {0} },
{EFUSE_RSSK_NAME,        0x5C,  256, 20,      8,       0,     {0} },
{EFUSE_HDCP_HASH_NAME,   0x7C,  128, -1,      9,       0,     {0} },
{EFUSE_RESERVED_NAME,    0x90,  896, -1,      -1,      0,     {0} },
{"",                     0,     0,   0,       0,       0,     {0} },
};

static struct efuse_key_map key_map_rd_pro = {
EFUSE_RD_PROTECT_NAME,  0x8C,   32,  -1,      -1,      1,     {0} };
static struct efuse_key_map key_map_wr_pro = {
EFUSE_WR_PROTECT_NAME,  0x8C,   32,  -1,      -1,      1,     {0} };

#elif defined(CONFIG_ARCH_SUN50IW1) || defined(CONFIG_ARCH_SUN50IW2)

static struct efuse_key_map key_maps[] = {
/* Name                  Offset Size ReadFlag BurnFlag Public Reserve */
{EFUSE_CHIPID_NAME,      0x0,   128, -1,      2,       1,     {0} },
{EFUSE_OEM_NAME,         0x10,  32,  -1,      -1,      1,     {0} },
{EFUSE_RSAKEY_HASH_NAME, 0x20,  160, -1,      8,       0,     {0} },
{EFUSE_THM_SENSOR_NAME,  0x34,  64,  -1,      -1,      1,     {0} },
{EFUSE_RENEW_NAME,       0x3C,  64,  -1,      -1,      0,     {0} },
{EFUSE_IN_NAME,          0x44,  192, 17,      9,       0,     {0} },
{EFUSE_OPT_ID_NAME,      0x5C,  32,  18,      10,      0,     {0} },
{EFUSE_ID_NAME,          0x60,  32,  19,      11,      0,     {0} },
{EFUSE_ROTPK_NAME,       0x64,  256, 14,      3,       0,     {0} },
{EFUSE_SSK_NAME,         0x84,  128, 15,      4,       0,     {0} },
{EFUSE_RSSK_NAME,        0x94,  256, 16,      5,       0,     {0} },
{EFUSE_HDCP_HASH_NAME,   0xB4,  128, -1,      6,       0,     {0} },
{EFUSE_EK_HASH_NAME,     0xC4,  128, -1,      7,       0,     {0} },
{EFUSE_SN_NAME,          0xD4,  192, 20,      12,      0,     {0} },
{EFUSE_BACKUP_KEY_NAME,  0xEC,  64,  21,      22,      0,     {0} },
{"",                     0,     0,   0,       0,       0,     {0} },
};

static struct efuse_key_map key_map_rd_pro = {
EFUSE_RD_PROTECT_NAME,  0xFC,  32,  -1,      -1,      1,     {0} };
static struct efuse_key_map key_map_wr_pro = {
EFUSE_WR_PROTECT_NAME,  0xFC,  32,  -1,      -1,      1,     {0} };

#elif defined(CONFIG_ARCH_SUN50IW3) || defined(CONFIG_ARCH_SUN50IW6)

static struct efuse_key_map key_maps[] = {
/* Name                  Offset Size ReadFlag BurnFlag Public Reserve */
{EFUSE_CHIPID_NAME,      0x0,   128, 0,       0,       1,     {0} },
{EFUSE_BROM_CONF_NAME,   0x10,  16,  1,       1,       1,     {0} },
{EFUSE_BROM_TRY_NAME,    0x12,  16,  1,       1,       1,     {0} },
{EFUSE_THM_SENSOR_NAME,  0x14,  64,  2,       2,       1,     {0} },
{EFUSE_FT_ZONE_NAME,     0x1C,  128, 3,       3,       1,     {0} },
{EFUSE_OEM_NAME,         0x2C,  160, 4,       4,       1,     {0} },
{EFUSE_JTAG_SECU_NAME,   0x48,  32,  7,       7,       0,     {0} },
{EFUSE_JTAG_ATTR_NAME,   0x4C,  32,  8,       8,       0,     {0} },
{EFUSE_IN_NAME,          0x50,  192, 9,       9,       0,     {0} },
{EFUSE_OPT_ID_NAME,      0x68,  32,  10,      10,      0,     {0} },
{EFUSE_ID_NAME,          0x6C,  32,  11,      11,      0,     {0} },
{EFUSE_ROTPK_NAME,       0x70,  256, 12,      12,      0,     {0} },
{EFUSE_SSK_NAME,         0x90,  128, 13,      13,      0,     {0} },
{EFUSE_RSSK_NAME,        0xA0,  256, 14,      14,      0,     {0} },
{EFUSE_HDCP_HASH_NAME,   0xC0,  128, 15,      15,      0,     {0} },
{EFUSE_EK_HASH_NAME,     0xD0,  128, 16,      16,      0,     {0} },
{EFUSE_SN_NAME,          0xE0,  192, 17,      17,      0,     {0} },
{EFUSE_NV1_NAME,         0xF8,  32,  18,      18,      0,     {0} },
{EFUSE_NV2_NAME,         0xFC,  224, 19,      19,      0,     {0} },
#if defined(CONFIG_ARCH_SUN50IW3)
{EFUSE_BACKUP_KEY_NAME,  0x118, 320, 20,      20,      0,     {0} },
#else
{EFUSE_HDCP_PKF_NAME,    0x118, 128, 20,      20,      0,     {0} },
{EFUSE_HDCP_DUK_NAME,    0x128, 128, 21,      21,      0,     {0} },
{EFUSE_BACKUP_KEY_NAME,  0x138, 576, 22,      22,      0,     {0} },
{EFUSE_KL_SCK0_NAME,     0x180, 256, 23,      23,      0,     {0} },
{EFUSE_KL_KEY0_NAME,     0x1A0, 256, 23,      23,      0,     {0} },
{EFUSE_KL_SCK1_NAME,     0x1C0, 256, 24,      24,      0,     {0} },
{EFUSE_KL_KEY1_NAME,     0x1E0, 256, 24,      25,      0,     {0} },
#endif
{"",                     0,     0,   0,       0,       0,     {0} }
};

static struct efuse_key_map key_map_wr_pro = {
EFUSE_WR_PROTECT_NAME,  0x40,  32,  5,       5,       0,     {0} };
static struct efuse_key_map key_map_rd_pro = {
EFUSE_RD_PROTECT_NAME,  0x44,  32,  6,       6,       0,     {0} };

#else

#define EFUSE_IS_PUBLIC
#define EFUSE_HAS_NO_RW_PROTECT

static struct efuse_key_map key_maps[] = {
/* Name                  Offset Size ReadFlag BurnFlag Public Reserve */
{EFUSE_CHIPID_NAME,      0x0,   128, -1,      -1,      1,     {0} },
{"",                     0,     0,   0,       0,       0,     {0} }
};

#endif

#endif /* end of __SUNXI_SID_EFUSE_H__ */

