/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Allwinner secure storage data format 
 */
#include <common.h>
#include <command.h>
#include <asm-generic/errno.h>
#include <malloc.h>
#include <asm/io.h>
#include <securestorage.h>
#include "sprite_storage_crypt.h"

extern void sunxi_dump(void *addr, unsigned int size);

//#define SST_DEBUG
#ifdef SST_DEBUG
int __command_dump(cmd_t *cmd)
{

	printf("	->command\n ");	

	printf("		: cmd_tyep		= " );

	switch(cmd->cmd_type){
		case SST_CMD_POLLING:
			printf(" SST_CMD_POLLING\n");
			break;
		case SST_CMD_BIND_REQ:
			printf(" SST_CMD_BIND_REQ\n");
			break;
		case SST_CMD_WORK_DONE:
			printf(" SST_CMD_WORK_DONE\n");
			break;
		case SST_CMD_UPDATE_REQ:
			printf(" SST_CMD_UPDATE_REQ\n");
			break;
		case SST_CMD_INITIALIZED:
			printf(" SST_CMD_INITIALIZED\n");
			break;
		default:
			break;
	}

	printf("		: cmd_parma[0]	= 0x%x\n", cmd->cmd_param[0] );
	printf("		: cmd_parma[1]	= 0x%x\n", cmd->cmd_param[1] );
	printf("		: cmd_parma[2]	= 0x%x\n", cmd->cmd_param[2] );
	printf("		: cmd_parma[3]	= 0x%x\n", cmd->cmd_param[3] );
	printf("		: cmd_buf		= 0x%x\n", (unsigned int)(cmd->cmd_buf) );
	printf("		: cmd_buf_len	= 0x%x\n", cmd->cmd_buf_len );
	printf("		: resp_type		= "  );

	switch(cmd->resp_type){
		case SST_RESP_DUMMY:
			printf(" SST_RESP_DUMMY\n");
			break;
		case SST_RESP_READ:
			printf(" SST_RESP_READ\n");
			break;
		case SST_RESP_READ_DONE:
			printf(" SST_RESP_READ_DONE\n");
			break;
		case SST_RESP_WRITE:
			printf(" SST_RESP_WRITE\n");
			break;
		case SST_RESP_WRITE_DONE:
			printf(" SST_RESP_WRITE_DONE\n");
			break;		
		case SST_RESP_DELETE:
			printf(" SST_RESP_DELETE\n");
			break;
		case SST_RESP_BIND_DONE:
			printf(" SST_RESP_BIND_DONE\n");
			break;
		case SST_RESP_UPDATE_DONE:
			printf(" SST_RESP_UPDATE_DONE\n");
			break;
		case SST_RESP_INITED_DONE:
			printf(" SST_RESP_INITED_DONE\n");
			break;
		default:
			printf("\n");
			break;
	}

	printf("		: resp_ret[0]	= 0x%x\n", cmd->resp_ret[0] );
	printf("		: resp_ret[1]	= 0x%x\n", cmd->resp_ret[1] );
	printf("		: resp_ret[2]	= 0x%x\n", cmd->resp_ret[2] );
	printf("		: resp_ret[3]	= 0x%x\n", cmd->resp_ret[3] );
	printf("		: resp_buf		= 0x%x\n", (unsigned int)(cmd->resp_buf) );
	printf("		: resp_buf_len	= 0x%x\n", cmd->resp_buf_len );

	return (0);
}

void __param_dump(struct te_oper_param *param)
{
	printf("Param dump :0x%x\n", (unsigned int)param );	
	printf("	param->index = %d\n", param->index);
	printf("	param->type = 0x%x\n", param->type);
	printf("	param->Mem.base = 0x%x\n", (unsigned int)param->u.Mem.base);
	printf("	param->Mem.phys= 0x%x\n", (unsigned int)param->u.Mem.phys);
	printf("	param->Mem.len= 0x%x\n", param->u.Mem.len);
	return ;
}

