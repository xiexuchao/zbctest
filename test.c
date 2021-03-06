#define _GNU_SOURCE
#include <libzbc/zbc.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/time.h>
#include <aio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <errno.h>
#include <pthread.h>

#define MEM_ALIGN				512  // Memory alignment
#define BYTE_PER_BLOCK			512 
#define LARGEST_REQUEST_SIZE	10000  // Largest request size in blocks

int main()
{
	struct zbc_device_info info;
    struct zbc_device *dev;
    
    struct zbc_zone *z,*zones=NULL;
    char path[64]="/dev/sdb";
    
    enum zbc_reporting_options ro = ZBC_RO_ALL;
    unsigned long long lba=0;
    unsigned int nr_zones;
    int nz=100;
    
    int i,ret,fd;
	struct aiocb *cb;
	char *buf;
	
	if (posix_memalign((void**)&buf, MEM_ALIGN, LARGEST_REQUEST_SIZE * BYTE_PER_BLOCK))
	{
		printf("Error allocating buffer \n");
		exit(-1);
	}
	
	for(i=0;i<LARGEST_REQUEST_SIZE*BYTE_PER_BLOCK;i++)
	{
		buf[i]=(char)(rand()%26+65);
	}

	fd=open(path, O_DIRECT | O_SYNC | O_RDWR);
	if(fd<0)
	{
		printf("Open Device Error!\n");
		exit(-1);
	}
	
    cb=(struct aiocb *)malloc(sizeof(struct aiocb));
    memset(cb,0,sizeof(struct aiocb));
	cb->aio_fildes=fd;
	cb->aio_buf=buf;
	cb->aio_nbytes=512*1000;
	cb->aio_offset=(long long)33554432*512;
	
	ret=aio_write(cb);
	if(ret)
	{
		printf("aio_write error!\n");
		exit(-1);
	}
	
	
	
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

    zones = (zbc_zone_t *) malloc(sizeof(zbc_zone_t) * nz);
    if ( ! zones  ) {
            fprintf(stderr, "No memory\n");
            ret = 1;
            goto out;
            
    }
        memset(zones, 0, sizeof(zbc_zone_t) * nz);

        /* Get zone information */
        ret = zbc_report_zones(dev, lba, ro, zones, &nz);
    if ( ret != 0  ) {
            fprintf(stderr, "zbc_list_zones failed %d\n", ret);
            ret = 1;
            goto out;
            
    }

        printf("%u / %u zone%s:\n", nz, nr_zones, (nz > 1) ? "s" : "");
    for(i = 0; i <=65; i++) {
                z = &zones[i];
        if ( zbc_zone_conventional(z)  ) {
                        printf("Zone %05d: type 0x%x (%s), cond 0x%x (%s), LBA %llu, %llu sectors, wp N/A\n",
                                                 i,
                                                 zbc_zone_type(z),
                                                 zbc_zone_type_str(zbc_zone_type(z)),
                                                 zbc_zone_condition(z),
                                                 zbc_zone_condition_str(zbc_zone_condition(z)),
                                                 zbc_zone_start_lba(z),
                                                 zbc_zone_length(z));
                    
        } else {
                        printf("Zone %05d: type 0x%x (%s), cond 0x%x (%s), need_reset %d, non_seq %d, LBA %llu, %llu sectors, wp %llu\n",
                                                 i,
                                                 zbc_zone_type(z),
                                                 zbc_zone_type_str(zbc_zone_type(z)),
                                                 zbc_zone_condition(z),
                                                 zbc_zone_condition_str(zbc_zone_condition(z)),
                                                 zbc_zone_need_reset(z),
                                                 zbc_zone_non_seq(z),
                                                 zbc_zone_start_lba(z),
                                                 zbc_zone_length(z),
                                                 zbc_zone_wp_lba(z));
                    
        }
            
    }

    //get the number of zones
    /*
    ret=zbc_report_nr_zones(dev, lba, ro, &nr_zones);
    if(ret !=0)
    {
        printf("report number error !\n");
        exit(-1);
    }
    */

out:

    zbc_close(dev);	
    
    close(fd);

	return 1;
}
