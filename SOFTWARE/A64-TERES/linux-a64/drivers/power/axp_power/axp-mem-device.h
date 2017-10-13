#ifndef AXP_MEM_DEVICE_H
#define AXP_MEM_DEVICE_H
struct regulator {
	struct device *dev;
	struct list_head list;
	unsigned int always_on:1;
	unsigned int bypass:1;
	int uA_load;
	int min_uV;
	int max_uV;
	char *supply_name;
	struct device_attribute dev_attr;
	struct regulator_dev *rdev;
	struct dentry *debugfs;
};


typedef struct {
	unsigned int mem_data;
	char id_name[20];
}axp_mem_data_t;

#endif
