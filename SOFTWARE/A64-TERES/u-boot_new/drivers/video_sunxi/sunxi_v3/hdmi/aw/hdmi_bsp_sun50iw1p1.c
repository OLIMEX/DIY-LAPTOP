#include "hdmi_bsp_i.h"
#include "hdmi_bsp.h"
#include "hdmi_core.h"

static unsigned int hdmi_base_addr;
static struct video_para glb_video;
//static struct audio_para glb_audio;

static int hdmi_phy_set(struct video_para *video);

struct para_tab
{
	unsigned int para[19];
};

struct pcm_sf
{
	unsigned int 	sf;
	unsigned char	cs_sf;
};

static struct para_tab ptbl[] =
{
	{{6			, 1	, 1,  1,		5,	 3, 	0,		1,	 4,		0,		0,	160,	20, 		38, 	124,	240,	22,		0,		0	}},
	{{21		, 11, 1,  1,		5,	 3, 	1,		1,	 2,		0,		0,	160,	32, 		24, 	126,	32,		24,		0,		0	}},
	{{2			, 11, 0,  0,		2,	 6, 	1,		0,	 9,		0,		0,	208,	138,		16, 	62, 	224,	45,		0,		0	}},
	{{17		, 11, 0,  0,		2,	 5, 	2,		0,	 5,		0,		0,	208,	144,		12, 	64, 	64,		49,		0,		0	}},
	{{19		, 4	, 0,  96,		5,	 5, 	2,		2,	 5,		1,		0,	0,		188,		184,	40, 	208,	30,		1,		1	}},
	{{4			, 4	, 0,  96,		5,	 5, 	2,		1,	 5,		0,		0,	0,		114,		110,	40, 	208,	30,		1,		1	}},
	{{20		, 4	, 0,  97,		7, 	 5, 	4,		2,	 2,		2,		0,	128,	208,		16,		44, 	56,		22,		1,		1	}},
	{{5			, 4	, 0,  97,		7, 	 5, 	4,		1,	 2,		0,		0,	128,	24, 		88, 	44, 	56,		22,		1,		1	}},
	{{31		, 2	, 0,  96,		7, 	 5, 	4,		2,	 4,		2,		0,	128,	208,		16,		44, 	56, 	45,		1,		1	}},
	{{16		, 2	, 0,  96,		7, 	 5, 	4,		1,	 4,		0,		0,	128,	24, 		88, 	44, 	56,		45,		1,		1	}},
	{{32		, 4	, 0,  96,		7, 	 5, 	4,		3,	 4,		2,		0,	128,	62, 		126,	44, 	56,		45,		1,		1	}},
	{{33		, 4	, 0,  0,		7, 	 5, 	4,		2,	 4,		2,		0,	128,	208,		16,		44, 	56, 	45,		1,		1	}},
	{{34		, 4	, 0,  0,		7, 	 5, 	4,		1,	 4,		0,		0,	128,	24, 		88, 	44, 	56,		45,		1,		1	}},
	{{160		,	2	, 0,  96,		7, 	 5, 	8,		3,	 4,		1,		0,	128,	62, 		126,	44, 	157,	45,		1,		1	}},
	{{147		, 2	, 0,  96,		5,	 5, 	5,		2,	 5,		1,		0,	0,		188,		184,	40, 	190,	30,		1,		1	}},
	{{132		, 2	, 0,  96,		5,	 5, 	5,		1,	 5,	  0,		0,	0,		114,		110,	40,  	160, 	30,		1,		1	}},
	{{257		, 1	, 0,  96,		15,	10, 	8,		2,	 8,		0,		0,	0,		48,			176,	88, 	112,	90,		1,		1	}},
	{{258		, 1	, 0,  96,		15,	10, 	8,		5,	 8,		4,		0,	0,		160,		32,		88, 	112,	90,		1,		1	}},
};

static unsigned char ca_table[64]=
{
	0x00,0x11,0x01,0x13,0x02,0x31,0x03,0x33,
	0x04,0x15,0x05,0x17,0x06,0x35,0x07,0x37,
	0x08,0x55,0x09,0x57,0x0a,0x75,0x0b,0x77,
	0x0c,0x5d,0x0d,0x5f,0x0e,0x7d,0x0f,0x7f,
	0x10,0xdd,0x11,0xdf,0x12,0xfd,0x13,0xff,
	0x14,0x99,0x15,0x9b,0x16,0xb9,0x17,0xbb,
	0x18,0x9d,0x19,0x9f,0x1a,0xbd,0x1b,0xbf,
	0x1c,0xdd,0x1d,0xdf,0x1e,0xfd,0x1f,0xff,
};

