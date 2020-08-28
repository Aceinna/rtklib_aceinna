#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#include <string.h>

const uint16_t debugPacketsTypeList[5] = { 177, 268, 42, 99, 507 };
const uint8_t DEBUG_PACKET_HEAD[4] = {0xAA, 0x44, 0x12, 0x1C};

#pragma pack(push, 1)
typedef struct {
	uint8_t state;
	uint8_t head_length;
	uint32_t payload_lenth;
	uint16_t messageid;
	uint32_t nbyte;
	uint8_t buff[256];

    double pos_llh[3];
	double latitude;
	double longitude;
	double height;
}debugRaw;

typedef struct {
	uint16_t time1;
	uint32_t time2;
}timeTag;

typedef struct {
	uint32_t GPS_Week;
	double GPS_TimeofWeek;
	uint8_t mode;
	double speed;
	uint8_t fwd;
	uint64_t wheel_tick;
}debugOdo;

typedef struct {
	uint32_t GPS_Week;
	double GPS_TimeofWeek;
	uint32_t imuStatus;
	float z_accel;
	float y_accel;
	float x_accel;
	float z_gyro;
	float y_gyro;
	float x_gyro;
}debugImu;

typedef struct {
	uint32_t solution_status;
	uint32_t position_type;
	double latitude;
	double longitude;
	double height;
	float undulation;
	uint32_t datum_id;
	float latitude_standard_deviation;
	float longitude_standard_deviation;
	float height_standard_deviation;
	int8_t base_station_id[4];
	float differential_age;
	float solution_age;
	uint8_t number_of_satellites;
	uint8_t number_of_satellites_in_solution;
	uint8_t num_gps_plus_glonass_l[2];
	uint8_t reserved;
	uint8_t extended_solution_status;
	uint8_t reserved2;
	uint8_t signals_used_mask;
}debugGnss;

typedef struct {
	uint32_t solution_status;
	uint32_t position_type;
	float latency;
	float age;
	double horizontal_speed;
	double track_over_ground;
	double vertical_speed;
	float reserved;
}debugVel;

typedef struct {
	uint32_t GPS_Week;
	double GPS_TimeofWeek;
	double latitude;
	double longitude;
	double height;
	double north_velocity;
	double east_velocity;
	double up_velocity;
	double roll;
	double pitch;
	double azimuth;
	int32_t status;
}debugIns;
#pragma pack(pop)

debugRaw debug_raw = { 0 };
timeTag time_tag = { 0 };
int packer_num = 0;
FILE* fALL = NULL;
FILE* fOdo = NULL;
FILE* fImu = NULL;
FILE* fGnss = NULL;
FILE* fVel = NULL;
FILE* fIns = NULL;
//FILE* fCsv[5] = { NULL };
char base_debug_file_name[256] = { 0 };
void set_base_debug_file_name(char* file_name)
{
	strcpy(base_debug_file_name, file_name);
}

void write_debug_log_file(int index, char* log) {
	char file_name[256] = { 0 };
	if (fALL == NULL) {
		sprintf(file_name, "%s_all.txt", base_debug_file_name);
		fALL = fopen(file_name, "w");
	}
	switch (index)
	{
	case 1:
	{
		if (fOdo == NULL) {
			sprintf(file_name, "%s_obo.csv", base_debug_file_name);
			fOdo = fopen(file_name, "w");
			if (fOdo) fprintf(fOdo, "GPS_Week,GPS_TimeofWeek,mode,speed,fwd,wheel_tick\n");
		}
		if (fOdo) fprintf(fOdo, log);
		if (fALL) fprintf(fALL, "$ODO,%s",log);
	}
	break;
	case 2:
	{
		if (fImu == NULL) {
			sprintf(file_name, "%s_imu.csv", base_debug_file_name);
			fImu = fopen(file_name, "w");
			if (fImu) fprintf(fImu, "GPS_Week,GPS_TimeofWeek,Time_Stamped,x_accel,y_accel,z_accel,x_gyro,y_gyro,z_gyro\n");
		}
		if (fImu) fprintf(fImu, log);
		if (fALL) fprintf(fALL, "$GPIMU,%s", log);
	}
	break;
	case 3:
	{
		if (fGnss == NULL) {
			sprintf(file_name, "%s_gnss.csv", base_debug_file_name);
			fGnss = fopen(file_name, "w");
			if (fGnss) fprintf(fGnss, "GPS_Week,GPS_TimeofWeek,latitude,longitude,height,latitude_standard_deviation,longitude_standard_deviation,height_standard_deviation,position_type\n");
		}
		if (fGnss) fprintf(fGnss, log);
		if (fALL) fprintf(fALL, "$GPGNSS,%s", log);
	}
	break;
	case 4:
	{
		if (fVel == NULL) {
			sprintf(file_name, "%s_vel.csv", base_debug_file_name);
			fVel = fopen(file_name, "w");
			if (fVel) fprintf(fVel, "GPS_Week,GPS_TimeofWeek,horizontal_speed,track_over_ground,vertical_speed\n");
		}
		if (fVel) fprintf(fVel, log);
		if (fALL) fprintf(fALL, "$GPVEL,%s", log);
	}
	break;
	case 5:
	{
		if (fIns == NULL) {
			sprintf(file_name, "%s_ins.csv", base_debug_file_name);
			fIns = fopen(file_name, "w");
			if (fIns) fprintf(fIns, "GPS_Week,GPS_TimeofWeek,latitude,longitude,height,north_velocity,east_velocity,up_velocity,roll,pitch,azimuth,status\n");
		}
		if (fIns) fprintf(fIns, log);
		if (fALL) fprintf(fALL, "$GPINS,%s", log);
	}
	break;
	}
}

