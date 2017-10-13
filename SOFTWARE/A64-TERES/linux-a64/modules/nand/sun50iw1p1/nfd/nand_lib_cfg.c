

#define   PHY_ONLY_TOG_AND_SDR              1

#define   PHY_WAIT_RB_BEFORE                1
#define   PHY_WAIT_RB_INTERRRUPT            1
#define   PHY_WAIT_DMA_INTERRRUPT           0



/*****************************************************************************
1.һ��ͨ������Ҫ��ͬһ��flash
2.����ͨ��Ӧ����ͬ����Ŀ�����͵�flash

��ͨ��
1.֧�� two-plane
2.֧�� vertical_interleave
3.�������ҳ����32k����֧��two-plane
4.vertical_interleave ͨ����rb����ͬ��chip���

˫ͨ��
1.֧�� two-plane
2.֧��dual_channel
3.֧��vertical_interleave
4.�������ҳ����32k����֧��two-plane
5.dual_channel ͨ����chip0��chip0���
6.vertical_interleave ͨ����rb����ͬ��chip���
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