static struct pcm_sf sf[10] =
{
	{22050,	0x04},
	{44100,	0x00},
	{88200,	0x08},
	{176400,0x0c},
	{24000,	0x06},
	{48000, 0x02},
	{96000, 0x0a},
	{192000,0x0e},
	{32000, 0x03},
	{768000,0x09},
};

static unsigned int n_table[21] =
{
	32000,			3072,			4096,
	44100,			4704,			6272,
	88200,			4704*2,		6272*2,
	176400,			4704*4,		6272*4,
	48000,			5120,			6144,
	96000,			5120*2,		6144*2,
	192000,			5120*4,		6144*4,
};

static void hdmi_write(unsigned int addr, unsigned char data)
{
	put_bvalue(hdmi_base_addr + addr, data);
}

static unsigned char hdmi_read(unsigned int addr)
{
	return get_bvalue(hdmi_base_addr + addr);
}

#if 1//def  LINUX_OS
static hdmi_udelay __hdmi_udelay = NULL;

int api_set_func(hdmi_udelay udelay)
{
	__hdmi_udelay = udelay;
	return 0;
}
#endif

static void hdmi_phy_init(struct video_para *video)
{
	hdmi_write(0x10000, 0x01);
	hdmi_write(0x10001, 0x00);
	hdmi_write(0x10002, 100 + 5);
	hdmi_write(0x10003, 0x00);
	hdmi_write(0x10007, 0xa0);
	hdmi_write(0x0083, 0x01);
	hdmi_udelay(1);
	hdmi_write(0x0240, 0x06);
	hdmi_write(0x0240, 0x16);
	hdmi_write(0x0240, 0x12);
	hdmi_write(0x8242, 0xf0);
	hdmi_write(0xA243, 0xff);
	hdmi_write(0x6240, 0xff);
	hdmi_write(0x0012, 0xff);
	hdmi_write(0x4010, 0xff);
	hdmi_write(0x0083, 0x00);
	hdmi_write(0x0240, 0x16);
	hdmi_write(0x0240, 0x06);
	hdmi_write(0x0241, 0x20);
	hdmi_write(0x2240, 100 + 5);
	hdmi_write(0x0241, 0x00);

//	hdmi_phy_set(video);
}

static unsigned int get_vid(unsigned int id)
{
	unsigned int i,count;
	count = sizeof(ptbl)/sizeof(struct para_tab);
	for(i=0;i<count;i++) {
		if(id == ptbl[i].para[0])
			return i;
	}
	return -1;
}