void close_debug_log_file() {
	if (fALL != NULL) fclose(fALL);
	if (fOdo != NULL) fclose(fOdo);
	if (fImu != NULL) fclose(fImu);
	if (fGnss != NULL) fclose(fGnss);
	if (fVel != NULL) fclose(fVel);
	if (fIns != NULL) fclose(fIns);
}

uint32_t calc_32value(uint32_t value) {
	uint32_t ulCRC = value;
	int i = 0;
	for (i = 0; i < 8; i++) {
		if (ulCRC & 1) {
			ulCRC = (ulCRC >> 1) ^ 0xEDB88320;
		}
		else {
			ulCRC = ulCRC >> 1;
		}
	}
	return ulCRC;
}

uint32_t calc_block_crc32(uint8_t* buff,uint32_t nbyte) {
	uint32_t ulCRC = 0;
	int i = 0;
	for (i = 0; i < nbyte; i++) {
		uint32_t ulTemp1 = (ulCRC >> 8) & 0x00FFFFFF;
		uint32_t ulTemp2 = calc_32value((ulCRC ^ buff[i]) & 0xff);
		ulCRC = ulTemp1 ^ ulTemp2;
	}
	return ulCRC;
}

void parse_debug_packet_payload(char* out_msg) {
	uint8_t* playload = debug_raw.buff + debug_raw.head_length;
	char log_str[1024] = { 0 };
	if(out_msg == NULL) return;
	switch (debug_raw.messageid)
	{
	case 177: 
	{
		debugOdo odo = { 0 };
		size_t packet_size = sizeof(debugOdo);
		memcpy(&odo, playload, debug_raw.payload_lenth);
		sprintf(log_str, "%d,%11.4f,%d,%8.4f,%d,%I64d\n", odo.GPS_Week, odo.GPS_TimeofWeek, odo.mode, odo.speed, odo.fwd, odo.wheel_tick);
		//write_debug_log_file(1, log_str);
		sprintf(out_msg, "$ODO,%s",log_str);
	}
	break;
	case 268://imu
	{
		debugImu imu = { 0 };
		size_t packet_size = sizeof(debugImu);
		memcpy(&imu, playload, debug_raw.payload_lenth);
		sprintf(log_str,"%d,%11.4f,%10.4f,%14.10f,%14.10f,%14.10f,%14.10f,%14.10f,%14.10f\n", imu.GPS_Week, imu.GPS_TimeofWeek/1000,(float)time_tag.time2/1000,
			imu.x_accel*9.7803267714e0,imu.y_accel*9.7803267714e0,imu.z_accel*9.7803267714e0, imu.x_gyro, imu.y_gyro, imu.z_gyro);
		//write_debug_log_file(2, log_str);
		sprintf(out_msg, "$GPIMU,%s",log_str);
	}
	break;
	case 42:
	{
		debugGnss gnss = { 0 };
		size_t packet_size = sizeof(debugGnss);
		memcpy(&gnss, playload, debug_raw.payload_lenth);
		sprintf(log_str,"%d,%11.4f,%14.9f,%14.9f,%10.4f,%10.4f,%10.4f,%10.4f,%d\n",time_tag.time1,(float)time_tag.time2/1000, gnss.latitude,gnss.longitude,gnss.height,
			gnss.latitude_standard_deviation, gnss.longitude_standard_deviation, gnss.height_standard_deviation, gnss.position_type);
		//write_debug_log_file(3, log_str);
		sprintf(out_msg, "$GPGNSS,%s",log_str);
		debug_raw.pos_llh[0] = gnss.latitude;
		debug_raw.pos_llh[1] = gnss.longitude;
		debug_raw.pos_llh[2] = gnss.height;
	}
	break;
	case 99:
	{
		debugVel  vel = { 0 };
		size_t packet_size = sizeof(debugVel);
		memcpy(&vel, playload, debug_raw.payload_lenth);
		sprintf(log_str,"%d,%11.4f,%14.9f,%14.9f,%10.4f\n", time_tag.time1, (float)time_tag.time2 / 1000,
			vel.horizontal_speed, vel.track_over_ground,vel.vertical_speed);
		//write_debug_log_file(4, log_str);
		sprintf(out_msg, "$GPVEL,%s",log_str);
	}
	break;
	case 507://ins
	{
		debugIns ins = { 0 };
		size_t packet_size = sizeof(debugIns);
		memcpy(&ins, playload, debug_raw.payload_lenth);
		sprintf(log_str,"%d,%11.4f,%14.9f,%14.9f,%10.4f,%10.4f,%10.4f,%10.4f,%14.9f,%14.9f,%14.9f,%d \n",
			ins.GPS_Week,ins.GPS_TimeofWeek,ins.latitude,ins.longitude,ins.height,ins.north_velocity,
			ins.east_velocity,ins.up_velocity,ins.roll,ins.pitch,ins.azimuth,ins.status);
		//write_debug_log_file(5, log_str);
		sprintf(out_msg, "$GPINS,%s",log_str);
	}
	break;
	}
}

