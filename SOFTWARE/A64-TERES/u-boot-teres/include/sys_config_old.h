#ifndef SYS_CONFIG_OLD_H_
#define SYS_CONFIG_OLD_H_


#define   SCRIPT_PARSER_OK                   (0)
#define   SCRIPT_PARSER_EMPTY_BUFFER         (-1)
#define   SCRIPT_PARSER_KEYNAME_NULL         (-2)
#define   SCRIPT_PARSER_DATA_VALUE_NULL      (-3)
#define   SCRIPT_PARSER_KEY_NOT_FIND         (-4)
#define   SCRIPT_PARSER_BUFFER_NOT_ENOUGH    (-5)

typedef enum
{
	SCIRPT_PARSER_VALUE_TYPE_INVALID = 0,
	SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD,
	SCIRPT_PARSER_VALUE_TYPE_STRING,
	SCIRPT_PARSER_VALUE_TYPE_MULTI_WORD,
	SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD
} script_parser_value_type_t;

typedef struct
{
	unsigned  main_key_count;
	unsigned  length;
	unsigned  version[2];
} script_head_t;

typedef struct
{
	char main_name[32];
	int  lenth;
	int  offset;
} script_main_key_t;

typedef struct
{
	char sub_name[32];
	int  offset;
	int  pattern;
} script_sub_key_t;

/* functions for early boot */
extern int sw_cfg_get_int(const char *script_buf, const char *main_key, const char *sub_key);
extern char *sw_cfg_get_str(const char *script_buf, const char *main_key, const char *sub_key, char *buf);

/* script operations */
extern int script_parser_init(char *script_buf);
extern int script_parser_exit(void);
extern unsigned script_get_length(void);
extern ulong script_parser_fetch_subkey_start(char *main_name);
extern int script_parser_fetch_subkey_next(ulong hd, char *sub_name, int value[], int *index);
extern int script_parser_fetch(char *main_name, char *sub_name, int value[], int count);
extern int script_parser_fetch_ex(char *main_name, char *sub_name, int value[],
               script_parser_value_type_t *type, int count);
extern int script_parser_patch(char *main_name, char *sub_name, void *str, int str_size);
extern int script_parser_subkey_count(char *main_name);
extern int script_parser_mainkey_count(void);
extern int script_parser_mainkey_get_gpio_count(char *main_name);
extern int script_parser_mainkey_get_gpio_cfg(char *main_name, void *gpio_cfg, int gpio_count);

extern ulong script_parser_subkey( script_main_key_t* main_name,char *subkey_name , uint *pattern);

extern int script_parser_patch_all(char *main_name, void *str, uint data_count);

#endif