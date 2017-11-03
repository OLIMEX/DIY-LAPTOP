/*
************************************************************************************************************************
*                                          Boot rom
*                                         Seucre Boot
*
*                             Copyright(C), 2006-2013, AllWinners Microelectronic Co., Ltd.
*											       All Rights Reserved
*
* File Name   : Base.h
*
* Author      : glhuang
*
* Version     : 0.0.1
*
* Date        : 2013.09.05
*
* Description :
*
* Others      : None at present.
*
*
* History     :
*
*  <Author>        <time>       <version>      <description>
*
* glhuang       2013.09.05       0.0.1        build the file
*
************************************************************************************************************************
*/
#ifndef	__SBROM_HW__H__
#define	__SBROM_HW__H__

#include "private_toc.h"
//==============================================================================

typedef struct
{
	struct sbrom_toc1_item_info  *key_certif;
	struct sbrom_toc1_item_info  *bin_certif;
	struct sbrom_toc1_item_info  *binfile;
	struct sbrom_toc1_item_info  *normal;
}sbrom_toc1_item_group;
//===================================


int toc1_init(void);
int toc1_item_traverse(void);
uint toc1_item_read(struct sbrom_toc1_item_info *p_toc_item, void * p_dest, u32 buff_len);

uint toc1_item_read_rootcertif(void * p_dest, u32 buff_len);
int toc1_item_probe_start(void);
int toc1_item_probe_next(sbrom_toc1_item_group *item_group);


#endif	//__SBROM_HW__H__