static int hdmi_phy_set(struct video_para *video)
{
	unsigned int id;
	id = get_vid(video->vic);

	switch(ptbl[id].para[1])
	{
		case 1:
			hdmi_write(0x2241, 0x06);
			hdmi_write(0xA240, 0x00);
			hdmi_write(0xA241, 0x00);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			hdmi_write(0x2241, 0x15);
			hdmi_write(0xA240, 0x00);
			hdmi_write(0xA241, 0x0f);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			hdmi_write(0x2241, 0x10);
			hdmi_write(0xA240, 0x00);
			hdmi_write(0xA241, 0x00);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			hdmi_write(0x2241, 0x19);
			hdmi_write(0xA240, 0x00);
			hdmi_write(0xA241, 0x02);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			hdmi_write(0x2241, 0x0e);
			hdmi_write(0xA240, 0x00);
			hdmi_write(0xA241, 0x00);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			hdmi_write(0x2241, 0x09);
			hdmi_write(0xA240, 0x80);
			hdmi_write(0xA241, 0x2b);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			break;
		case 2:
			hdmi_write(0x2241, 0x06);
			hdmi_write(0xA240, 0x04);
			hdmi_write(0xA241, 0xa0);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			hdmi_write(0x2241, 0x15);
			hdmi_write(0xA240, 0x00);
			hdmi_write(0xA241, 0x0a);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			hdmi_write(0x2241, 0x10);
			hdmi_write(0xA240, 0x00);
			hdmi_write(0xA241, 0x00);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			hdmi_write(0x2241, 0x19);
			hdmi_write(0xA240, 0x00);
			hdmi_write(0xA241, 0x02);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			hdmi_write(0x2241, 0x0e);
			hdmi_write(0xA240, 0x00);
			hdmi_write(0xA241, 0x21);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			hdmi_write(0x2241, 0x09);
			hdmi_write(0xA240, 0x80);
			hdmi_write(0xA241, 0x29);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			break;
		case 4:
			hdmi_write(0x2241, 0x06);
			hdmi_write(0xA240, 0x05);
			hdmi_write(0xA241, 0x40);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			hdmi_write(0x2241, 0x15);
			hdmi_write(0xA240, 0x00);
			hdmi_write(0xA241, 0x05);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			hdmi_write(0x2241, 0x10);
			hdmi_write(0xA240, 0x00);
			hdmi_write(0xA241, 0x00);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			hdmi_write(0x2241, 0x19);
			hdmi_write(0xA240, 0x00);
			hdmi_write(0xA241, 0x07);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			hdmi_write(0x2241, 0x0e);
			hdmi_write(0xA240, 0x02);
			hdmi_write(0xA241, 0xb5);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			hdmi_write(0x2241, 0x09);
			hdmi_write(0xA240, 0x80);
			hdmi_write(0xA241, 0x09);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			break;
		case 11:
			hdmi_write(0x2241, 0x06);
			hdmi_write(0xA240, 0x01);
			hdmi_write(0xA241, ptbl[id].para[2] ? 0xe3 : 0xe0);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			hdmi_write(0x2241, 0x15);
			hdmi_write(0xA240, 0x00);
			hdmi_write(0xA241, 0x00);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			hdmi_write(0x2241, 0x10);
			hdmi_write(0xA240, 0x08);
			hdmi_write(0xA241, 0xda);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			hdmi_write(0x2241, 0x19);
			hdmi_write(0xA240, 0x00);
			hdmi_write(0xA241, 0x07);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			hdmi_write(0x2241, 0x0E);
			hdmi_write(0xA240, 0x03);
			hdmi_write(0xA241, 0x18);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			hdmi_write(0x2241, 0x09);
			hdmi_write(0xA240, 0x80);
			hdmi_write(0xA241, 0x09);
			hdmi_write(0xA242, 0x10);
			hdmi_udelay(2000);
			break;
		default:
			return -1;
	}
	hdmi_write(0x2241, 0x1e);
	hdmi_write(0xA240, 0x00);
	hdmi_write(0xA241, 0x00);
	hdmi_write(0xA242, 0x10);
	hdmi_udelay(2000);
	hdmi_write(0x2241, 0x13);
	hdmi_write(0xA240, 0x00);
	hdmi_write(0xA241, 0x00);
	hdmi_write(0xA242, 0x10);
	hdmi_udelay(2000);
	hdmi_write(0x2241, 0x17);
	hdmi_write(0xA240, 0x00);
	hdmi_write(0xA241, 0x00);
	hdmi_write(0xA242, 0x10);
	hdmi_udelay(2000);
	hdmi_write(0x0240, 0x0e);
	return 0;
}

void bsp_hdmi_set_addr(unsigned int base_addr)
{
	hdmi_base_addr = base_addr;
}