void __request_dump(struct te_request *r )
{
	printf("Request dump :0x%x\n", (unsigned int)r);
	printf("	request->type = 0x%x\n", r->type);
	printf("	request->session_id = 0x%x\n", r->session_id);
	printf("	request->command_id = 0x%x\n", r->command_id);
	printf("	request->params = 0x%x\n", (unsigned int)(r->params));
	printf("	request->params_size = 0x%x\n", r->params_size);
}
#else

void __request_dump(struct te_request *r )
{
	return ;
}	

void __param_dump(struct te_oper_param *param)
{	
	return ;
}

int __command_dump(cmd_t *cmd)
{	
	return 0;
}
#endif

#ifdef CONFIG_SUNXI_SECURE_SYSTEM
static  char oem_class[MAX_OEM_STORE_NUM][64] ;
static char *align_resp_buf = NULL, 
			*align_cmd_buf = NULL ;

static int parse_resp(
		struct te_request * request, 
		struct te_oper_param  *param, 
		cmd_t *cmd
		)
{
	__command_dump(cmd);
	if(cmd->resp_buf != NULL )
		memcpy(cmd->resp_buf, align_resp_buf, cmd->resp_buf_len);

	return 0 ;
}

static int package_cmd(
		struct te_request * request, 
		struct te_oper_param  *param, 
		cmd_t *cmd
		)
{
	unsigned int i =0 ;

	__command_dump(cmd);

	char *cmd_buf, 
		 *resp_buf ;

	if(!align_cmd_buf || !align_resp_buf ){
		cmd_buf = malloc(SZ_8K);	
		resp_buf = malloc(SZ_8K);
		if(!cmd_buf || !resp_buf){
			printf("smc load sst out of memory\n");
			return -1 ;
		}

		align_cmd_buf = (char *)ALIGN_BUF((unsigned int)cmd_buf, SZ_4K ) ;
		align_resp_buf= (char *)ALIGN_BUF((unsigned int)resp_buf,SZ_4K) ;
	}


	memset(request,0, sizeof(*request))	;
	memset(param,0, sizeof(*param))	;

	request->type = TEE_SMC_SST_COMMAND ; 
	request->params = param ;
	request->params_size = 3 ; 
	request->params->index = i =0 ;

	param[0].type =  (uint32_t)TE_PARAM_TYPE_MEM_RW;
	param[0].u.Mem.base = cmd;
	param[0].u.Mem.phys =(void *)virt_to_phys(cmd);
	param[0].u.Mem.len= sizeof(*cmd);

	i ++;
	memset(&param[i],0, sizeof(*param));
	param[i].index = i ;
	if( cmd->cmd_buf != NULL ){
		memcpy(align_cmd_buf, cmd->cmd_buf,cmd->cmd_buf_len );
		param[i].type = TE_PARAM_TYPE_MEM_RO;
		param[i].u.Mem.base = align_cmd_buf;
		param[i].u.Mem.phys = (void *)virt_to_phys(align_cmd_buf);
		param[i].u.Mem.len= cmd->cmd_buf_len;

	}

	i ++;
	memset(&param[i],0, sizeof(*param));
	param[i].index = i ;
	if( cmd->resp_buf != NULL ){
		param[i].type = (uint32_t)TE_PARAM_TYPE_MEM_RW;
		param[i].u.Mem.base = align_resp_buf;
		param[i].u.Mem.phys =(void *)virt_to_phys(align_resp_buf);
		param[i].u.Mem.len= cmd->resp_buf_len;
	}

	__request_dump(request);
	for(i =0 ;i< 3; i++)
		__param_dump(&param[i]);

	return 0;
}

