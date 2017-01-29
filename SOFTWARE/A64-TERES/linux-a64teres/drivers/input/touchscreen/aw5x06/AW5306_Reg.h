/**************************************************************************
*  AW5306_Reg.h
* 
*  AW5306 Driver code version 1.0
* 
*  Create Date : 2012/06/25
* 
*  Modify Date : 
*
*  Create by   : wuhaijun
* 
**************************************************************************/

#ifndef AW5306_REG_H

#define AW5306_REG_H

#define SA_PAGE						0x00
#define SA_IDRST					0x01
#define SA_CTRL						0x02
#define SA_SCANMD					0x03
#define SA_IER		   				0x04
#define SA_RX_NUM					0x05
#define SA_RX_START					0x06
#define SA_TX_NUM					0x07
#define SA_TX_INDEX0    			0x08
#define SA_TX_INDEX1    			0x09
#define SA_TX_INDEX2    			0x0A
#define SA_TX_INDEX3    			0x0B
#define SA_TX_INDEX4    			0x0C
#define SA_TX_INDEX5    			0x0D
#define SA_TX_INDEX6    			0x0E
#define SA_TX_INDEX7    			0x0F
#define SA_TX_INDEX8    			0x10
#define SA_TX_INDEX9    			0x11
#define SA_TX_INDEX10   			0x12
#define SA_TX_INDEX11   			0x13
#define SA_TX_INDEX12   			0x14
#define SA_TX_INDEX13   			0x15
#define SA_TX_INDEX14   			0x16
#define SA_TX_INDEX15   			0x17
#define SA_TX_INDEX16   			0x18
#define SA_TX_INDEX17   			0x19
#define SA_TX_INDEX18   			0x1A
#define SA_TX_INDEX19				0x1B
#define SA_TX_INDEX20				0x1C
#define SA_TXCAC0    				0x1D
#define SA_TXCAC1    				0x1E
#define SA_TXCAC2    				0x1F
#define SA_TXCAC3    				0x20
#define SA_TXCAC4    				0x21
#define SA_TXCAC5    				0x22
#define SA_TXCAC6    				0x23
#define SA_TXCAC7    				0x24
#define SA_TXCAC8    				0x25
#define SA_TXCAC9    				0x26
#define SA_TXCAC10   				0x27
#define SA_TXCAC11   				0x28
#define SA_TXCAC12   				0x29
#define SA_TXCAC13   				0x2A
#define SA_TXCAC14   				0x2B
#define SA_TXCAC15   				0x2C
#define SA_TXCAC16   				0x2D
#define SA_TXCAC17   				0x2E
#define SA_TXCAC18   				0x2F
#define SA_TXCAC19   				0x30
#define SA_TXCAC20					0x31
#define SA_TXOFFSET0    			0x32
#define SA_TXOFFSET1    			0x33
#define SA_TXOFFSET2    			0x34
#define SA_TXOFFSET3    			0x35
#define SA_TXOFFSET4    			0x36
#define SA_TXOFFSET5    			0x37
#define SA_TXOFFSET6    			0x38
#define SA_TXOFFSET7    			0x39
#define SA_TXOFFSET8    			0x3A
#define SA_TXOFFSET9    			0x3B
#define SA_TXOFFSET10   			0x3C
#define SA_RXCAC0					0x3E
#define SA_RXCAC1					0x3F
#define SA_RXCAC2					0x40
#define SA_RXCAC3					0x41
#define SA_RXCAC4					0x42
#define SA_RXCAC5					0x43
#define SA_RXCAC6					0x44
#define SA_RXCAC7					0x45
#define SA_RXCAC8					0x46
#define SA_RXCAC9					0x47
#define SA_RXCAC10					0x48
#define SA_RXCAC11					0x49
#define SA_RXOFFSET0    			0x4A
#define SA_RXOFFSET1    			0x4B
#define SA_RXOFFSET2    			0x4C
#define SA_RXOFFSET3    			0x4D
#define SA_RXOFFSET4   	 			0x4E
#define SA_RXOFFSET5    			0x4F
#define SA_DRV_VLT					0x51
#define SA_SCANFREQ1				0x52
#define SA_SCANFREQ2				0x53
#define SA_SCANFREQ3				0x54
#define SA_TXADCGAIN0   			0x55
#define SA_TXADCGAIN1   			0x56
#define SA_TXADCGAIN2   			0x57
#define SA_TXADCGAIN3   			0x58
#define SA_TXADCGAIN4   			0x59
#define SA_TXADCGAIN5   			0x5A
#define SA_TXADCGAIN6   			0x5B
#define SA_TXADCGAIN7   			0x5C
#define SA_TXADCGAIN8   			0x5D
#define SA_TXADCGAIN9   			0x5E
#define SA_TXADCGAIN10  			0x5F
#define SA_TXADCGAIN11  			0x60
#define SA_TXADCGAIN12  			0x61
#define SA_TXADCGAIN13  			0x62
#define SA_TXADCGAIN14  			0x63
#define SA_TXADCGAIN15  			0x64
#define SA_TXADCGAIN16  			0x65
#define SA_TXADCGAIN17  			0x66
#define SA_TXADCGAIN18  			0x67
#define SA_TXADCGAIN19  			0x68
#define SA_TXADCGAIN20				0x69
#define SA_WAITTIME					0x6A
#define SA_TCLKDLY      			0x6B
#define SA_FINEADJ					0x6C
#define SA_TXCLKFREQ				0x6D
#define SA_SCANTIM      			0x6E
#define SA_READSEL					0x70
#define SA_ISR          			0x71
#define SA_STATE1       			0x72
#define SA_POSCNT					0x73
#define SA_NEGCNT					0x74
#define SA_VLDNUM					0x75
#define SA_ADDRH					0x7D
#define SA_ADDRL					0x7E
#define SA_RAWDATA					0x7F
////////////////////////
// Page 2               
////////////////////////
#define SA_SINETABE1				0x03
#define SA_SINETABE2				0x04
#define SA_DATAOFFSET				0x05
#define SA_TRACECTRL1				0x10
#define SA_TRACECTRL2				0x11
#define SA_TRACECTRL3				0x12
#define SA_TRACEST	    			0x13
#define SA_RPTNEGTH     			0x14
#define SA_RPTPOSTH     			0x15
#define SA_TRACESTEP				0x16
#define SA_TRCLVLLO 				0x17
#define SA_TRCLVLPOSHI				0x18
#define SA_TRCLVLNEGHI				0x19
#define SA_TRACEINTERVAL 			0x1A
#define SA_RXSTABLETH				0x1B
#define SA_POSLEVELTH   			0x1C
#define SA_POSNUMTH     			0x1D
#define SA_NEGLEVELTH   			0x1E
#define SA_NEGNUMTH     			0x1F
#define SA_BIGPOINTTH   			0x20
#define SA_BIGPOSTIMTH  			0x21
#define SA_BIGNEGTIMTH  			0x22
#define SA_NEGTIMTH     			0x23
#define SA_TRACEHIGHTIM 			0x24
#define SA_INITPNTTH    			0x25
#define SA_TCHCLRTIMSET 			0x26
#define SA_INITLVTH					0x27
#define SA_MAXCHKTH     			0x28
#define SA_MINCHKTH     			0x29
#define SA_INITFORCEQUIT 			0x2A               
#define SA_CHAMPCFG					0x30
#define SA_ADCCFG					0x31
#define SA_IBCFG1   				0x32
#define SA_IBCFG2   				0x33
#define SA_LDOCFG					0x34
#define SA_OSCCFG1   				0x35
#define SA_OSCCFG2   				0x36
#define SA_OSCCFG3   				0x37
#define SA_EN_CLK_QNTZ1 			0x38
#define SA_EN_CLK_QNTZ2 			0x39
#define SA_CPFREQ					0x3A
#define SA_ATEST1					0x3B
#define SA_ATEST2					0x3C
#define SA_RAMTST					0x60
#define SA_TESTCFG					0x61
#define SA_TSTDATAH     			0x62
#define SA_TSTDATAL     			0x63
#endif

