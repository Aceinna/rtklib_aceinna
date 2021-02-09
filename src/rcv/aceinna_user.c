#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#include <string.h>
#include "rtklib.h"

#define USER_PREAMB 0x55
#ifndef NEAM_HEAD
#define NEAM_HEAD 0x24
#endif // !NEAM_HEAD

#define MAX_NMEA_TYPES 14
#define MAX_PACKET_TYPES 5

const char* userNMEAList[MAX_NMEA_TYPES] = { "$GPGGA", "$GPRMC", "$GPGSV", "$GLGSV", "$GAGSV", "$BDGSV", "$GPGSA", "$GLGSA", "$GAGSA", "$BDGSA", "$GPZDA", "$GPVTG", "$PASHR", "$GNINS" };
const char* userPacketsTypeList[MAX_PACKET_TYPES] = { "s1", "g1", "i1", "o1", "y1" };      

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
	double GPS_TimeofWeek_2;
    gtime_t time;
	double pos_llh[3];
	uint32_t sol_stat;
} usrRaw;

typedef struct
{
	uint16_t GPS_Week;
	uint32_t GPS_TimeOfWeek;
	float x_accel;
	float y_accel;
	float z_accel;
	float x_gyro;
	float y_gyro;
	float z_gyro;
} user_packet_s1;
/*
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
*/
typedef struct
{
	uint16_t GPS_Week;
	uint32_t GPS_TimeOfWeek;
	uint8_t  position_type;
	double   latitude;
	double   longitude;
	double   height;
	float    latitude_standard_deviation;
	float    longitude_standard_deviation;
	float    height_standard_deviation;
	uint8_t  number_of_satellites;
	uint8_t  number_of_satellites_in_solution;
	float    hdop;
	float    diffage;
	float    north_vel;
	float    east_vel;
	float    up_vel;
	float    north_vel_standard_deviation;
	float    east_vel_standard_deviation;
	float    up_vel_standard_deviation;
} user_packet_g1;

typedef struct
{
	uint16_t GPS_Week;
	uint32_t GPS_TimeOfWeek;
	uint8_t  ins_status;
	uint8_t  ins_position_type;
	double   latitude;
	double   longitude;
	double   height;
	double   north_velocity;
	double   east_velocity;
	double   up_velocity;
	double   roll;
	double   pitch;
	double   heading;
	float    latitude_std;
	float    longitude_std;
	float    height_std;
	float    north_velocity_std;
	float    east_velocity_std;
	float    up_velocity_std;
	float    roll_std;
	float    pitch_std;
	float    heading_std;
} user_packet_i1;

typedef struct
{
	uint16_t GPS_Week;
	uint32_t GPS_TimeOfWeek;
	uint8_t  mode;
	double   speed;
	uint8_t  fwd;
	uint64_t wheel_tick;
} user_packet_o1;

typedef struct
{
	uint16_t GPS_Week;
	uint32_t GPS_TimeOfWeek;
	uint8_t  satelliteId;
	uint8_t  systemId;
	uint8_t  antennaId;
	uint8_t  l1cn0;
	uint8_t  l2cn0;
	float    azimuth;
    float    elevation;
}user_packet_y1;

#pragma pack(pop)

usrRaw user_raw = { 0 };

FILE* fnmea = NULL;
FILE* fs1 = NULL;
FILE* fg1 = NULL;
FILE* fi1 = NULL;
FILE* fo1 = NULL;
FILE* fy1 = NULL;