extern u32 sunxi_smc_call(u32 arg0, u32 arg1, u32 arg2, u32 arg3);
static int post_cmd_wait_resp( cmd_t *cmd )
{

	int ret = -1 ;
	struct te_request *request;
	struct te_oper_param *param;

	request = malloc(sizeof(*request));
	param = malloc(sizeof(*param));
	if(!request || !param){
		printf("post cmd out of memory\n");
		return -1 ;
	}

	package_cmd(request,param, cmd);
	
	/* Send request to Swd and wait */
	printf("begin to enter smc\n");
	ret =sunxi_smc_call(
			TEE_SMC_SST_COMMAND,
			virt_to_phys(request), 
			virt_to_phys(param),
			0
			);

	printf("after smc\n");
	/* Parse the return from Swd */
	parse_resp(request, param, cmd);

	free(request);
	free(param);
	return ret  ; 
}

int sst_oem_item_id(char *item)
{
	int id ;
	for(id =0; id <MAX_OEM_STORE_NUM; id ++ ){
		if( oem_class[id][0] != 0 && \
			!strncmp(item, oem_class[id],strnlen(item, 64) )){
			return id ;
		}
	}
	return -1 ;
}

static int sst_cmd_binding_store(
		unsigned int id ,
		char *src, 
		unsigned int len,
		char *dst,
		unsigned int *retLen
		)
{
	cmd_t *cmd ; 
	int ret = -1 ;

	cmd = malloc(sizeof(*cmd));
	if(!cmd){
		printf("sst_cmd_binding_store out of memory\n");
		return -1 ;
	}
	memset(cmd, 0, sizeof(*cmd));

	do{
		/* Binding command */
		memset(cmd, 0 ,sizeof(*cmd)) ;
		cmd->cmd_type		= SST_CMD_BIND_REQ;
		cmd->cmd_param[0]	= OEM_DATA ;
		cmd->cmd_param[1]	= id ;
		cmd->cmd_buf		= (unsigned char *)src ;
		cmd->cmd_buf_len	= len ;

		cmd->resp_buf		=(unsigned char *)dst;
		cmd->resp_buf_len	= MAX_STORE_LEN;
		cmd->resp_ret[0]	= -1 ;
		cmd->status			= SST_STAS_CMD_RDY;
	
		/* Send the command */
		ret =  post_cmd_wait_resp(cmd );
		if( ret != 0 ){
			printf(" command procedure error 0x%x\n", ret);
			free(cmd);
			return (- 1);
		}

		/* Check response */
		if( cmd->resp_ret[0] != SST_RESP_RET_SUCCESS ){
			printf(" cmd response fail:0x%x error\n", cmd->resp_ret[0]);
			free(cmd);
			return (- 1 );
		}
		if( cmd->status != SST_STAS_CMD_RET ){
			printf(" cmd status:0x%x error\n", cmd->status);
			free(cmd);
			return (- 1 );
		}

		if( cmd->resp_type != SST_RESP_BIND_DONE )	{
			printf(" bind error in Swd:\
					len = 0x%x , response type =0x%x \n",
					cmd->resp_buf_len, cmd->resp_type);
			free(cmd);
			return (- 1);
		}
		*retLen = cmd->resp_ret[1];

	} while( 0 ) ;

	free(cmd);
	return 0;
}

