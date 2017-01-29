#ifndef _SPRITE_STORAGE_CRYPT_H_
#define _SPRITE_STORAGE_CRYPT_H_


#define MAX_STORE_LEN 0xc00 /*3K payload*/
#define STORE_OBJECT_MAGIC	0x17253948
#define STORE_REENCRYPT_MAGIC 0x86734716
typedef struct{
	unsigned int	magic ; /* store object magic*/
	int				id ;    /*store id, 0x01,0x02.. for user*/
	char			name[64]; /*OEM name*/
    unsigned int	re_encrypt; /*flag for OEM object*/
	unsigned int	version ;	
	unsigned int	reserved[4];
	unsigned int	actual_len ; /*the actual len in data buffer*/
	unsigned char	data[MAX_STORE_LEN]; /*the payload of secure object*/
	unsigned int	crc ; /*crc to check the sotre_objce valid*/
}store_object_t;


enum{
    /* TEE SMC command type */
    TEE_SMC_INIT_CALL                   = 0x0FFFFFF1,
    TEE_SMC_PLAFORM_OPERATION           = 0x0FFFFFF2,
    TEE_SMC_OPEN_SESSION                = 0x0FFFFFF3,
    TEE_SMC_CLOSE_SESSION               = 0x0FFFFFF4,
    TEE_SMC_INVOKE_COMMAND              = 0x0FFFFFF5,
    TEE_SMC_NS_IRQ_CALL                 = 0x0FFFFFF6,
    TEE_SMC_NS_IRQ_DONE	                = 0x0FFFFFF7,
    TEE_SMC_NS_KERNEL_CALL              = 0x0FFFFFF8,
    TEE_SMC_SECURE_FIQ_CALL		= 0x0FFFFFF9,
    TEE_SMC_SECURE_FIQ_DONE		= 0x0FFFFFFA,
    TEE_SMC_CONFIG_SHMEM	        = 0x0FFFFFFB,
    TEE_SMC_RPC_CALL                    = 0x0FFFFFFC,
    TEE_SMC_RPC_RETURN                  = 0x0FFFFFFD,
	TEE_SMC_SST_COMMAND                 = 0x0FFFFF10,
};

enum {
    TE_PARAM_TYPE_NONE          = 0,
    TE_PARAM_TYPE_INT_RO        = 1,
    TE_PARAM_TYPE_INT_RW        = 2,
    TE_PARAM_TYPE_MEM_RO        = 3,
    TE_PARAM_TYPE_MEM_RW        = 4,
};

struct te_oper_param {
    uint32_t index;
    uint32_t type;
    union {
        struct {
                    uint32_t val_a;
					uint32_t val_b;
                } Int;
        struct {
                    void  *base;
                    void  *phys;
                    uint32_t len;
                } Mem;
    } u;
    void *next_ptr_user;
};

struct te_request {
    uint32_t		type;
    uint32_t		session_id;
    uint32_t		command_id;
    struct te_oper_param	*params;
    uint32_t		params_size;
    uint32_t		dest_uuid[4];
    uint32_t		result;
    uint32_t		result_origin;
};

/*communication structure*/
typedef struct command{
	unsigned int	cmd_type;
	unsigned int	cmd_param[4];
	unsigned char	*cmd_buf;
	unsigned int	cmd_buf_len ;

	unsigned int	resp_type;
	unsigned int	resp_ret[4];
	unsigned char   *resp_buf;
	unsigned int	resp_buf_len;

	unsigned int	status;
}cmd_t; 

enum{/*cmd type*/
	SST_CMD_POLLING =1 ,
	SST_CMD_BIND_REQ ,
	SST_CMD_WORK_DONE,
	SST_CMD_UPDATE_REQ,
	SST_CMD_INITIALIZED,
	SST_CMD_FUNC_TEST,
};

enum{/*response type*/
	SST_RESP_DUMMY = 1,
	SST_RESP_READ,
	SST_RESP_READ_DONE,
	SST_RESP_WRITE,
	SST_RESP_WRITE_DONE,
	SST_RESP_DELETE,
	SST_RESP_BIND_DONE,
	SST_RESP_UPDATE_DONE,
	SST_RESP_INITED_DONE, 
	SST_RESP_FUNC_TEST_DONE,
};

enum{/*response result*/
	SST_RESP_RET_SUCCESS = 0,
	SST_RESP_RET_FAIL,
};
enum{ /*cmd status*/
	SST_STAS_CMD_INIT =1,/*Nwd*/
	SST_STAS_CMD_RDY , /*Nwd*/
	SST_STAS_CMD_POST ,/*Nwd*/
	SST_STAS_CMD_GET,  /*Swd*/
	SST_STAS_CMD_DEAL,/*Swd*/
	SST_STAS_CMD_RET, /*Swd*/
	SST_STAS_CMD_DONE,/*Nwd*/
};

typedef enum TYPE{
	OEM_DATA = 1,
	USER_DATA,
}type_t;  

#define MAX_OEM_STORE_NUM       0x10
#define MAX_STORE_LEN 0xc00 /*3K payload*/

#define ALIGN_BUF(src, align) (((src) + (align) -1) &(~((align)-1)))

extern int sst_oem_item_id(char *item);
#endif

