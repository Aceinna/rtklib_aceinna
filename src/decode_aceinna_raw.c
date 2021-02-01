#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

//#define MAIN_PROGRAM

#ifndef NEAM_HEAD
#define NEAM_HEAD 0x24 //'$'
#endif // !NEAM_HEAD
#define IMU_HEAD_1 0xD4
#define IMU_HEAD_2 0x34

#define ACEINNA_HEAD_SIZE 4
#define MAX_BUFFER_SIZE 1024
#define IMU_CONST_SIZE 23

#define TYPE_ROV 1
#define TYPE_IMU 2
#define TYPE_REF 3
#define TYPE_BAS 4

#define ROV_HEAD_LEN 8
#define REF_HEAD_LEN 7
#define BAS_HEAD_LEN 7

#pragma pack(push, 1)

 typedef struct  {
	 uint32_t buffer_len;
	 uint8_t buffer[MAX_BUFFER_SIZE];
	 uint8_t type;
	 uint8_t rov_index;
	 uint32_t data_len;
}aceinna_raw_t;

#pragma pack(pop)

static aceinna_raw_t raw = {0};
static FILE* aceinna_log_file = NULL;
static FILE* aceinna_rov1_file = NULL;
static FILE* aceinna_rov2_file = NULL;
static FILE* aceinna_rov3_file = NULL;
static FILE* aceinna_ref_file = NULL;
static FILE* aceinna_bas1_file = NULL;
static FILE* aceinna_imu_file = NULL;

static char aceinna_file_basename[256] = { 0 };

extern void set_aceinna_file_basename(char* input_name) {
#ifdef MAIN_PROGRAM
	char * pch;
	int size = 0;
	pch = strrchr(input_name, '.');
	size = pch - input_name;
	memcpy(aceinna_file_basename, input_name, size);
#else
	strcpy(aceinna_file_basename, input_name);
#endif
}

extern void open_aceinna_log_file() {
	char file_name[256] = { 0 };
	if (aceinna_log_file == NULL) {
		sprintf(file_name, "%s.log", aceinna_file_basename);
		aceinna_log_file = fopen(file_name, "wb");
	}
}

extern void close_aceinna_all_file() {
	if (aceinna_log_file)fclose(aceinna_log_file); aceinna_log_file = NULL;
	if (aceinna_rov1_file)fclose(aceinna_rov1_file); aceinna_rov1_file = NULL;
	if (aceinna_rov2_file)fclose(aceinna_rov2_file); aceinna_rov2_file = NULL;
	if (aceinna_rov3_file)fclose(aceinna_rov3_file); aceinna_rov3_file = NULL;
	if (aceinna_ref_file)fclose(aceinna_ref_file); aceinna_ref_file = NULL;
	if (aceinna_bas1_file)fclose(aceinna_bas1_file); aceinna_bas1_file = NULL;
	if (aceinna_imu_file)fclose(aceinna_imu_file); aceinna_imu_file = NULL;
}

static void write_aceinna_rov1_file(uint8_t *buff, uint32_t len) {
	char file_name[256] = { 0 };
	if (aceinna_rov1_file == NULL) {
		sprintf(file_name, "%s_rov1.rtcm", aceinna_file_basename);
		aceinna_rov1_file = fopen(file_name, "wb");
	}
	if (aceinna_rov1_file) fwrite(buff, 1, len, aceinna_rov1_file);
}

static void write_aceinna_rov2_file(uint8_t *buff, uint32_t len) {
	char file_name[256] = { 0 };
	if (aceinna_rov2_file == NULL) {
		sprintf(file_name, "%s_rov2.rtcm", aceinna_file_basename);
		aceinna_rov2_file = fopen(file_name, "wb");
	}
	if (aceinna_rov2_file) fwrite(buff, 1, len, aceinna_rov2_file);
}

static void write_aceinna_rov3_file(uint8_t *buff, uint32_t len) {
	char file_name[256] = { 0 };
	if (aceinna_rov3_file == NULL) {
		sprintf(file_name, "%s_rov3.rtcm", aceinna_file_basename);
		aceinna_rov3_file = fopen(file_name, "wb");
	}
	if (aceinna_rov3_file) fwrite(buff, 1, len, aceinna_rov3_file);
}

static void write_aceinna_ref_file(uint8_t *buff, uint32_t len) {
	char file_name[256] = { 0 };
	if (aceinna_ref_file == NULL) {
		sprintf(file_name, "%s_ref.rtcm", aceinna_file_basename);
		aceinna_ref_file = fopen(file_name, "wb");
	}
	if (aceinna_ref_file) fwrite(buff, 1, len, aceinna_ref_file);
}