void bsp_hdmi_init()
{
	struct video_para vpara;

	hdmi_write(0x10010,0x45);
	hdmi_write(0x10011,0x45);
	hdmi_write(0x10012,0x52);
	hdmi_write(0x10013,0x54);
	hdmi_write(0x8080, 0x00);
	hdmi_udelay(1);
	hdmi_write(0xF01F, 0x00);
	hdmi_write(0x8403, 0xff);
	hdmi_write(0x904C, 0xff);
	hdmi_write(0x904E, 0xff);
	hdmi_write(0xD04C, 0xff);
	hdmi_write(0x8250, 0xff);
	hdmi_write(0x8A50, 0xff);
	hdmi_write(0x8272, 0xff);
	hdmi_write(0x40C0, 0xff);
	hdmi_write(0x86F0, 0xff);
	hdmi_write(0x0EE3, 0xff);
	hdmi_write(0x8EE2, 0xff);
	hdmi_write(0xA049, 0xf0);
	hdmi_write(0xB045, 0x1e);
	hdmi_write(0x00C1, 0x00);
	hdmi_write(0x00C1, 0x03);
	hdmi_write(0x00C0, 0x00);
	hdmi_write(0x40C1, 0x10);
	hdmi_write(0x0081, 0xff);
	hdmi_write(0x0081, 0x00);
	hdmi_write(0x0081, 0xff);
	hdmi_write(0x0010, 0xff);
	hdmi_write(0x0011, 0xff);
	hdmi_write(0x8010, 0xff);
	hdmi_write(0x8011, 0xff);
	hdmi_write(0x0013, 0xff);
	hdmi_write(0x8012, 0xff);
	hdmi_write(0x8013, 0xff);

	vpara.vic = 17;
	hdmi_phy_init(&vpara);
}

void bsp_hdmi_set_video_en(unsigned char enable)
{
#if 0
	if(enable)
		hdmi_write(0x0840, 0x00);
	else
		hdmi_write(0x0840, 0x01);
#endif
}

int bsp_hdmi_video(struct video_para *video)
{
	unsigned int id = get_vid(video->vic);
	glb_video.vic = video->vic;

	switch(glb_video.vic)
	{
		case 2:
		case 4:
		case 6:
		case 17:
		case 19:
		case 21:
			video->csc = BT601;
			break;
		default:
			video->csc = BT709;
			break;
	}

	bsp_hdmi_init();
	
	hdmi_write(0x0840, 0x01);
	hdmi_write(0x4845, 0x00);
	hdmi_write(0x0040, ptbl[id].para[3] |	0x10);
	hdmi_write(0x10001, ( (ptbl[id].para[3] < 96) ? 0x03 : 0x00 ) );
	hdmi_write(0x8040, ptbl[id].para[4]);
	hdmi_write(0x4043, ptbl[id].para[5]);
	hdmi_write(0x8042, ptbl[id].para[6]);
	hdmi_write(0x0042, ptbl[id].para[7]);
	hdmi_write(0x4042, ptbl[id].para[8]);
	hdmi_write(0x4041, ptbl[id].para[9]);
	hdmi_write(0xC041, ptbl[id].para[10]);
	hdmi_write(0x0041, ptbl[id].para[11]);
	hdmi_write(0x8041, ptbl[id].para[12]);
	hdmi_write(0x4040, ptbl[id].para[13]);
	hdmi_write(0xC040, ptbl[id].para[14]);
	hdmi_write(0x0043, ptbl[id].para[15]);
	hdmi_write(0x8043, ptbl[id].para[16]);
	hdmi_write(0x0045, 0x0c);
	hdmi_write(0x8044, 0x20);
	hdmi_write(0x8045, 0x01);
	hdmi_write(0x0046, 0x0b);
	hdmi_write(0x0047, 0x16);
	hdmi_write(0x8046, 0x21);
	hdmi_write(0x3048, ptbl[id].para[2] ? 0x21 : 0x10);
	hdmi_write(0x0401, ptbl[id].para[2] ? 0x41 : 0x40);
	hdmi_write(0x8400, 0x07);
	hdmi_write(0x8401, 0x00);
	hdmi_write(0x0402, 0x47);

	hdmi_write(0x0800, 0x01);
	hdmi_write(0x0801, 0x07);
	hdmi_write(0x8800, 0x00);
	hdmi_write(0x8801, 0x00);
	hdmi_write(0x0802, 0x00);
	hdmi_write(0x0803, 0x00);
	hdmi_write(0x8802, 0x00);
	hdmi_write(0x8803, 0x00);

	if(video->is_hdmi)
	{
		hdmi_write(0xB045, 0x08);
		hdmi_write(0x2045, 0x00);
		hdmi_write(0x2044, 0x0c);
		hdmi_write(0x6041, 0x03);
		hdmi_write(0xA044, ((ptbl[id].para[0]&0x100) == 0x100) ? 0x20 : ( ((ptbl[id].para[0]&0x80) == 0x80) ? 0x40 : 0x00) );
		hdmi_write(0xA045, ((ptbl[id].para[0]&0x100) == 0x100) ? (ptbl[id].para[0]&0x7f) : 0x00);
		hdmi_write(0x2046, 0x00);
		hdmi_write(0x3046, 0x01);
		hdmi_write(0x3047, 0x11);
		hdmi_write(0x4044, 0x00);
		hdmi_write(0x0052, 0x00);
		hdmi_write(0x8051, 0x11);
		hdmi_write(0x10010,0x45);
		hdmi_write(0x10011,0x45);
		hdmi_write(0x10012,0x52);
		hdmi_write(0x10013,0x54);
		hdmi_write(0x0040, hdmi_read(0x0040) | 0x08 );
		hdmi_write(0x10010,0x52);
		hdmi_write(0x10011,0x54);
		hdmi_write(0x10012,0x41);
		hdmi_write(0x10013,0x57);
		hdmi_write(0x4045, video->is_yuv ? 0x02 : 0x00);
		if(ptbl[id].para[16] == 0)
			hdmi_write(0xC044, (video->csc << 6) | 0x18);
		else if(ptbl[id].para[16] == 1)
			hdmi_write(0xC044, (video->csc << 6) | 0x28);
		else
			hdmi_write(0xC044, (video->csc << 6) | 0x08);

		hdmi_write(0xC045, 0x00);
		hdmi_write(0x4046, ptbl[id].para[0]&0x7f);
	}

	if(video->is_hcts)
	{
		hdmi_write(0x00C0, video->is_hdmi ? 0x91 : 0x90 );
		hdmi_write(0x00C1, 0x05);
		hdmi_write(0x40C1, (ptbl[id].para[3] < 96) ? 0x10 : 0x1a);
		hdmi_write(0x80C2, 0xff);
		hdmi_write(0x40C0, 0xfd);
		hdmi_write(0xC0C0, 0x40);
		hdmi_write(0x00C1, 0x04);
		hdmi_write(0x10010,0x45);
		hdmi_write(0x10011,0x45);
		hdmi_write(0x10012,0x52);
		hdmi_write(0x10013,0x54);
		hdmi_write(0x0040, hdmi_read(0x0040) | 0x80 );
		hdmi_write(0x00C0, video->is_hdmi ? 0x95 : 0x94 );
		hdmi_write(0x10010,0x52);
		hdmi_write(0x10011,0x54);
		hdmi_write(0x10012,0x41);
		hdmi_write(0x10013,0x57);
	}

	hdmi_write(0x0082, 0x00);
	hdmi_write(0x0081, 0x00);

	if(hdmi_phy_set(video)!=0)
		return -1;

	hdmi_write(0x0840, 0x00);

	if(video->is_hcts)
	{

	}

	return 0;
}

