#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#include <string.h>
#include "rtklib.h"

#define USER_PREAMB 0x55
#define NEAM_HEAD 0x24

const char* userNMEAList[10] = { "$GPGGA", "$GPRMC", "$GPGSV", "$GLGSV", "$GAGSV", "$BDGSV", "$GPGSA", "$GLGSA", "$GAGSA", "$BDGSA" };
const char* userPacketsTypeList[7] = { "s1", "K1", "pS", "sK", "iN", "d1", "gN" };

#pragma pack(push, 1)

typedef struct {
	uint8_t nmea_flag;
	uint8_t flag;
	uint8_t header_len;
	uint8_t header[4];
	uint32_t nbyte;
	uint8_t buff[256];
	uint32_t nmeabyte;
	uint8_t nmea[128];
	uint32_t GPS_Week;
	double GPS_TimeofWeek;
    gtime_t time;
	double pos_llh[3];
} usrRaw;

typedef struct
{
	uint32_t GPS_Week;
	double GPS_TimeofWeek;
	float x_accel;
	float y_accel;
	float z_accel;
	float x_rate;
	float y_rate;
	float z_rate;
} user_packet_s1;

typedef struct
{
	uint32_t GPS_Week;
	double GPS_TimeofWeek;
	uint32_t positionMode;
	double latitude;
	double longitude;
	double height;
	uint32_t numberOfSVs;
	float hdop;
	float age;
	uint32_t velocityMode;
	uint32_t insStatus;
	uint32_t insPositionType;
	float velocityNorth;
	float velocityEast;
	float velocityUp;
	float roll;
	float pitch;
	float heading;
	float latitude_std;
	float longitude_std;
	float height_std;
	float north_vel_std;
	float east_vel_std;
	float up_vel_std;
	float roll_std;
	float pitch_std;
	float heading_std;
}user_packet_pS;

typedef struct
{
	double GPS_TimeofWeek;
	uint8_t satelliteId;
	uint8_t systemId;
	uint8_t antennaId;
	uint8_t l1cn0;
	uint8_t l2cn0;
	float azimuth;
	float elevation;
} user_packet_sK;

#pragma pack(pop)

usrRaw user_raw = { 0 };

FILE* fnmea = NULL;
FILE* fs1 = NULL;
FILE* fK1 = NULL;
FILE* fpS = NULL;
FILE* fsK = NULL;

char base_user_file_name[256] = { 0 };
void set_base_user_file_name(char* file_name)
{
	strcpy(base_user_file_name, file_name);
}
void close_user_log_file() {
	if (fnmea)fclose(fnmea);
	if (fs1)fclose(fs1);
	if (fpS)fclose(fpS);
	if (fsK)fclose(fsK);
}

void write_user_log_file(int index, char* log) {
	char file_name[256] = { 0 };
	switch (index)
	{
	case 0:
	{
		if (fnmea == NULL) {
			sprintf(file_name, "%s.nmea", base_user_file_name);
			fnmea = fopen(file_name, "w");
		}
		if (fnmea) fprintf(fnmea, log);
	}
	break;
	case 1:
	{
		if (fs1 == NULL) {
			sprintf(file_name, "%s_s1.csv", base_user_file_name);
			fs1 = fopen(file_name, "w");
			if (fs1) fprintf(fs1, "GPS_Week,GPS_TimeofWeek,x_accel,y_accel,z_accel,x_rate,y_rate,z_rate\n");
		}
		if (fs1) fprintf(fs1, log);
	}
	break;
	case 3:
	{
		if (fpS == NULL) {
			sprintf(file_name, "%s_pS.csv", base_user_file_name);
			fpS = fopen(file_name, "w");
			if (fpS) fprintf(fpS, "GPS_Week,GPS_TimeofWeek,positionMode,latitude,longitude,height,numberOfSVs,hdop,age,velocityMode,insStatus,insPositionType,velocityNorth,velocityEast,velocityUp,roll,pitch,heading,latitude_std,longitude_std,height_std,north_vel_std,east_vel_std,up_vel_std,roll_std,pitch_std,heading_std\n");
		}
		if (fpS) fprintf(fpS, log);
	}
	break;
	case 4:
	{
		if (fsK == NULL) {
			sprintf(file_name, "%s_sK.csv", base_user_file_name);
			fsK = fopen(file_name, "w");
			if (fsK) fprintf(fsK, "GPS_TimeofWeek,satelliteId,systemId,antennaId,l1cn0,l2cn0,azimuth,elevation\n");
		}
		if (fsK) fprintf(fsK, log);
	}
	break;
	}
}

