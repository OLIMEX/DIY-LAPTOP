/*
 * head file
 * operation on ini file API used in kernel
 * parse ini file to mainkey and subkey value
 * Author: raymonxiu
 * 
 */

#ifndef __CFG__OP__H__
#define __CFG__OP__H__


#define LINE_MAX_CHAR_NUM 512
#define MAX_LINE_NUM      2048
#define MAX_NAME_LEN      32
#define MAX_VALUE_LEN     128
#define MAX_MAINKEY_NUM   64
#define MAX_SUBKEY_NUM    1024
#define INI_MAX_CHAR_NUM (LINE_MAX_CHAR_NUM * MAX_LINE_NUM)
#define BIN_MAX_SIZE			4096*4

enum cfg_item_type {
	CFG_ITEM_VALUE_TYPE_INVALID = 0,
	CFG_ITEM_VALUE_TYPE_INT,
	CFG_ITEM_VALUE_TYPE_STR,
};

enum cfg_key_flag {
  CFG_KEY_RELEASE,
  CFG_KEY_INIT,
};

struct cfg_item {
  int                 val;
  char                *str;
};

struct cfg_subkey {
  char                name[MAX_NAME_LEN];
  struct cfg_item      *value;
  enum cfg_item_type  type;
  enum cfg_key_flag   cfg_flag;
};

struct cfg_mainkey {
  char                name[MAX_NAME_LEN];
  struct cfg_subkey   *subkey[MAX_SUBKEY_NUM];
  char                *subkey_name[MAX_SUBKEY_NUM];
  char                *subkey_value[MAX_SUBKEY_NUM];
  int                 subkey_cnt;
  enum cfg_key_flag   cfg_flag;
};

struct cfg_section {
  struct cfg_mainkey  *mainkey[MAX_MAINKEY_NUM];
  char                *mainkey_name[MAX_MAINKEY_NUM];
  int                 mainkey_cnt;
  enum cfg_key_flag   cfg_flag;
};

int cfg_get_sections(char *buffer, char *sections[]);
int cfg_get_one_key_value(char *buffer, struct cfg_mainkey *scts, struct cfg_subkey *subkey);
int cfg_get_all_keys_value(char *buffer, struct cfg_mainkey *scts);
void cfg_subkey_init(struct cfg_subkey **subkey);
void cfg_subkey_release(struct cfg_subkey **subkey);
void cfg_mainkey_init(struct cfg_mainkey **mainkey, char **mainkey_name);
void cfg_mainkey_release(struct cfg_mainkey **mainkey, char **mainkey_name);
void cfg_section_init(struct cfg_section **cfg_sct);
void cfg_section_release(struct cfg_section **cfg_sct);
int cfg_read_ini(char *file_path, struct cfg_section **cfg_section);
int cfg_read_file(char *file_path, char *buf, size_t len);

int cfg_get_one_subkey(struct cfg_section *cfg_section, char *main, char *sub, struct cfg_subkey *subkey);

struct file* cfg_open_file(char *file_path);
int cfg_close_file(struct file *fp);
int cfg_write_file(	struct file* fp, char *buf, size_t len);
#endif //__CFG__OP__H__