int bsp_hdmi_audio(struct audio_para *audio)
{
	unsigned int i;
	unsigned int n;
	unsigned id = get_vid(glb_video.vic);

	hdmi_write(0xA049, (audio->ch_num > 2) ? 0xf1 : 0xf0);

	for(i = 0; i < 64; i += 2)
	{
		if(audio->ca == ca_table[i])
		{
			hdmi_write(0x204B, ~ca_table[i+1]);
			break;
		}
	}
	hdmi_write(0xA04A, 0x00);
	hdmi_write(0xA04B, 0x30);
	hdmi_write(0x6048, 0x00);
	hdmi_write(0x6049, 0x01);
	hdmi_write(0xE048, 0x42);
	hdmi_write(0xE049, 0x86);
	hdmi_write(0x604A, 0x31);
	hdmi_write(0x604B, 0x75);
	hdmi_write(0xE04A, 0x00 | 0x01);
	for(i = 0; i < 10; i += 1)
	{
		if(audio->sample_rate == sf[i].sf)
		{
			hdmi_write(0xE04A, 0x00 | sf[i].cs_sf);
			break;
		}
	}
	hdmi_write(0xE04B, 0x00 |
						(audio->sample_bit == 16) ? 0x02 : ((audio->sample_bit == 24) ? 0xb : 0x0) );


	hdmi_write(0x0251, audio->sample_bit);



	n = 6272;
	//cts = 0;
	for(i = 0; i < 21; i += 3)
	{
		if(audio->sample_rate == n_table[i])
		{
			if(ptbl[id].para[1] == 1)
				n = n_table[i+1];
			else
				n = n_table[i+2];

			//cts = (n / 128) * (glb_video.tmds_clk / 100) / (audio->sample_rate / 100);
			break;
		}
	}

	hdmi_write(0x0A40, n);
	hdmi_write(0x0A41, n >> 8);
	hdmi_write(0x8A40, n >> 16);
	hdmi_write(0x0A43, 0x00);
	hdmi_write(0x8A42, 0x04);
	hdmi_write(0xA049, (audio->ch_num > 2) ? 0x01 : 0x00);
	hdmi_write(0x2043, audio->ch_num * 16);
	hdmi_write(0xA042, 0x00);
	hdmi_write(0xA043, audio->ca);
	hdmi_write(0x6040, 0x00);

	if(audio->type == PCM)
	{
		hdmi_write(0x8251, 0x00);
	}
	else if((audio->type == DTS_HD) || (audio->type == DDP))
	{
		hdmi_write(0x8251, 0x03);
		hdmi_write(0x0251, 0x15);
		hdmi_write(0xA043, 0);
	}
	else
	{
		hdmi_write(0x8251, 0x02);
		hdmi_write(0x0251, 0x15);
		hdmi_write(0xA043, 0);
	}

	hdmi_write(0x0250, 0x00);
	hdmi_write(0x0081, 0x08);
	hdmi_write(0x8080, 0xf7);
	hdmi_udelay(100);
	hdmi_write(0x0250, 0xaf);
	hdmi_udelay(100);
	hdmi_write(0x0081, 0x00);

	return 0;
}