uint16_t calc_crc(uint8_t* buff, uint32_t nbyte) {
	uint16_t crc = 0x1D0F;
	int i, j;
	for (i = 0; i < nbyte; i++) {
		crc = crc ^ (buff[i] << 8);
		for (j = 0; j < 8; j++) {
			if (crc & 0x8000) {
				crc = (crc << 1) ^ 0x1021;
			}
			else {
				crc = crc << 1;
			}
		}
	}
	crc = crc & 0xffff;
	return crc;
}

void parse_user_packet_payload(uint8_t* buff, uint32_t nbyte, obs_t *obs, rtk_t* rtk, char* out_msg) {
	uint8_t payload_lenth = buff[2];
	char packet_type[4] = { 0 };
	uint8_t* payload = buff + 3;
	char log_str[1024] = { 0 };
	memcpy(packet_type, buff, 2);
    if(out_msg == NULL) return;
	if (strcmp(packet_type, "s1") == 0) {
		size_t packet_size = sizeof(user_packet_s1);
		if (payload_lenth == packet_size) {
			user_packet_s1 packet = { 0 };
			memcpy(&packet, payload, packet_size);
			sprintf(out_msg,"%d,%11.4f,%14.10f,%14.10f,%14.10f,%14.10f,%14.10f,%14.10f\n", packet.GPS_Week, packet.GPS_TimeofWeek,
				packet.x_accel, packet.y_accel, packet.z_accel, packet.x_rate, packet.y_rate, packet.z_rate);
			//write_user_log_file(1, log_str);
			user_raw.GPS_Week = packet.GPS_Week;
			user_raw.GPS_TimeofWeek = packet.GPS_TimeofWeek;
			user_raw.time = gpst2time(user_raw.GPS_Week,user_raw.GPS_TimeofWeek);
		}
	}
	else if (strcmp(packet_type, "K1") == 0) {

	}
	else if (strcmp(packet_type, "pS") == 0) {
		size_t packet_size = sizeof(user_packet_pS);
		if (payload_lenth == packet_size) {
			user_packet_pS pak = { 0 };
			memcpy(&pak, payload, packet_size);
			sprintf(out_msg, "%d,%11.4f,%d,%16.12f,%16.12f,%16.12f,%2d,%8.4f,%8.4f,%d,%d,%d,%8.4f,%8.4f,%8.4f,%8.4f,%8.4f,%8.4f,%8.4f,%8.4f,%8.4f,%8.4f,%8.4f,%8.4f,%8.4f,%8.4f,%8.4f\n",
				pak.GPS_Week, pak.GPS_TimeofWeek, pak.positionMode, pak.latitude, pak.longitude, pak.height, pak.numberOfSVs,
				pak.hdop, pak.age, pak.velocityMode, pak.insStatus, pak.insPositionType, pak.velocityNorth, pak.velocityEast,
				pak.velocityUp, pak.roll, pak.pitch, pak.heading,pak.latitude_std, pak.longitude_std, pak.height_std, pak.north_vel_std, 
				pak.east_vel_std, pak.up_vel_std, pak.roll_std, pak.pitch_std, pak.heading_std);

			user_raw.pos_llh[0] = pak.latitude;
			user_raw.pos_llh[1] = pak.longitude;
			user_raw.pos_llh[2] = pak.height;

			user_raw.GPS_Week = pak.GPS_Week;
			user_raw.GPS_TimeofWeek = pak.GPS_TimeofWeek;
			user_raw.time = gpst2time(user_raw.GPS_Week,user_raw.GPS_TimeofWeek);
			//write_user_log_file(3, log_str);
		}
	}
	else if (strcmp(packet_type, "sK") == 0) {
		char* p = out_msg;
		size_t packet_size = sizeof(user_packet_sK);
		if (payload_lenth % packet_size == 0) {
			user_packet_sK pak = { 0 };
			int num = payload_lenth / packet_size;
			int i = 0;
			int sys = SYS_NONE;
			for (i = 0; i < num; i++) {
				memcpy(&pak, payload+i* packet_size, packet_size);
				sprintf(p, "%11.4f,%2d,%2d,%2d,%2d,%2d,%14.10f,%14.10f\n",
					pak.GPS_TimeofWeek, pak.satelliteId, pak.systemId, pak.antennaId, pak.l1cn0, pak.l2cn0, pak.azimuth, pak.elevation);
				pak.azimuth = pak.azimuth*D2R;
				pak.elevation = pak.elevation*D2R;
				if(obs && rtk){
					if(fabs(user_raw.GPS_TimeofWeek - pak.GPS_TimeofWeek) > 0.01f){
						obs->n = 0;
						user_raw.GPS_TimeofWeek = pak.GPS_TimeofWeek;
						if(user_raw.GPS_Week > 0) user_raw.time = gpst2time(user_raw.GPS_Week,user_raw.GPS_TimeofWeek);
					}
					switch(pak.systemId){
						case 0:sys = SYS_GPS;break;
						case 1:sys = SYS_GLO;break;
						case 2:sys = SYS_GAL;break;
						case 3:sys = SYS_QZS;break;
						case 4:sys = SYS_CMP;break;
						case 5:sys = SYS_SBS;break;
					}
					obs->data[obs->n].sat = satno(sys,pak.satelliteId);
					obs->data[obs->n].SNR[0] = pak.l1cn0*4;
					obs->data[obs->n].SNR[1] = pak.l2cn0*4;
					rtk->ssat[obs->data[obs->n].sat-1].azel[0] = pak.azimuth;
					rtk->ssat[obs->data[obs->n].sat-1].azel[1] = pak.elevation;
					rtk->ssat[obs->data[obs->n].sat-1].vs = 1;
					obs->n++;
                }
				if(strlen(out_msg) > 1000) break;
				p = out_msg + strlen(out_msg);
				//write_user_log_file(4, log_str);
			}
			printf("n:%d\n", obs->n);
		}
	}
}

