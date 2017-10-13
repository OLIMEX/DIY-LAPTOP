



.globl RMR_TO64

RMR_TO64:
	MRC p15,0,r1,c12,c0,2
	ORR r1,r1,#(0x3<<0)
	DSB
	MCR p15,0,r1,c12,c0,2
	ISB

Loop1:
	WFI
	B	Loop1
	BX	lr