char base_user_file_name[256] = { 0 };
void set_base_user_file_name(char* file_name)
{
	strcpy(base_user_file_name, file_name);
}
void close_user_log_file() {
	if (fnmea)fclose(fnmea);fnmea=NULL;
	if (fs1)fclose(fs1);fs1=NULL;
	if (fg1)fclose(fg1);fg1=NULL;
	if (fi1)fclose(fi1);fi1=NULL;
	if (fo1)fclose(fo1);fo1=NULL;
	if (fy1)fclose(fy1);fy1=NULL;
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
			if (fs1) fprintf(fs1, "GPS_Week,GPS_TimeOfWeek,x_accel,y_accel,z_accel,x_gyro,y_gyro,z_gyro\n");
		}
		if (fs1) fprintf(fs1, log);
	}
	break;
	case 2:
	{
		if (fg1 == NULL) {
			sprintf(file_name, "%s_g1.csv", base_user_file_name);
			fg1 = fopen(file_name, "w");
			if (fg1) fprintf(fg1, "GPS_Week,GPS_TimeOfWeek,position_type,latitude,longitude,height,\
latitude_standard_deviation,longitude_standard_deviation, height_standard_deviation,\
number_of_satellites, number_of_satellites_in_solution,hdop,diffage,north_vel,\
east_vel,up_vel, north_vel_standard_deviation,east_vel_standard_deviation,up_vel_standard_deviation\n");
		}
		if (fg1) fprintf(fg1, log);
	}
	break;
	case 3:
	{
		if (fi1 == NULL) {
			sprintf(file_name, "%s_i1.csv", base_user_file_name);
			fi1 = fopen(file_name, "w");
			if (fi1) fprintf(fi1, "GPS_Week,GPS_TimeOfWeek,ins_status,ins_position_type,latitude,longitude,height, \
			north_velocity,east_velocity,up_velocity,roll,pitch,heading,latitude_std,longitude_std,height_std, \
			north_velocity_std,east_velocity_std,up_velocity_std,roll_std,pitch_std,heading_std\n");
		}
		if (fi1) fprintf(fi1, log);
    }
	break;
	case 4:
	{
		if (fo1 == NULL) {
			sprintf(file_name, "%s_o1.csv", base_user_file_name);
			fo1 = fopen(file_name, "w");
			if (fo1) fprintf(fo1, "GPS_Week,GPS_TimeOfWeek,mode,speed,fwd,wheel_tick\n");
		}
		if (fo1) fprintf(fo1, log);
	}
    break;
	case 5:
	{
		if (fy1 == NULL) {
			sprintf(file_name, "%s_y1.csv", base_user_file_name);
			fy1 = fopen(file_name, "w");
			if (fy1) fprintf(fy1, "GPS_Week,GPS_TimeOfWeek,satelliteId,systemId,antennaId,l1cn0,l2cn0,azimuth,elevation\n");
		}
		if (fy1) fprintf(fy1, log);
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
			user_packet_s1 pak = { 0 };
			memcpy(&pak, payload, packet_size);
			sprintf(out_msg,"%d,%11.4f,%14.10f,%14.10f,%14.10f,%14.10f,%14.10f,%14.10f\n", pak.GPS_Week,(double)pak.GPS_TimeOfWeek/1000.0,
				pak.x_accel, pak.y_accel, pak.z_accel, pak.x_gyro, pak.y_gyro, pak.z_gyro);
			write_user_log_file(1, out_msg);
			user_raw.GPS_Week = pak.GPS_Week;
			user_raw.GPS_TimeofWeek = (double)pak.GPS_TimeOfWeek/1000.0;
			user_raw.time = gpst2time(user_raw.GPS_Week,user_raw.GPS_TimeofWeek);
		}
	}
	else if (strcmp(packet_type, "g1") == 0) {
		size_t packet_size = sizeof(user_packet_g1); 
		if (payload_lenth == packet_size) {
			user_packet_g1 pak = { 0 };
			memcpy(&pak, payload, packet_size);
			sprintf(out_msg,"%d,%11.4f,%d,%14.9f,%14.9f,%10.4f,%10.4f,%10.4f,%10.4f,%d,%d,%5.1f,%5.1f,%10.4f,%10.4f,%10.4f,%10.4f,%10.4f,%10.4f\n",  
				pak.GPS_Week,(double)pak.GPS_TimeOfWeek/1000.0,pak.position_type,pak.latitude,pak.longitude,pak.height,
				pak.latitude_standard_deviation,pak.longitude_standard_deviation,pak.height_standard_deviation,
				pak.number_of_satellites,pak.number_of_satellites_in_solution,pak.hdop,pak.diffage,pak.north_vel,pak.east_vel,pak.up_vel,
				pak.north_vel_standard_deviation,pak.east_vel_standard_deviation,pak.up_vel_standard_deviation);
			write_user_log_file(2, out_msg);

			user_raw.pos_llh[0] = pak.latitude;
			user_raw.pos_llh[1] = pak.longitude;
			user_raw.pos_llh[2] = pak.height;

			user_raw.GPS_Week = pak.GPS_Week;
			user_raw.GPS_TimeofWeek = (double)pak.GPS_TimeOfWeek/1000.0;
			user_raw.time = gpst2time(user_raw.GPS_Week,user_raw.GPS_TimeofWeek);
			user_raw.sol_stat = pak.position_type;
        }
	}
	else if (strcmp(packet_type, "i1") == 0) {
		size_t packet_size = sizeof(user_packet_i1); 
		if (payload_lenth == packet_size) {   
			user_packet_i1 pak = { 0 };
			memcpy(&pak, payload, packet_size);
			sprintf(out_msg,"%d,%11.4f,%d,%d,%14.9f,%14.9f,%10.4f,%10.4f,%10.4f,%10.4f,%14.9f,%14.9f,%14.9f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f\n",  
				pak.GPS_Week,(double)pak.GPS_TimeOfWeek/1000.0,pak.ins_status,pak.ins_position_type,pak.latitude,pak.longitude,pak.height,
				pak.north_velocity,pak.east_velocity,pak.up_velocity,pak.roll,pak.pitch,pak.heading,pak.latitude_std,pak.longitude_std,pak.height_std,
				pak.north_velocity_std,pak.east_velocity_std,pak.up_velocity_std,pak.roll_std,pak.pitch_std,pak.heading_std);
			write_user_log_file(3, out_msg);
		}
	}
	else if (strcmp(packet_type, "o1") == 0) {
		size_t packet_size = sizeof(user_packet_o1); 
		if (payload_lenth == packet_size) {   
			user_packet_o1 pak = { 0 };
			memcpy(&pak, payload, packet_size);
			sprintf(out_msg,"%d,%11.4f,%d,%10.4f,%d,%I64d\n",  
				pak.GPS_Week,(double)pak.GPS_TimeOfWeek/1000.0,pak.mode,pak.speed,pak.fwd,pak.wheel_tick);
			write_user_log_file(4, out_msg);
		}	 
	}
	else if (strcmp(packet_type, "y1") == 0) {
		char* p = out_msg;
		size_t packet_size = sizeof(user_packet_y1);	
		if (payload_lenth % packet_size == 0) { 
			user_packet_y1 pak = { 0 };
			int num = payload_lenth / packet_size;
			int i = 0;
			int sys = SYS_NONE;
			double GPS_TimeofWeek;
			for (i = 0; i < num; i++) {
				memcpy(&pak, payload+i* packet_size, packet_size);
				sprintf(p, "%d,%11.4f,%2d,%2d,%2d,%2d,%2d,%10.3f,%10.3f\n",pak.GPS_Week,(double)pak.GPS_TimeOfWeek/1000.0,
					pak.satelliteId, pak.systemId, pak.antennaId, pak.l1cn0, pak.l2cn0, pak.azimuth, pak.elevation);
				p = out_msg + strlen(out_msg);
				pak.azimuth = pak.azimuth*D2R;
				pak.elevation = pak.elevation*D2R;
				if(obs && rtk){
					GPS_TimeofWeek = (double)pak.GPS_TimeOfWeek/1000.0;
					if(fabs(user_raw.GPS_TimeofWeek_2 - GPS_TimeofWeek) > 0.01f){
						obs->n = 0;
						user_raw.GPS_TimeofWeek_2 = GPS_TimeofWeek;
						if(user_raw.GPS_Week > 0) user_raw.time = gpst2time(user_raw.GPS_Week,user_raw.GPS_TimeofWeek_2);
						memset(rtk->ssat,0,sizeof(ssat_t)*MAXSAT);
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
					rtk->ssat[obs->data[obs->n].sat-1].vsat[0] = 1;
					obs->n++;
                }
			}
			write_user_log_file(5, out_msg);
        }
	}
    /*
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
    */
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
			for (i = 0; i < MAX_NMEA_TYPES; i++) {
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
			write_user_log_file(0, (char*)user_raw.nmea);
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
			for (i = 0; i < MAX_PACKET_TYPES; i++) {
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

uint32_t user_raw_get_sol_stat()
{
	return user_raw.sol_stat;
}