static void write_aceinna_bas1_file(uint8_t *buff, uint32_t len) {
	char file_name[256] = { 0 };
	if (aceinna_bas1_file == NULL) {
		sprintf(file_name, "%s_bas1.rtcm", aceinna_file_basename);
		aceinna_bas1_file = fopen(file_name, "wb");
	}
	if (aceinna_bas1_file) fwrite(buff, 1, len, aceinna_bas1_file);
}

static void open_aceinna_imu_file() {
	char file_name[256] = { 0 };
	if (aceinna_imu_file == NULL) {
		sprintf(file_name, "%s_imu.imu", aceinna_file_basename);
		aceinna_imu_file = fopen(file_name, "w");
	}
}

static uint32_t rtcm_getbitu(const uint8_t *buff, uint32_t pos, uint32_t len)
{
	uint32_t ret = 0;
	uint32_t i;
	for (i = pos; i < (pos + len); i++)
	{
		uint32_t slen  = 7 - (i   % 8);
		if   ((slen   >= 0) && (slen   <= 8))
		{
			uint8_t   sval   = (buff[i   / 8] >> (uint8_t)slen)   &  (uint8_t)1;
			ret = (ret << 1) + (uint32_t)sval;
		}
	}
	return ret;
}

static int rtcm_getbits(const unsigned char *buff, int pos, int len)
{
	unsigned int bits = rtcm_getbitu(buff, pos, len);

	if (len <= 0 || 32 <= len || !(bits & (1u << (len - 1))))
		return (int)bits;

	return (int)(bits | (~0u << len)); /* extend sign */
}

static unsigned char crc8_chk_value(unsigned char *message, unsigned char len)
{
	unsigned char crc;
	unsigned char i;

	crc = 0;
	while (len--)
	{
		crc ^= *message++;
		for (i = 0; i < 8; i++)
		{
			if (crc & 0x01)
			{
				crc = (crc >> 1) ^ 0x8c;
			}
			else
			{
				crc >>= 1;
			}
		}
	}
	return crc;
}

static void decode_aceinna_imu(uint8_t* imumsg) {
	if (IMU_HEAD_1 == imumsg[0] && IMU_HEAD_2 == imumsg[1]) {
		uint8_t crc8, raw_crc;
		crc8 = crc8_chk_value(&imumsg[2], 20);
		raw_crc = imumsg[22];
		if (crc8 == raw_crc) {
			int i = 0, getdata = 0;
			double gga_time = 0, accel_g[3] = { 0 }, rate_dps[3] = { 0 };
			rtcm_getbitu(imumsg, i, 8); i += 8;
			rtcm_getbitu(imumsg, i, 8); i += 8;
			getdata = rtcm_getbitu(imumsg, i, 28); i += 28;
			gga_time = (double)getdata / 100.0;
			getdata = rtcm_getbits(imumsg, i, 20); i += 20;
			accel_g[0] = (double)getdata / (double)1e4;
			getdata = rtcm_getbits(imumsg, i, 20); i += 20;
			accel_g[1] = (double)getdata / (double)1e4;
			getdata = rtcm_getbits(imumsg, i, 20); i += 20;
			accel_g[2] = (double)getdata / (double)1e4;
			getdata = rtcm_getbits(imumsg, i, 24); i += 24;
			rate_dps[0] = (double)getdata / (double)1e4;
			getdata = rtcm_getbits(imumsg, i, 24); i += 24;
			rate_dps[1] = (double)getdata / (double)1e4;
			getdata = rtcm_getbits(imumsg, i, 24); i += 24;
			rate_dps[2] = (double)getdata / (double)1e4;
			if (aceinna_imu_file) fprintf(aceinna_imu_file, "$IMU, %09.2f,%8.4f,%8.4f,%8.4f,%8.4f,%8.4f,%8.4f \n", gga_time, accel_g[0], accel_g[1], accel_g[2], rate_dps[0], rate_dps[1], rate_dps[2]);
		}
	}
}

