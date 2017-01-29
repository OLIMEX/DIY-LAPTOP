	.globl  sunxi_bromjump_start
	.globl  sunxi_bromjump_end
sunxi_bromjump_start:
	.incbin "drivers/soc/allwinner/pm/standby/brom/resumes.code"
sunxi_bromjump_end:
	.align  2
