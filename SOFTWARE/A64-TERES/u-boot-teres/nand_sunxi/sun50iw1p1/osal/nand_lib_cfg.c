

#define   PHY_ONLY_TOG_AND_SDR              1

#define   PHY_WAIT_RB_BEFORE                0
#define   PHY_WAIT_RB_INTERRRUPT            0
#define   PHY_WAIT_DMA_INTERRRUPT           0



/*****************************************************************************
1.一个通道内需要贴同一种flash
2.两个通道应该贴同样数目和类型的flash

单通道
1.支持 two-plane
2.支持 vertical_interleave
3.如果超级页超过32k，不支持two-plane
4.vertical_interleave 通道内rb不相同的chip配对

双通道
1.支持 two-plane
2.支持dual_channel
3.支持vertical_interleave
4.如果超级页超过32k，不支持two-plane
5.dual_channel 通道间chip0和chip0配对
6.vertical_interleave 通道内rb不相同的chip配对
*****************************************************************************/
#define   PHY_SUPPORT_TWO_PLANE                          1
#define   PHY_SUPPORT_VERTICAL_INTERLEAVE                1
#define   PHY_SUPPORT_DUAL_CHANNEL                       1


/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int nand_cfg_interface(void)
{
    return PHY_ONLY_TOG_AND_SDR ? 1 : 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int nand_wait_rb_before(void)
{
    return PHY_WAIT_RB_BEFORE ? 1 : 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int nand_wait_rb_mode(void)
{
    return PHY_WAIT_RB_INTERRRUPT ? 1 : 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int nand_wait_dma_mode(void)
{
    return PHY_WAIT_DMA_INTERRRUPT ? 1 : 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int nand_support_two_plane(void)
{
    return PHY_SUPPORT_TWO_PLANE ? 1 : 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int nand_support_vertical_interleave(void)
{
    return PHY_SUPPORT_VERTICAL_INTERLEAVE ? 1 : 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int nand_support_dual_channel(void)
{
    return PHY_SUPPORT_DUAL_CHANNEL ? 1 : 0;
}