extern int input_aceinna_format_raw(uint8_t c) {
	int ret = 0;
	if (raw.buffer_len == 0) {
		if (c == NEAM_HEAD) {
			raw.buffer[raw.buffer_len++] = c;
		}
	}
	else {
		if (raw.buffer_len < ACEINNA_HEAD_SIZE) {
			raw.buffer[raw.buffer_len++] = c;
			if (raw.buffer_len == ACEINNA_HEAD_SIZE) {
				if (strncmp("$ROV", raw.buffer, ACEINNA_HEAD_SIZE) == 0) {
					raw.type = TYPE_ROV;
				}
				else if (strncmp("$REF", raw.buffer, ACEINNA_HEAD_SIZE) == 0) {
					raw.type = TYPE_REF;
				}
				else if (strncmp("$IMU", raw.buffer, ACEINNA_HEAD_SIZE) == 0) {
					raw.type = TYPE_IMU;
				}
				else if (strncmp("$BAS", raw.buffer, ACEINNA_HEAD_SIZE) == 0) {
					raw.type = TYPE_BAS;
				}
				else {
					raw.type = 0;
					if (aceinna_log_file)fprintf(aceinna_log_file, "%s\n", raw.buffer);
				}
			}
		}
		else {
			if (raw.type == TYPE_ROV) {
				raw.buffer[raw.buffer_len++] = c;
				if (raw.buffer_len == ROV_HEAD_LEN) {
					char str_bin_len[4] = { 0 };
					raw.rov_index = raw.buffer[ACEINNA_HEAD_SIZE] - 48;
					memcpy(str_bin_len, raw.buffer + ROV_HEAD_LEN-3, 3);
					raw.data_len = atoi(str_bin_len);
				}
				if (raw.data_len > 0 && raw.buffer_len > ROV_HEAD_LEN) {
					ret = raw.type;
				}
				if (raw.data_len > 0 && raw.buffer_len == ROV_HEAD_LEN + raw.data_len) {
					//write rov
					switch (raw.rov_index) {
					case 1:
						write_aceinna_rov1_file(raw.buffer + ROV_HEAD_LEN, raw.data_len);
						break;
					case 2:
						write_aceinna_rov2_file(raw.buffer + ROV_HEAD_LEN, raw.data_len);
						break;
					case 3:
						write_aceinna_rov3_file(raw.buffer + ROV_HEAD_LEN, raw.data_len);
						break;
					}
					memset(&raw, 0, sizeof(aceinna_raw_t));
				}
			}
			else if (raw.type == TYPE_REF)
			{
				raw.buffer[raw.buffer_len++] = c;
				if (raw.buffer_len == REF_HEAD_LEN) {
					char str_bin_len[4] = { 0 };
					memcpy(str_bin_len, raw.buffer + REF_HEAD_LEN-3, 3);
					raw.data_len = atoi(str_bin_len);
				}
				if (raw.data_len > 0 && raw.buffer_len > REF_HEAD_LEN) {
					ret = raw.type;
				}
				if (raw.data_len > 0 && raw.buffer_len == REF_HEAD_LEN + raw.data_len) {
					//write ref
					write_aceinna_ref_file(raw.buffer + REF_HEAD_LEN, raw.data_len);
					memset(&raw, 0, sizeof(aceinna_raw_t));
				}
			}
			else if (raw.type == TYPE_IMU)
			{
				raw.buffer[raw.buffer_len++] = c;
				if (raw.buffer_len == ACEINNA_HEAD_SIZE + IMU_CONST_SIZE) {
					//write imu
					open_aceinna_imu_file();
					decode_aceinna_imu(raw.buffer + ACEINNA_HEAD_SIZE);
					memset(&raw, 0, sizeof(aceinna_raw_t));
				}
			}
			else if (raw.type == TYPE_BAS)
			{
				raw.buffer[raw.buffer_len++] = c;
				if (raw.buffer_len == BAS_HEAD_LEN) {
					char str_bin_len[4] = { 0 };
					memcpy(str_bin_len, raw.buffer + BAS_HEAD_LEN - 3, 3);
					raw.data_len = atoi(str_bin_len);
				}
				if (raw.data_len > 0 && raw.buffer_len > BAS_HEAD_LEN) {
					ret = raw.type;
				}
				if (raw.data_len > 0 && raw.buffer_len == BAS_HEAD_LEN + raw.data_len) {
					//write ref
					write_aceinna_bas1_file(raw.buffer + BAS_HEAD_LEN, raw.data_len);
					memset(&raw, 0, sizeof(aceinna_raw_t));
				}
			}
			else {
				memset(&raw, 0, sizeof(aceinna_raw_t));
			}
		}
	}
	return  ret;
}

#ifdef  MAIN_PROGRAM

int main(int argc, char* argv[]) {
	//int  i;
	//for (i = 0; i < argc; i++) {
	//	printf("%s\n",argv[i]);
	//}
	if (argc >= 1) {
		char* filename = argv[1];
		printf("decode %s \n", argv[1]);

		char c = ' ';
		FILE* file = fopen(filename, "rb");
		if (file) {
			set_aceinna_file_basename(filename);
			//open_log_file();
			while (!feof(file)) {
				fread(&c, sizeof(char), 1, file);
				if (feof(file)) break;
				input_aceinna_format_raw(c);
			}
			close_aceinna_all_file();
			fclose(file);
		}
	}
	return  0;
}

#endif //  MAIN_PROGRAM

