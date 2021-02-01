#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#ifndef NEAM_HEAD
#define NEAM_HEAD 0x24
#endif // !NEAM_HEAD

#define HEAD_SIZE 6
#define HEAD_TYPES 4
#define RTCM_FLAG_SIZE 20
#define MAX_HEAD_SIZE 24
#define MAX_IMU_SIZE 256
const char* aceinna_NMEA_headers[HEAD_TYPES] = {"$GPGGA","$GPIMU","$GPROV","$GPREF"};

#pragma pack(push, 1)

typedef struct {
	uint32_t header_len;
	uint8_t header[MAX_HEAD_SIZE];
	uint32_t ntype;
	uint32_t dot_num;
	uint32_t rtcm_len;
	uint32_t rtcm_bytes;
	int8_t imu[MAX_IMU_SIZE];
	uint32_t imu_len;
}aceinnaRaw;

#pragma pack(pop)

aceinnaRaw aceinna_raw = { 0 };

FILE* frov = NULL;
FILE* fref = NULL;
FILE* fleft = NULL;
FILE* fimu = NULL;

uint8_t last_data = 0;

char base_aceinna_file_name[256] = { 0 };
void clear_aceinna_struct()
{
    memset(&aceinna_raw,0,sizeof(aceinna_raw));
}

extern void set_base_aceinna_file_name(char* file_name)
{
	strcpy(base_aceinna_file_name, file_name);
}

extern void close_aceinna_log_file() {
	if (frov)fclose(frov);frov=NULL;
	if (fref)fclose(fref);fref=NULL;
	if (fleft)fclose(fleft);fleft=NULL;
	if (fimu)fclose(fimu);fimu=NULL;
	clear_aceinna_struct();
}

void write_rov_file(uint8_t data){
	char file_name[256] = { 0 };
	if (frov == NULL){
		sprintf(file_name, "%s_rov.rtcm", base_aceinna_file_name);
		frov = fopen(file_name, "wb");
	}
	if (frov) fwrite(&data,1,1,frov);
}

void write_ref_file(uint8_t data){
	char file_name[256] = { 0 };
	if (fref == NULL){
		sprintf(file_name, "%s_ref.rtcm", base_aceinna_file_name);
		fref = fopen(file_name, "wb");
	}
	if (fref) fwrite(&data,1,1,fref);
}

void write_left_file(uint8_t data){
	char file_name[256] = { 0 };
	if (fleft == NULL){
		sprintf(file_name, "%s_left.rtcm", base_aceinna_file_name);
		fleft = fopen(file_name, "wb");
	}
	if (fleft) fwrite(&data,1,1,fleft);
}

void write_imu_file(char* str){
	char file_name[256] = { 0 };
	if (fimu == NULL){
		sprintf(file_name, "%s.imu", base_aceinna_file_name);
		fimu = fopen(file_name, "w");
	}
	if (fimu) fprintf(fimu,str);
}

extern int input_aceinna_raw(uint8_t data,int* ntype){
	int ret = 0;
	if(aceinna_raw.header_len == 0){
		if (data == NEAM_HEAD && last_data == '\n'){
		   aceinna_raw.header[aceinna_raw.header_len++] = data;
		}
	}
	else{
		if(aceinna_raw.header_len <= RTCM_FLAG_SIZE){
			aceinna_raw.header[aceinna_raw.header_len++] = data;
			if (aceinna_raw.header_len == HEAD_SIZE) {
				int i;
				for(i = 0;i < HEAD_TYPES;i++){
					if (strncmp(aceinna_NMEA_headers[i],aceinna_raw.header,HEAD_SIZE) == 0){
						aceinna_raw.ntype = i+1;
						break;
					}
				}
				if(aceinna_raw.ntype == 0){
					clear_aceinna_struct();
				}
			}
		}
		else{
            aceinna_raw.header_len++;
        }
	}

	if(aceinna_raw.ntype > 0){
		*ntype = aceinna_raw.ntype;
		switch (aceinna_raw.ntype) {
		case 1: //$GPGGA
		case 2: //$GPIMU
		{
			if(aceinna_raw.ntype == 2 && data != 'U')
			{
				aceinna_raw.imu[aceinna_raw.imu_len++] = data;
			}
			if (data == '\n'){
				write_imu_file(aceinna_raw.header);
				write_imu_file(aceinna_raw.imu);
				clear_aceinna_struct();
			}
		}
		break;
		case 3: //$GPROV
		case 4: //$GPREF
		{
			if(last_data == ','){
				aceinna_raw.dot_num++;
			}
			if(aceinna_raw.rtcm_len == 0 && aceinna_raw.dot_num == 3){
				char dest[4] = {0};
				strncpy(dest, aceinna_raw.header+aceinna_raw.header_len-6, 4);
				aceinna_raw.rtcm_len = atoi(dest);
				aceinna_raw.rtcm_len -= 5;
				if(aceinna_raw.rtcm_len <= 0){
					clear_aceinna_struct();
				}
            }
			if(aceinna_raw.rtcm_len > 0){
                aceinna_raw.rtcm_bytes++;
                ret = 1;
				if(aceinna_raw.ntype == 3){
					write_rov_file(data);
				}
				else if(aceinna_raw.ntype == 4){
					write_ref_file(data);
                }
				if(aceinna_raw.rtcm_len-1 == aceinna_raw.rtcm_bytes){
					clear_aceinna_struct();
				}
            }
		}
		break;
		}
	}
	if(ret == 0){
		//write_left_file(data);
	}
	last_data = data;
	return ret;
}