extern int input_debug_raw(uint8_t data, char* out_msg) {
    int ret = 0;
	debug_raw.buff[debug_raw.nbyte++] = data;
	if (debug_raw.state == 0) {
		if (debug_raw.nbyte == 1) {
			if (debug_raw.buff[0] != DEBUG_PACKET_HEAD[0]) {
				debug_raw.nbyte = 0;
				return 0;
			}
		}
		else if (debug_raw.nbyte == 2) {
			if (debug_raw.buff[1] != DEBUG_PACKET_HEAD[1]) {
				debug_raw.nbyte = 0;
				return 0;
			}
		}
		else if (debug_raw.nbyte == 3) {
			if (debug_raw.buff[2] != DEBUG_PACKET_HEAD[2]) {
				debug_raw.nbyte = 0;
				return 0;
			}
		}
		else if (debug_raw.nbyte == 4) {
			if (debug_raw.buff[3] != DEBUG_PACKET_HEAD[3]) {
				debug_raw.nbyte = 0;
				return 0;
			}
		}
		else if (debug_raw.nbyte == 6) {//type
			uint16_t messageid = 256 * debug_raw.buff[5] + debug_raw.buff[4];
			int i = 0, haveid = 0;
			for (i = 0; i < 5; i++) {
				if (debugPacketsTypeList[i] == messageid) {
					haveid = 1;
					debug_raw.messageid = messageid;
					break;
				}
			}
			if (haveid == 0) {
				debug_raw.nbyte = 0;
			}
			return 0;
		}
		else if (debug_raw.nbyte == DEBUG_PACKET_HEAD[3]) {
			debug_raw.head_length = DEBUG_PACKET_HEAD[3];
			debug_raw.payload_lenth = debug_raw.buff[8] + (debug_raw.buff[9] << 8);
			switch (debug_raw.messageid)
			{
			case 177:
			{
				size_t packet_size = sizeof(debugOdo);
				if (packet_size != debug_raw.payload_lenth) {
					debug_raw.nbyte = 0;
				}
			}
			break;
			case 268:
			{
				size_t packet_size = sizeof(debugImu);
				if (packet_size != debug_raw.payload_lenth) {
					debug_raw.nbyte = 0;
				}
			}
			break;
			case 42:
			{
				size_t packet_size = sizeof(debugGnss);
				if (packet_size != debug_raw.payload_lenth) {
					debug_raw.nbyte = 0;
				}
			}
			break;
			case 99:
			{
				size_t packet_size = sizeof(debugVel);
				if (packet_size != debug_raw.payload_lenth) {
					debug_raw.nbyte = 0;
				}
			}
			break;
			case 507://ins
			{
				size_t packet_size = sizeof(debugIns);
				if (packet_size != debug_raw.payload_lenth) {
					debug_raw.nbyte = 0;
				}
			}
			break;
			default:
				debug_raw.nbyte = 0;
				break;
			}

			if (debug_raw.nbyte != 0) {
				debug_raw.state = 1;
			}
		}
	}
	else {
		uint32_t len = debug_raw.head_length + debug_raw.payload_lenth + 4;
		if (debug_raw.nbyte == len) {
			uint32_t packet_crc = (debug_raw.buff[debug_raw.nbyte - 1] << 24) + (debug_raw.buff[debug_raw.nbyte - 2] << 16) + (debug_raw.buff[debug_raw.nbyte - 3] << 8) + debug_raw.buff[debug_raw.nbyte - 4];
			uint32_t calc_crc = calc_block_crc32(debug_raw.buff, debug_raw.nbyte - 4);
			if (packet_crc == calc_crc) {
				memcpy(&time_tag, debug_raw.buff + 14, 6);
				//printf("%d:%d", time_tag.time1, time_tag.time2);
				parse_debug_packet_payload(out_msg);
				packer_num++;
				ret = 1;
			}
			else {
				printf("%d:%d", packet_crc,calc_crc);
			}
			debug_raw.nbyte = 0;
			debug_raw.state = 0;
		}
	}
	return ret;
}

double* debug_raw_get_pos()
{
	return debug_raw.pos_llh;
}