int bsp_hdmi_ddc_read(char cmd,char pointer,char offset,int nbyte,char * pbuf)
{
	unsigned char off = offset;
	unsigned int to_cnt;
	int ret = 0;

	hdmi_write(0x8EE3, 0x05);
	hdmi_write(0x0EE3, 0x08);

	to_cnt = 10;
	while(nbyte > 0)
	{
		to_cnt = 10;
		hdmi_write(0x0EE0, 0xa0 >> 1);
		hdmi_write(0x0EE1, off);
		hdmi_write(0x4EE0, 0x60 >> 1);
		hdmi_write(0xCEE0, pointer);
		hdmi_write(0x0EE2, 0x02);
		hdmi_write(0x10010,0x45);
		hdmi_write(0x10011,0x45);
		hdmi_write(0x10012,0x52);
		hdmi_write(0x10013,0x54);
		while(1)
	  {
			to_cnt--;	//wait for 10ms for timeout
			if(to_cnt == 0)
				break;

			if( (hdmi_read(0x0013) & 0x02) == 0x02)
			{
				hdmi_write(0x0013, hdmi_read(0x0013) & 0x02);
				* pbuf++ =  hdmi_read(0x8EE1);
				break;
			}
			else if( (hdmi_read(0x0013) & 0x01) == 0x01)
			{
				hdmi_write(0x0013, hdmi_read(0x0013) & 0x01);
				ret = -1;
				break;
			}
			hdmi_udelay(1000);
	  }
	  nbyte --;
	  off ++;
	}

	hdmi_write(0x10010,0x52);
	hdmi_write(0x10011,0x54);
	hdmi_write(0x10012,0x41);
	hdmi_write(0x10013,0x57);

	return ret;
}

unsigned int bsp_hdmi_get_hpd()
{
	unsigned int ret = 0;

	hdmi_write(0x10010,0x45);
	hdmi_write(0x10011,0x45);
	hdmi_write(0x10012,0x52);
	hdmi_write(0x10013,0x54);

	if( (hdmi_read(0x0243) & 0x2) == 0x2)
		ret = 1;
	else
		ret = 0;

	hdmi_write(0x10010,0x52);
	hdmi_write(0x10011,0x54);
	hdmi_write(0x10012,0x41);
	hdmi_write(0x10013,0x57);

	return ret;
}

void bsp_hdmi_standby()
{
	hdmi_write(0x0240, 0x14);
  hdmi_write(0x0083, 0x01);
  hdmi_write(0x10007, 0x20);
}

void bsp_hdmi_hrst()
{
	hdmi_write(0x00C1, 0x04);
}

void bsp_hdmi_hdl()
{

}
