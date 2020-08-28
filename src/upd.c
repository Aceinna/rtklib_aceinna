/*------------------------------------------------------------------------------
* upd.c : UPD functions
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static int addupd(upd_t *upd, updb_t *updb)
{
	updb_t *upd_updb;

	if (upd->nu>=upd->numax) {
		upd->numax+=256;
		if (!(upd_updb=(updb_t *)realloc(upd->updb,sizeof(updb_t)*upd->numax))) {
			printf("readupdb malloc error n=%d\n",upd->numax);
			free(upd->updb); upd->updb=NULL; upd->nu=upd->numax=0;
			return 0;
		}
		upd->updb=upd_updb;
	}
	upd->updb[upd->nu++]=*updb;
	return 1;
}
static void readupdh(FILE *fp)
{
	char buff[1024],*label;
	double bias;
	int sat;

	while (fgets(buff,sizeof(buff),fp)) {
		if (strlen(buff)<60) continue;
		label=buff+60;
		if (strstr(label,"COMMENT")) {
			if (!strncmp(buff,"WL",2)&&(sat=satid2no(buff+4))&&
				sscanf(buff+14,"%lf",&bias)==1) {
			 //		cPPPAr.m_upd.wlbias[sat-1]=bias;
			}
		}
		else if (strstr(label,"END OF HEADER"))
			break;
	}
}
static void readupdb(FILE *fp)
{
	gtime_t time;
	updb_t updb;
	char buff[1024];
	int i,sat,sys,prn;
	int bvalid=1;
	double val,std;

	while (!feof(fp))
	{
		if (bvalid)
		{
			fgets(buff,sizeof(buff),fp);
		}
		bvalid=1;

		if (buff[0]!='*'||str2time(buff,2,26,&time))
		{
			continue;
		}
		updb.time=time;

		for (i=0;i<MAXSAT;i++) {
			updb.l1bias[i]=0.0;
			updb.std[i]=0.0;
		}

		for (i=0;i<MAXSAT&&fgets(buff,sizeof(buff),fp);i++) {
			if (buff[0]=='*') {
				bvalid=0;
				break;
			}

			sys=buff[1]==' '?SYS_GPS:code2sys(buff[1]);
			prn=(int)str2num(buff,2,2);
			if (!(sat=satno(sys,prn))) continue;

			val=str2num(buff,24,6);
			std=str2num(buff,54,6);

			//if (std<...)
			updb.l1bias[sat-1]=val;
			updb.std[sat-1]=std;
		}
	  //	if (!addupd(&cPPPAr.m_upd,&updb)) return;
	}
}
extern void readupd(const char *file)
{
	FILE *fp;
	gtime_t time={0};
	const char *ext;

	if (!(ext=strrchr(file,'.'))) return;

	if (!strstr(ext+1,"fcb")&&!strstr(ext+1,".FCB")) return;

	if (!(fp=fopen(file,"r"))) {
		return;
	}

   //	cPPPAr.m_upd.nu=cPPPAr.m_upd.numax=0;

	/* read upd header */
	readupdh(fp);

	/* read sp3 body */
	readupdb(fp);
}
extern double upd_NL(gtime_t time, const int sat)
{
	int i;
	double upd_nl;

  /*	for (i=0;i<cPPPAr.m_upd.nu;i++) {
		if (timediff(cPPPAr.m_upd.updb[i].time,time)>=0.0) break;
	}
	if (i==0) i=1;
	if (i>=cPPPAr.m_upd.nu) i=cPPPAr.m_upd.nu-1;
	
	upd_nl=cPPPAr.m_upd.updb[i-1].l1bias[sat-1];    */

	return upd_nl;
}