#ifndef INI_H
#define INI_H

#define MAX_CFG_BUF       512
#define SUCCESS				0
/* return value */
#define CFG_OK						  SUCCESS
#define CFG_SECTION_NOT_FOUND         -1 
#define CFG_KEY_NOT_FOUND             -2 
#define CFG_ERR                       -10 

#define CFG_ERR_OPEN_FILE             -10 
#define CFG_ERR_CREATE_FILE           -11 
#define CFG_ERR_READ_FILE             -12 
#define CFG_ERR_WRITE_FILE            -13 
#define CFG_ERR_FILE_FORMAT           -14 


#define CFG_ERR_EXCEED_BUF_SIZE       -22 

#define COPYF_OK                      SUCCESS 
#define COPYF_ERR_OPEN_FILE           -10 
#define COPYF_ERR_CREATE_FILE         -11 
#define COPYF_ERR_READ_FILE           -12 
#define COPYF_ERR_WRITE_FILE          -13 


struct ini_key_location {
	int ini_section_line_no;
	int ini_key_line_no;
	int ini_key_lines;
};


int ini_get_key(char *filedata, char * section, char * key, char * value);
int ini_get_sections(char *filedata, unsigned char * sections[], int max);

int  ini_split_section(char *section, char **name, char **index);
//int  ini_join_section(char **section, char *name, char *index);

int atoi(char *nptr);

#endif
