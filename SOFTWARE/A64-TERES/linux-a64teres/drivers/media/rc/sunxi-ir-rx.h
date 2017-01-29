#ifndef SUNXI_IR_RX_H
#define SUNXI_IR_RX_H

#define CONFIG_FPGA_V4_PLATFORM
/* Registers */
#define IR_REG(x)        (x)

#define IR_CTRL_REG      IR_REG(0x00)     /* IR Control */
#define IR_RXCFG_REG     IR_REG(0x10)     /* Rx Config */
#define IR_RXDAT_REG     IR_REG(0x20)     /* Rx Data */
#define IR_RXINTE_REG    IR_REG(0x2C)     /* Rx Interrupt Enable */
#define IR_RXINTS_REG    IR_REG(0x30)     /* Rx Interrupt Status */
#define IR_SPLCFG_REG    IR_REG(0x34)     /* IR Sample Config */

#define IR_FIFO_SIZE     (64)             /* 64Bytes */

#if (defined CONFIG_FPGA_V4_PLATFORM) || (defined CONFIG_FPGA_V7_PLATFORM)
#define CIR_FPGA
#endif

#ifdef CIR_FPGA
#define IR_SIMPLE_UNIT	 (21333)	  /* simple in ns */
#define IR_CLK		 (24000000)	  /* fpga clk output is fixed */
#define IR_CIR_MODE      (0x3<<4)         /* CIR mode enable */
#define IR_ENTIRE_ENABLE (0x3<<0)         /* IR entire enable */
#define IR_SAMPLE_DEV    (0x3<<0)         /* 24MHz/512 =46875Hz (21333ns) */
#define IR_FIFO_32       (((IR_FIFO_SIZE>>1)-1)<<8)
#define IR_IRQ_STATUS    ((0x1<<4)|0x3)
#else
#define IR_SIMPLE_UNIT	 (32000)	  /* simple in ns */
#define IR_CLK		 (8000000)
#define IR_CIR_MODE      (0x3<<4)         /* CIR mode enable */
#define IR_ENTIRE_ENABLE (0x3<<0)         /* IR entire enable */
#define IR_SAMPLE_DEV    (0x2<<0)         /* 4MHz/256 =31250Hz (32000ns) */
#define IR_FIFO_32       (((IR_FIFO_SIZE>>1)-1)<<8)
#define IR_IRQ_STATUS    ((0x1<<4)|0x3)
#endif

//Bit Definition of IR_RXINTS_REG Register
#define IR_RXINTS_RXOF   (0x1<<0)         /* Rx FIFO Overflow */
#define IR_RXINTS_RXPE   (0x1<<1)         /* Rx Packet End */
#define IR_RXINTS_RXDA   (0x1<<4)         /* Rx FIFO Data Available */

#ifdef CIR_FPGA
#define IR_RXFILT_VAL    (((16)&0x3f)<<2)  /* Filter Threshold = 16*21.3 = ~341us < 500us */
#define IR_RXIDLE_VAL    (((5)&0xff)<<8)  /* Idle Threshold = (5+1)*128*21.3 = ~16.4ms > 9ms */
#define IR_ACTIVE_T      ((0&0xff)<<16)   /* Active Threshold */
#define IR_ACTIVE_T_C    ((1&0xff)<<23)   /* Active Threshold */
#else
#define IR_RXFILT_VAL    (((12)&0x3f)<<2)  /* Filter Threshold = 12*32 = ~384us < 500us */
#define IR_RXIDLE_VAL    (((2)&0xff)<<8)  /* Idle Threshold = (2+1)*128*32 = ~23.8ms > 9ms */
#define IR_ACTIVE_T      ((99&0xff)<<16)   /* Active Threshold */
#define IR_ACTIVE_T_C    ((0&0xff)<<23)   /* Active Threshold */
#endif

#define IR_ERROR_CODE    (0xffffffff)
#define IR_REPEAT_CODE   (0x00000000)
#define DRV_VERSION      "1.00"

#define RC_MAP_SUNXI "rc_map_sunxi"

enum {
	DEBUG_INIT = 1U << 0,
	DEBUG_INT = 1U << 1,
	DEBUG_DATA_INFO = 1U << 2,
	DEBUG_SUSPEND = 1U << 3,
	DEBUG_ERR = 1U << 4,
};

enum ir_mode {
	CIR_MODE_ENABLE,
	IR_MODULE_ENABLE,
};
enum ir_sample_config {
	IR_SAMPLE_REG_CLEAR,
	IR_CLK_SAMPLE,
	IR_FILTER_TH,
	IR_IDLE_TH,
	IR_ACTIVE_TH,
};
enum ir_irq_config {
	IR_IRQ_STATUS_CLEAR,
	IR_IRQ_ENABLE,
	IR_IRQ_FIFO_SIZE,
};
enum {
	IR_SUPLY_DISABLE = 0,
	IR_SUPLY_ENABLE,
};

struct sunxi_ir_data{
	void __iomem 	*reg_base;
	struct platform_device	*pdev;
	struct clk *mclk;
	struct clk *pclk;
	struct rc_dev *rcdev;
	struct regulator *suply;
	int irq_num;
	u32 ir_addr;
};

int init_rc_map_sunxi(u32 addr);
void exit_rc_map_sunxi(void);

#endif /* SUNXI_IR_RX_H */