int parse_nmea(uint8_t data, char* out_msg) {
	if (user_raw.nmea_flag == 0) {
		if (NEAM_HEAD == data) {
			user_raw.nmea_flag = 1;
			user_raw.nmeabyte = 0;
			user_raw.nmea[user_raw.nmeabyte++] = data;
		}
	}
	else if (user_raw.nmea_flag == 1) {
		user_raw.nmea[user_raw.nmeabyte++] = data;
		if (user_raw.nmeabyte == 6) {
			int i = 0;
			char NMEA[8] = { 0 };
			memcpy(NMEA, user_raw.nmea, 6);
			for (i = 0; i < 10; i++) {
				if (strcmp(NMEA, userNMEAList[i]) == 0) {
					user_raw.nmea_flag = 2;
					break;
				}
			}
			if (user_raw.nmea_flag != 2) {
				user_raw.nmea_flag = 0;
			}
		}
	}
	else if (user_raw.nmea_flag == 2) {
		user_raw.nmea[user_raw.nmeabyte++] = data;
		if (user_raw.nmea[user_raw.nmeabyte-1] == 0x0A && user_raw.nmea[user_raw.nmeabyte-2] == 0x0D){
			user_raw.nmea[user_raw.nmeabyte-1] = 0;
			user_raw.nmea_flag = 0;
            if(out_msg == NULL) return 0;
			//write_user_log_file(0, (char*)user_raw.nmea);
			strcpy(out_msg,user_raw.nmea);
			return 2;
		}
	}
    return 0;
}

extern int input_user_raw(uint8_t data,obs_t *obs,rtk_t* rtk,char* out_msg) {
	int ret = 0;
	if (user_raw.flag == 0) {
		user_raw.header[user_raw.header_len++] = data;
		if (user_raw.header_len == 1) {
			if (user_raw.header[0] != USER_PREAMB) {
				user_raw.header_len = 0;
				//return 0;
			}
		}
		if (user_raw.header_len == 2) {
			if (user_raw.header[1] != USER_PREAMB) {
				user_raw.header_len = 0;
				//return 0;
			}
		}
		if (user_raw.header_len == 4) {
			int i = 0;
			for (i = 0; i < 7; i++) {
				const char* packetType = userPacketsTypeList[i];
				if (packetType[0] == user_raw.header[2] && packetType[1] == user_raw.header[3]) {
					user_raw.flag = 1;
					user_raw.buff[user_raw.nbyte++] = packetType[0];
					user_raw.buff[user_raw.nbyte++] = packetType[1];
					break;
				}
			}
			user_raw.header_len = 0;
			//return 0;
		}
		return parse_nmea(data,out_msg);
	}
	else {
		user_raw.buff[user_raw.nbyte++] = data;
		if (user_raw.nbyte == user_raw.buff[2] + 5) { //5 = [type1,type2,len] + [crc1,crc2]
			uint16_t packet_crc = 256 * user_raw.buff[user_raw.nbyte - 2] + user_raw.buff[user_raw.nbyte - 1];
			if (packet_crc == calc_crc(user_raw.buff, user_raw.nbyte - 2)) {
				parse_user_packet_payload(user_raw.buff, user_raw.nbyte,obs,rtk, out_msg);
				ret = 1;
			}
			user_raw.flag = 0;
			user_raw.nbyte = 0;
		}
	}
	return ret;
}

double* user_raw_get_pos()
{
	return user_raw.pos_llh;
}
