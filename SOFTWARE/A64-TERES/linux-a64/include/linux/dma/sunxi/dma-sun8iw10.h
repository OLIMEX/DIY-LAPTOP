#ifndef __DMA_SUN8IW10__
#define __DMA_SUN8IW10__

#define DRQSRC_SRAM		0
#define DRQSRC_SDRAM		0
#define DRQSRC_SPDIFRX		2
#define DRQSRC_DAUDIO_0_RX	3
#define DRQSRC_DAUDIO_1_RX	4
#define DRQSRC_NAND0		5
#define DRQSRC_UART0RX		6
#define DRQSRC_UART1RX 		7
#define DRQSRC_UART2RX		8
#define DRQSRC_UART3RX		9
#define DRQSRC_UART4RX		10
#define DRQSRC_UART5RX		11

/* #define DRQSRC_RESEVER		12 */
/* #define DRQSRC_RESEVER		13 */

#define DRQSRC_DMIC_RX		14
#define DRQSRC_AUDIO_CODEC	15
#define DRQSRC_CODEC		DRQSRC_AUDIO_CODEC

/* #define DRQSRC_RESEVER		16 */

#define DRQSRC_OTG_EP1		17
#define DRQSRC_OTG_EP2		18
#define DRQSRC_OTG_EP3		19
#define DRQSRC_OTG_EP4		20
#define DRQSRC_OTG_EP5		21

/* #define DRQSRC_RESEVER		22 */

#define DRQSRC_SPI0RX		23
#define DRQSRC_SPI1RX		24
#define DRQSRC_SPI2RX		25

/* #define DRQSRC_RESEVER		26 */
/* #define DRQSRC_RESEVER		27 */
/* #define DRQSRC_RESEVER		28 */
/* #define DRQSRC_RESEVER		29 */
/* #define DRQSRC_RESEVER		30 */


/*
 * The destination DRQ type and port corresponding relation
 *
 */
#define DRQDST_SRAM		0
#define DRQDST_SDRAM		0
#define DRQDST_SPDIFTX		2
#define DRQDST_DAUDIO_0_TX	3
#define DRQDST_DAUDIO_1_TX	4
#define DRQDST_NAND0		5
#define DRQDST_UART0TX		6
#define DRQDST_UART1TX 		7
#define DRQDST_UART2TX		8
#define DRQDST_UART3TX		9
#define DRQDST_UART4TX		10
#define DRQDST_UART5TX		11

/* #define DRQDST_RESEVER		12 */

#define DRQDST_DSD_TX		13

/* #define DRQDST_RESEVER		14 */

#define DRQDST_AUDIO_CODEC	15
#define DRQDST_CODEC		DRQDST_AUDIO_CODEC

/* #define DRQDST_RESEVER		16 */

#define DRQDST_OTG_EP1		17
#define DRQDST_OTG_EP2		18
#define DRQDST_OTG_EP3		19
#define DRQDST_OTG_EP4		20
#define DRQDST_OTG_EP5		21

/* #define DRQDST_RESEVER		22 */

#define DRQDST_SPI0TX		23
#define DRQDST_SPI1TX		24
#define DRQDST_SPI2TX		25
/* #define DRQDST_RESEVER		26 */
/* #define DRQDST_RESEVER		27 */
/* #define DRQDST_RESEVER		28 */
/* #define DRQDST_RESEVER		29 */
/* #define DRQDST_RESEVER		30 */

#endif /*__DMA_SUN8IW10__  */
