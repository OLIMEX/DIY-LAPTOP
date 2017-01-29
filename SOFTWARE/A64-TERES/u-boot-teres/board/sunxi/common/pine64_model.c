
#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

int get_model_from_dram_size(char* model)
{
	phys_size_t l = 512 * 1024 * 1024;
	puts("get Pine64 model from DRAM size\n");
	if (gd->ram_size > l) {
		puts("DRAM >512M\n");
		sprintf(model, "pine64-plus");
	} else {
		puts("DRAM <= 512M\n");
		sprintf(model, "pine64");
	}

	return 0;
}

int pine64_set_model(void)
{
	char model[128] = {0};
	get_model_from_dram_size(model);

	printf("Pine64 model: %s\n", model);
	if(setenv("pine64_model", model))
	{
		printf("error:set variable [pine64_model] fail\n");
	}
	return 0;
}
