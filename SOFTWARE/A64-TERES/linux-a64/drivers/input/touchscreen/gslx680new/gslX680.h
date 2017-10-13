#ifndef GSLX680NEW
#define GSLX680NEW
#define TPD_X_RES           768
#define TPD_Y_RES           1024


//#define STRETCH_FRAME

#ifdef STRETCH_FRAME
#define CTP_MAX_X 		TPD_X_RES
#define CTP_MAX_Y 		TPD_Y_RES

#define X_STRETCH_MAX	(CTP_MAX_X/12)
#define Y_STRETCH_MAX	(CTP_MAX_Y/20)

#define XL_RATIO_1	25
#define XL_RATIO_2	45
#define XR_RATIO_1	35
#define XR_RATIO_2	65
#define YL_RATIO_1	0
#define YL_RATIO_2	0
#define YR_RATIO_1	0
#define YR_RATIO_2	0

#define X_STRETCH_CUST	(CTP_MAX_X/12)
#define Y_STRETCH_CUST	(CTP_MAX_Y/20)
#define X_RATIO_CUST	-15
#define Y_RATIO_CUST	2
#endif
//#define GSL_ALG_ID		//有没有id算法

struct gsl_touch_info
{
	int x[10];
	int y[10];
	int id[10];
	int finger_num;	
};

struct fw_data
{
    u32 offset : 8;
    u32 : 0;
    u32 val;
};
extern void gsl_alg_id_main(struct gsl_touch_info *cinfo);
extern void gsl_DataInit(int *ret);
extern unsigned int gsl_version_id(void);
extern unsigned int gsl_mask_tiaoping(void);

#endif
