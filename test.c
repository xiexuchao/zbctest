#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <libzbc/zbc.h>

int main()
{
	struct zbc_device_info info;
    struct zbc_device *dev;
    
    struct zbc_zone *z,*zones=NULL;
    int i, ret=1;
    char *path;
    enum zbc_reporting_options ro = ZBC_RO_ALL;
    char path[64]="/dev/sdb";
    /*
    unsigned long long sector = 0, nr_sectors = 0;
        
        unsigned int nr_zones = 0, nz = 0;
        struct zbc_zone *z, *zones = NULL;
        unsigned int lba_unit = 0;
        unsigned long long start = 0;
        int i, ret = 1;
        int num = 0;
        
	*/
	ret=zbc_open(path, O_RDONLY, &dev);
	if(ret!=0)
	{
		printf("open ERROR!\n");
		return 1;
	}
	zbc_get_device_info(dev,&info);
	zbc_print_device_info(&info,stdout);
	
	
	

	return 1;
}