static int sst_cmd_update_object(
		unsigned int type ,
		unsigned int id ,
		char *src, 
		unsigned int len,
		char * name
		)
{
	cmd_t *cmd ; 
	int ret = -1 ;
	unsigned char *name_buf ;

	cmd = malloc(sizeof(*cmd));
	name_buf = malloc(strnlen(name, 64));
	if(!cmd || !name_buf){
		printf("sst_cmd_binding_store out of memory\n");
		return -1 ;
	}
	memset(cmd, 0, sizeof(*cmd));

	do{

		memset(cmd, 0 ,sizeof(*cmd)) ;
		cmd->cmd_type		= SST_CMD_UPDATE_REQ;
		cmd->cmd_param[0]	= type ;
		cmd->cmd_param[1]	= id ;
		cmd->cmd_buf		= (unsigned char *)src ;
		cmd->cmd_buf_len	= len ;

		cmd->resp_ret[0]	= -1 ;
		cmd->status			= SST_STAS_CMD_RDY;

		if(name != NULL && type == OEM_DATA ){
			memcpy(name_buf, name, strnlen(name,64));
			cmd->resp_buf		= (unsigned char *)name_buf ;
			cmd->resp_buf_len	= strnlen(name,64) ;
		}

		/* Send the command */
		ret =  post_cmd_wait_resp(cmd );
		if( ret != 0 ){
			printf(" command procedure error 0x%x\n", ret);
			free(cmd);
			free(name_buf);
			return (- 1);
		}

		/* Check response */
		if( cmd->resp_ret[0] != SST_RESP_RET_SUCCESS ){
			printf(" cmd response fail:0x%x error\n", cmd->resp_ret[0]);
			free(cmd);
			free(name_buf);
			return (- 1 );
		}
		if( cmd->status != SST_STAS_CMD_RET ){
			printf(" cmd status:0x%x error\n", cmd->status);
			free(cmd);
			free(name_buf);
			return (- 1 );
		}
		if( cmd->resp_type != SST_RESP_UPDATE_DONE )	{
			printf(" update error in Swd:\
					len = 0x%x , response type =0x%x \n",
					cmd->resp_buf_len, cmd->resp_type);
			free(cmd);
			free(name_buf);
			return (- 1);
		}

	} while( 0 ) ;

	free(cmd);
	free(name_buf);
	return 0;
}

int smc_load_sst_encrypt(
		char *name,
		char *in, unsigned int len,
		char *out, unsigned int *outLen)
{
	int id ,ret ;

	if( (id = sst_oem_item_id(name)) <0 )
		return 1 ;
	
	ret = sst_cmd_binding_store((unsigned int)id, in , len, out, outLen );

	return ret ;
}

int smc_load_sst_decrypt(char *name, char *in, unsigned int len)
{
	int id ,ret ;

	if( (id = sst_oem_item_id(name)) <0 )
		return 1 ;

	ret = sst_cmd_update_object(OEM_DATA ,(unsigned int)id, in , len, name );

	return ret ;
}

int smc_set_sst_crypt_name(char *name)
{
	int id ;
	
	if( (id = sst_oem_item_id(name)) >0 )
		return 0 ;/*The name had been set*/

	printf("set name[%s] to encrypt list\n", name);

	for(id =0; id <MAX_OEM_STORE_NUM; id ++ ){
		if( oem_class[id][0] == 0 ){
			memcpy(oem_class[id],name, strnlen(name, 64) );
			break ;
		}
	}

	return 0 ;
}

int smc_load_sst_test(void)
{
	char buffer[4096],
		 en_buffer[4096];
	unsigned int retLen  ;
	store_object_t *so ;
	int ret ;

	/*call to encrypt data */
	ret = sunxi_secure_storage_read("Widevine", buffer, 4096, (int *)&retLen);
	if(ret <0 ){
		printf("read Widevine secure object fail\n");
		return -1 ;
	}

	printf("Widevine data: \n");
	sunxi_dump(buffer, retLen);

	so = (store_object_t *)buffer;
	printf("so name %s\n",so->name);
	ret = smc_load_sst_encrypt("Widevine",
			(char *)so->data,so->actual_len, 
			en_buffer, &retLen);
	if(ret <0){
		printf("smc load sst encrypt fail\n");
		return -1 ;
	}

	printf("Encrypt Widevine data: \n");
	sunxi_dump(en_buffer, retLen);

	memcpy(so->data, en_buffer, retLen);
	so->actual_len = retLen;
	so->re_encrypt = STORE_REENCRYPT_MAGIC;
	so->crc = crc32( 0 , (void *)so, sizeof(*so)-4 ); 

	printf("so name %s\n",so->name);
	/*call to decrypt data to secure memory*/
	ret = smc_load_sst_decrypt(so->name, (char *)so->data,so->actual_len);
	if(ret <0){
		printf("smc_load sst decrypt fail\n");
		return -1 ;
	}
	return 0 ;

}

#endif
