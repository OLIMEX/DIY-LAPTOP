/*
 * sunxi mipi bsp interface
 * Author:wangxuan
 */
//#include "include.h"
#include "bsp_mipi_csi.h"
#include "../utility/vfe_io.h"

void bsp_mipi_csi_set_version(unsigned int sel, unsigned int ver)
{
	return;
}

int bsp_mipi_csi_set_base_addr(unsigned int sel, unsigned long addr_base)
{
	return 0;
}

int bsp_mipi_dphy_set_base_addr(unsigned int sel, unsigned long addr_base)
{
  return 0;
}

void bsp_mipi_csi_dphy_init(unsigned int sel)
{
	return;
}

void bsp_mipi_csi_dphy_exit(unsigned int sel)
{
	return;
}

void bsp_mipi_csi_dphy_enable(unsigned int sel)
{
	return;
}

void bsp_mipi_csi_dphy_disable(unsigned int sel)
{
	return;
}

void bsp_mipi_csi_protocol_enable(unsigned int sel)
{
	return;
}

void bsp_mipi_csi_protocol_disable(unsigned int sel)
{
	return;
}

void bsp_mipi_csi_set_para(unsigned int sel, struct mipi_para *para)
{
	return;
}

void bsp_mipi_csi_set_fmt(unsigned int sel, unsigned int total_rx_ch, struct mipi_fmt *fmt)
{   
	return;
}
    
