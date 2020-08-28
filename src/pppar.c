/*------------------------------------------------------------------------------
* pppar.c : precise point positioning ambiguity resolution
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

#define SQR(x)      ((x)*(x))
#define SQRT(x)     ((x)<=0.0?0.0:sqrt(x))

static void init(rtk_t *rtk)
{
	int i;
	for (i=0;i<MAXSAT;i++)
	{
		rtk->pppar.m_satAmb1[i].bfxied[0]= rtk->pppar.m_satAmb1[i].bfxied[1]=0;
		rtk->pppar.m_satAmb1[i].ifamb[0] = rtk->pppar.m_satAmb1[i].ifamb[1]=0.0;
		rtk->pppar.m_satAmb1[i].l1amb[0] = rtk->pppar.m_satAmb1[i].l1amb[1]=0.0;
		rtk->pppar.m_satAmb1[i].mwamb[0] = rtk->pppar.m_satAmb1[i].mwamb[1]=0.0;
		rtk->pppar.m_satAmb1[i].ifsig    = 0.0;
		rtk->pppar.m_satAmb1[i].l1sig    = 0.0;
		rtk->pppar.m_satAmb1[i].mwsig    = 0.0;
		rtk->pppar.m_satAmb1[i].azel[0]  = 0.0;
		rtk->pppar.m_satAmb1[i].azel[1]  = 0.0;
		rtk->pppar.m_satAmb1[i].vsat     = 0;
		rtk->pppar.m_satAmb1[i].nlock=rtk->pppar.m_satAmb1[i].nfix=0;

		rtk->pppar.m_satAmb2[i].bfxied[0] = rtk->pppar.m_satAmb2[i].bfxied[1] = 0;
		rtk->pppar.m_satAmb2[i].ifamb [0] = rtk->pppar.m_satAmb2[i].ifamb [1] = 0.0;
		rtk->pppar.m_satAmb2[i].l1amb [0] = rtk->pppar.m_satAmb2[i].l1amb [1] = 0.0;
		rtk->pppar.m_satAmb2[i].mwamb [0] = rtk->pppar.m_satAmb2[i].mwamb [1] = 0.0;
		rtk->pppar.m_satAmb2[i].ifsig     = 0.0;
		rtk->pppar.m_satAmb2[i].l1sig     = 0.0;
		rtk->pppar.m_satAmb2[i].mwsig     = 0.0;
		rtk->pppar.m_satAmb2[i].azel[0]   = 0.0;
		rtk->pppar.m_satAmb2[i].azel[1]   = 0.0;
		rtk->pppar.m_satAmb2[i].vsat      = 0;
		rtk->pppar.m_satAmb2[i].nlock = rtk->pppar.m_satAmb2[i].nfix = 0;
	}
	rtk->pppar.m_solStat.fRRatio=0.0;
	rtk->pppar.m_solStat.fSRate_boot =0.0;
}

static int getAmbProperty(rtk_t *rtk)
{
	int i,g1,n=0,nx=rtk->nx;
	prcopt_t *opt=&rtk->opt;

	rtk->xa[0]=0.0;
	rtk->xa[1]=0.0;
	rtk->xa[2]=0.0;

	init(rtk);

	for (i=0;i<MAXSAT;i++)
	{
		if (rtk->ssat[i].outc[0]>=10)
			rtk->pppar.m_satAmb1[i].nfix = rtk->pppar.m_satAmb2[i].nfix = 0;

		if (rtk->ssat[i].vsat[0]==0)
			continue;

		if (opt->sFlag[i].sys==SYS_CMP)
		{
			if (opt->sFlag[i].prn<=5)
				continue;
		}

		rtk->pppar.m_satAmb1[i].ifamb[0] = rtk->x[IB(i + 1, 0, opt)] * 1.0 / opt->lam[i][0];
		rtk->pppar.m_satAmb1[i].mwamb[0] = opt->ssat_Ex[i].mw[1];
		rtk->pppar.m_satAmb1[i].azel [0] = rtk->ssat[i].azel[0];
		rtk->pppar.m_satAmb1[i].azel [1] = rtk->ssat[i].azel[1];
		rtk->pppar.m_satAmb1[i].mwsig    = SQRT(opt->ssat_Ex[i].mwVar_c);
		rtk->pppar.m_satAmb1[i].nlock    = rtk->ssat[i].lock[0];
		rtk->pppar.m_satAmb1[i].vsat     = 1;
		rtk->pppar.m_satAmb2[i].ifamb[0] = rtk->x[IB(i + 1, 0, opt)] * 1.0 / opt->lam[i][0];
		rtk->pppar.m_satAmb2[i].mwamb[0] = opt->ssat_Ex[i].mw[1];
		rtk->pppar.m_satAmb2[i].azel [0] = rtk->ssat[i].azel[0];
		rtk->pppar.m_satAmb2[i].azel [1] = rtk->ssat[i].azel[1];
		rtk->pppar.m_satAmb2[i].mwsig    = SQRT(opt->ssat_Ex[i].mwVar_c);
		rtk->pppar.m_satAmb2[i].nlock    = rtk->ssat[i].lock[0];
		rtk->pppar.m_satAmb2[i].vsat     = 1;

		n++;
	}


	for (i=0;i<MAXSAT;i++)
	{
		if (rtk->ssat[i].vsat[0]==0)
			continue;

		g1=IB(i+1,0,opt);

		rtk->pppar.m_satAmb1[i].ifsig=SQRT(rtk->P[g1*nx+g1])*1.0/opt->lam[i][0];
		rtk->pppar.m_satAmb1[i].l1sig=rtk->pppar.m_satAmb1[i].ifsig*(1.0 + opt->lam[i][0]/opt->lam[i][1]);

		rtk->pppar.m_satAmb2[i].ifsig = SQRT(rtk->P[g1*nx + g1])*1.0 / opt->lam[i][0];
		rtk->pppar.m_satAmb2[i].l1sig = rtk->pppar.m_satAmb2[i].ifsig*(1.0 + opt->lam[i][0]/opt->lam[i][1]);

		trace(3," sat %2d vsat %2d nlock %2d wlamb %8.3f sig %6.4f ifamb %8.3f sig %6.4f\n",
			  i+1, rtk->pppar.m_satAmb1[i].vsat, rtk->pppar.m_satAmb1[i].nlock,
			  rtk->pppar.m_satAmb1[i].mwamb[0], rtk->pppar.m_satAmb1[i].mwsig,
			  rtk->pppar.m_satAmb1[i].ifamb[0],rtk->pppar.m_satAmb2[i].l1sig);
	}

	return n;
}

 static int satsysId(const int sat)
 {
	   int sysId;
	   int sys=satsys(sat,NULL);

		if (sys==SYS_GPS)
		   sysId=0;
		else if (sys==SYS_GAL)
		   sysId=1;
		else if (sys==SYS_CMP)
		   sysId=2;
		else
		   sysId=-1;

	   return sysId;
 }


static int WLfix_round(const int *rsat, rtk_t *rtk)
{
	double dt,wlSr;
	int i,sysId,num,ret=1;

	num=0;
	rtk->pppar.m_solStat.nFixedAmb_WL=0;

	for (i = 0; i < 3; i++)
	{
		if (rsat[i]!=-1)
		{
		   if ( fabs( rtk->pppar.m_upd.wlbias[rsat[i]-1] )>=10.0 )  return;
		   wlSr=rtk->pppar.m_satAmb1[rsat[i]-1].mwamb[0]-rtk->pppar.m_upd.wlbias[rsat[i]-1];

		   rtk->pppar.m_satAmb1[rsat[i] - 1].mwamb[1]  = myRound(wlSr);
		   rtk->pppar.m_satAmb1[rsat[i] - 1].mwsig     = 1.0e-10;
		   rtk->pppar.m_satAmb1[rsat[i] - 1].bfxied[0] = 1;
		}
	}

	for (i=0;i<MAXSAT;i++)
	{
		sysId=satsysId(i+1);

		if (sysId      ==-1)   continue;
		if (rsat[sysId]==-1)   continue;

		if (i+1                             == rsat[sysId])                         continue;
		if (rtk->pppar.m_satAmb1[i].vsat    == 0   )                                continue;
		if (fabs(rtk->pppar.m_upd.wlbias[i])>= 10.0)                                continue;
		if (rtk->pppar.m_satAmb1[i].mwsig   <= 0.0 )                                continue;
		if (rtk->pppar.m_satAmb1[i].mwsig   >  rtk->pppar.m_prcOptAr.fMaxSig   [0]) continue;
		if (rtk->pppar.m_satAmb1[i].nlock   <  rtk->pppar.m_prcOptAr.nLockEpMin[0]) continue;

		dt=rtk->pppar.m_satAmb1[i].mwamb[0]=rtk->pppar.m_satAmb1[i].mwamb[0]-rtk->pppar.m_upd.wlbias[i]-wlSr;

		trace(3,"WLfix_round sat=%d vsat=%d mwsig=%8.4f frac=%8.4f\n",i+1,rtk->pppar.m_satAmb1[i].vsat,
		rtk->pppar.m_satAmb1[i].mwsig,dt);

		dt=frac(dt);
		if (fabs(dt)>rtk->pppar.m_prcOptAr.fMaxFrac[0]) continue;

		rtk->pppar.m_satAmb1[i].bfxied[0] = 1;
		rtk->pppar.m_satAmb2[i].bfxied[0] = 1;
		rtk->pppar.m_satAmb1[i].mwamb [1] = myRound(rtk->pppar.m_satAmb1[i].mwamb[0]);
		rtk->pppar.m_satAmb2[i].mwamb [1] = myRound(rtk->pppar.m_satAmb1[i].mwamb[0]);

		num++;
	}

	rtk->pppar.m_solStat.nFixedAmb_WL=num;

	if (num<4) ret=0;

	return ret;
}

static void WLAmbsUpdateDatabase(const int *rsat, rtk_t *rtk)
{
	int i,j,hasCycleslip,betterAmb,num = 0;

	for (i = 0; i < 3; i++)
	{
		if (rsat[i]==-1) continue;
		rtk->pppar.m_ambDatabase.m_ambWL   [rsat[i] - 1] = rtk->pppar.m_satAmb1[rsat[i] - 1].mwamb[1];
		rtk->pppar.m_ambDatabase.m_varambWL[rsat[i] - 1] = 1.0e-8;
		rtk->pppar.m_ambDatabase.m_ixWL    [rsat[i] - 1] = 1;

		if (rtk->pppar.m_ambDatabase.refsatWL[i]==0)
	   {
			for (j = 0; j<MAXSAT; j++)
			{
			   if (j + 1 == rsat[i]                  ) continue;
			   if (!rtk->pppar.m_satAmb1[j].bfxied[0]) continue;

			   rtk->pppar.m_ambDatabase.m_ambWL    [j] = rtk->pppar.m_satAmb1[j].mwamb[1] + rtk->pppar.m_ambDatabase.m_ambWL[rsat[i]-1];
			   if (rtk->pppar.m_ambDatabase.m_ambWL[j] == 0.0) continue;
			   rtk->pppar.m_ambDatabase.m_varambWL [j] = 1.0e-4;
			   rtk->pppar.m_ambDatabase.m_ixWL     [j] = 1;

			   if (rtk->pppar.m_ambDatabase.m_ixWL[j]) num++;
			}

		   if (num) rtk->pppar.m_ambDatabase.refsatWL[i] = rsat[i];
	   }
	   else if (rtk->pppar.m_ambDatabase.refsatWL[i]==rsat[i])
	   {
			for (j = 0; j < MAXSAT; i++)
			{
				if (j + 1 == rsat[i]) continue;
				if (!rtk->pppar.m_satAmb1[j].bfxied[0]) continue;

				hasCycleslip = rtk->ssat[j].slip[0] || rtk->ssat[j].slip[1];

				betterAmb = (hasCycleslip || !rtk->pppar.m_ambDatabase.m_ixWL[j]
						   || SQRT(rtk->pppar.m_satAmb1[j].mwsig) < rtk->pppar.m_ambDatabase.m_varambWL[j] );

				if (betterAmb)
				{
					rtk->pppar.m_ambDatabase.m_ambWL   [j] = rtk->pppar.m_satAmb1[j].mwamb[1]  + rtk->pppar.m_ambDatabase.m_ambWL[rsat[i] - 1];
					rtk->pppar.m_ambDatabase.m_varambWL[j] =  1.0e-4 ;
					rtk->pppar.m_ambDatabase.m_ixWL    [j] =  1 ;
				}
			}
	   }
	  else if (rtk->pppar.m_ambDatabase.refsatWL[i] != rsat[i])
	  {

		double refambshift = rtk->pppar.m_satAmb1[rsat[i] - 1].mwamb[1] - rtk->pppar.m_ambDatabase.m_ambWL[rtk->pppar.m_ambDatabase.refsatWL[i] - 1];

		for (j = 0; j < MAXSAT; j++)
		{
			if (rtk->pppar.m_ambDatabase.m_ixWL[j] != 0)
				rtk->pppar.m_ambDatabase.m_ambWL[j] -= refambshift;
		}
		rtk->pppar.m_ambDatabase.refsatWL[i] = rsat[i];

		for (j = 0; j < MAXSAT; j++)
		{
			if (!rtk->pppar.m_satAmb1[j].bfxied[0]) continue;
			hasCycleslip = rtk->ssat[j].slip[0] || rtk->ssat[j].slip[1];
			betterAmb = (hasCycleslip || !rtk->pppar.m_ambDatabase.m_ixWL[j] || SQRT(rtk->pppar.m_satAmb1[j].mwsig) < rtk->pppar.m_ambDatabase.m_varambWL[j]);

			if (betterAmb)
			{
				rtk->pppar.m_ambDatabase.m_ambWL[j]    = (rtk->pppar.m_satAmb1[j].bfxied[0]) ? rtk->pppar.m_satAmb1[j].mwamb[1] : rtk->pppar.m_satAmb1[j].mwamb[0];
				rtk->pppar.m_ambDatabase.m_varambWL[j] = (rtk->pppar.m_satAmb1[j].bfxied[0]) ? 1.0e-4 : SQRT(rtk->pppar.m_satAmb1[j].mwsig);
				rtk->pppar.m_ambDatabase.m_ixWL[j]     = (rtk->pppar.m_satAmb1[j].bfxied[0]) ? 1 : 0;
			}
		}
	  }

	}
}

void IntegerWeightedAmbsUpdateDatabase(const int *rsat, rtk_t *rtk, double aveambdiff)
{
	int i,j,sysId;
	double refambshift;
	if (rtk->pppar.m_ratio_rate > 2.5 && rtk->pppar.m_ratio > 2.5 )
	{
		for (i = 0; i < rtk->pppar.m_nwa; i++)
		{
			rtk->pppar.m_ambDatabase.m_ambL1   [rtk->pppar.m_ix1[i]] = rtk->pppar.m_ifip[i];
			rtk->pppar.m_ambDatabase.m_varambL1[rtk->pppar.m_ix1[i]] = 1.0e-4;
			rtk->pppar.m_ambDatabase.m_ixL1    [rtk->pppar.m_ix1[i]] = 1;
			rtk->pppar.m_ambDatabase.m_ratio = rtk->pppar.m_ratio;
			for (j = 0; j < 3; j++)
			{
			   rtk->pppar.m_ambDatabase.refsatL1[j] = rsat[j];
			}
		}
	}
	else
	{
	   for (i = 0; i < MAXSAT; i++)
	   {
		 sysId=satsysId(i+1);
		 if (sysId      ==-1) continue;
		 if (rsat[sysId]==-1) continue;

		  refambshift = rtk->pppar.m_ambDatabase.m_ambL1[rsat[sysId] - 1];
		  rtk->pppar.m_ambDatabase.m_ixL1[rsat[sysId] - 1] = 0;

		  if (rtk->pppar.m_ambDatabase.m_ixL1[i] != 0)
			  rtk->pppar.m_ambDatabase.m_ambL1[i] -= refambshift;
		}

		rtk->pppar.m_ambDatabase.refsatL1[sysId] = rsat[sysId];
	}
}

static void IntegerAmbsUpdateDatabase(const int *rsat, rtk_t *rtk)
{
	int i,j;

	for (j = 0; j < 3; j++)
	{
	   if (rsat[j]==-1) continue;

	   if (rtk->pppar.m_ambDatabase.refsatL1[j] == rsat[j])
	   {
		   for (i = 0; i < rtk->pppar.m_nfi; i++)
		   {
			rtk->pppar.m_ambDatabase.m_ambL1   [rtk->pppar.m_ix2[i]] = rtk->pppar.m_ifip2[i];
			rtk->pppar.m_ambDatabase.m_varambL1[rtk->pppar.m_ix2[i]] = 1.0e-4;
			rtk->pppar.m_ambDatabase.m_ixL1    [rtk->pppar.m_ix2[i]] = 1;
		   }
	   }
	   else if (rtk->pppar.m_ambDatabase.refsatL1[j] != rsat[j])
	   {
		   double refambshift = rtk->pppar.m_ambDatabase.m_ambL1[rsat[j] - 1];

		   for (i = 0; i < MAXSAT; i++)
		   {
			if (rtk->pppar.m_ambDatabase.m_ixL1[i] != 0)
				rtk->pppar.m_ambDatabase.m_ambL1[i] -= refambshift;
		   }
		   rtk->pppar.m_ambDatabase.refsatL1[j] = rsat[j];
	   }
	}

}

static int check_L1amb_PartialFixing(const int *rsat, double *pdop, double *X0,rtk_t *rtk)
{
	int i,sysId,num=0;
	double dt,azel[MAXSAT*2],dop[4];

	for (i=0;i<MAXSAT;i++)
	{
		sysId=satsysId(i+1);

		if (sysId==-1)        continue;
		if (i+1==rsat[sysId]) continue;
		if (rtk->pppar.m_satAmb2[i].bfxied[0]     == 0                                  ) continue;
		if (rtk->pppar.m_satAmb2[i].vsat          == 0                                  ) continue;
		if (rtk->pppar.m_satAmb2[i].azel[1] * R2D  < 10.0                               ) continue;
		if (rtk->pppar.m_satAmb2[i].l1sig          > rtk->pppar.m_prcOptAr.fMaxSig[1]   ) continue;
		if (rtk->pppar.m_satAmb2[i].nlock          < rtk->pppar.m_prcOptAr.nLockEpMin[1]) continue;
		if (rtk->pppar.m_satAmb2[i].azel[1] * R2D  < rtk->pppar.m_prcOptAr.fMinElAr_deg&&rtk->pppar.m_satAmb2[i].nlock < 50) continue;

		   dt=frac(rtk->pppar.m_satAmb2[i].l1amb[0]);
		if (fabs(dt)>rtk->pppar.m_prcOptAr.fMaxFrac[1]) continue;

		azel[2 * num + 0] = rtk->pppar.m_satAmb2[i].azel[0];
		azel[2 * num + 1] = rtk->pppar.m_satAmb2[i].azel[1];

		rtk->pppar.m_satAmb2[i].bfxied[1]=1;
		X0[num]=rtk->pppar.m_satAmb2[i].l1amb[0];
		rtk->pppar.m_ix2[num]=i;
		rtk->pppar.iRsGNSS[num]=rsat[sysId];

		num++;
	}

	dops(num,azel,0.0,dop);
	*pdop=dop[1];

	return num;
}

static int TransformAmbsToZSpace(const int nn, double* matZ, double *amb, double* m_ifip)
{
	double m_matZp1[MAXFAMB*MAXFAMB];
	int info;
	matcpy(m_matZp1, matZ, nn, nn);

	matmul("TN", nn, 1, nn, 1.0, m_matZp1, m_ifip, 0.0, amb);

	return 1;
}

static int check_L1amb_IntegerWeightedAve(const int *rsat, int excsat, double *pdop, double *X1,rtk_t *rtk)
{
	int i,sysId,num = 0;
	double dt, azel[MAXSAT * 2], dop[4];

	for (i = 0; i < MAXSAT; i++)
	{
		sysId=satsysId(i+1);
		if (sysId==-1)       continue;

		if (i + 1 == rsat[sysId]  ) continue;
		if (i + 1 == excsat       ) continue;

		if (rtk->pppar.m_satAmb1[i].bfxied[0]      == 0                                  ) continue;
		if (rtk->pppar.m_satAmb1[i].vsat           == 0                                  ) continue;
		if (rtk->pppar.m_satAmb1[i].azel[1] * R2D < 10.0                                 ) continue;
		if (rtk->pppar.m_satAmb1[i].l1sig         > rtk->pppar.m_prcOptAr.fMaxSig[2]     ) continue;
		if (rtk->pppar.m_satAmb1[i].nlock         < rtk->pppar.m_prcOptAr.nLockEpMin[1]  ) continue;

		azel[2 * num + 0] = rtk->pppar.m_satAmb1[i].azel[0];
		azel[2 * num + 1] = rtk->pppar.m_satAmb1[i].azel[1];

		rtk->pppar.m_satAmb1[i].bfxied[1] = 1;
		X1[num]                           = rtk->pppar.m_satAmb1[i].l1amb[0];
		rtk->pppar.m_ix1[num]             = i;
		rtk->pppar.iRsGNSS[num]           = rsat[sysId];
		num++;
	}

	dops(num, azel, 0.0, dop);
	*pdop = dop[1];

	return num;
}

static void ComputeIntegerWeightedAmbsRatio(const int nn, double *scand, double *sw,rtk_t *rtk)
{
	rtk->pppar.m_ratio = scand[1] / sw[0];

	if (rtk->pppar.m_prev_s[0] == 0.0 && rtk->pppar.m_prev_s[1] == 0.0)
	{
		rtk->pppar.m_prev_s[0] = scand[0] / nn;
		rtk->pppar.m_prev_s[1] = sw[0] / nn;
	}
	else if (rtk->pppar.m_curr_s[0] == 0.0 && rtk->pppar.m_curr_s[1] == 0.0)
	{
		rtk->pppar.m_curr_s[0] = scand[0] / nn;
		rtk->pppar.m_curr_s[1] = sw[0] / nn;
		rtk->pppar.m_ratio_rate = (rtk->pppar.m_prev_s[1] - rtk->pppar.m_curr_s[1]) / (fabs(rtk->pppar.m_curr_s[0] - rtk->pppar.m_prev_s[0]) + 1.0e-6);
		if (rtk->pppar.m_curr_s[1] / rtk->pppar.m_curr_s[0] > 1.2 || rtk->pppar.m_prev_s[1] / rtk->pppar.m_prev_s[0] > 1.2)
			rtk->pppar.m_ratio_rate = 1.0;
	}
	else
	{
		rtk->pppar.m_prev_s[0] = rtk->pppar.m_curr_s[0];
		rtk->pppar.m_prev_s[1] = rtk->pppar.m_curr_s[1];
		rtk->pppar.m_curr_s[0] = scand[0] / nn;
		rtk->pppar.m_curr_s[1] = sw[0] / nn;
		rtk->pppar.m_ratio_rate = (rtk->pppar.m_prev_s[1] - rtk->pppar.m_curr_s[1]) / (fabs(rtk->pppar.m_curr_s[0] - rtk->pppar.m_prev_s[0]) + 1.0e-6);
		if (rtk->pppar.m_curr_s[1] / rtk->pppar.m_curr_s[0] > 1.2 || rtk->pppar.m_prev_s[1] / rtk->pppar.m_prev_s[0] > 1.2)
			rtk->pppar.m_ratio_rate = 1.0;
	}
}

static void UpdateAmbs(const int nn, double *X1, const int *rsat, rtk_t *rtk)
{
	int i;
	double aveambdiff1 = 0.0;
	double aveambdiff2 = 0.0;
	for (i = 0; i < rtk->pppar.m_nwa; i++)
	{
		aveambdiff1 = aveambdiff1 + fabs(rtk->pppar.m_ifip[i] - X1[i]);
		if (rtk->pppar.m_ambDatabase.m_ambL1[rtk->pppar.m_ix1[i]] != 0.0)
			aveambdiff2 = aveambdiff2 + fabs(rtk->pppar.m_ambDatabase.m_ambL1[rtk->pppar.m_ix1[i]] - X1[i]);

	}
	aveambdiff1 = aveambdiff1 / rtk->pppar.m_nwa;
	aveambdiff2 = aveambdiff2 / rtk->pppar.m_nwa;

	IntegerWeightedAmbsUpdateDatabase(rsat, rtk, aveambdiff1);

	rtk->pppar.m_database = 0;

	if (rtk->pppar.m_ratio < 2.5  && aveambdiff2 > 1.0e-3)
	{
		for (i = 0; i < nn; i++)
		{
			if (rtk->pppar.m_ambDatabase.m_ambL1[rtk->pppar.m_ix1[i]] != 0.0)
			{
				rtk->pppar.m_ifip[i] = rtk->pppar.m_ambDatabase.m_ambL1[rtk->pppar.m_ix1[i]];
				rtk->pppar.m_matZavp[i] = 1.e-4;
			}
		}
		TransformAmbsToZSpace(nn, rtk->pppar.m_matZp1, rtk->pppar.m_zavp,rtk->pppar.m_ifip);
		rtk->pppar.m_database = 1;
	}
	else
	{
		for (i = 0; i < nn; i++)
		{
			if (aveambdiff1 > 1.5)
				rtk->pppar.m_matZavp[i] = 1.0e4;
		}
	}
}

static int L1fix_lambda(rtk_t *rtk, const int *rsat, double *sdP1, double *sdP2)
{
	prcopt_t *opt  =&rtk->opt;
	int i,j,nx=rtk->nx,num1=0, num2=0;
	int nn;
	double X0[MAXFAMB];
	double X1[MAXFAMB], Q1[MAXFAMB*MAXFAMB], fPdop1 = 0.0;
	double X2[MAXFAMB], Q2[MAXFAMB*MAXFAMB], fPdop2 = 0.0;
	double m_matLp[MAXFAMB*MAXFAMB],m_matDp[MAXFAMB];
	double scand[MAXAMBCAND], Mtp1[MAXFAMB*MAXFAMB];
	double s[2], sr = 0.0;
	double sw[1];

	rtk->pppar.m_solStat.nFixedAmb_L1p=0;

	num2 = check_L1amb_PartialFixing(rsat,&fPdop2,X2,rtk);

	if (fPdop2>rtk->pppar.m_prcOptAr.fMaxPdop)
	{
		return 0;
	}

	rtk->pppar.m_ip=0;
	rtk->pppar.m_nfi=num2;

	for (i=0;i<num2;i++)
	{
		for (j=0;j<num2;j++)
		{
			Q2[i*num2+j]=sdP2[IB(rtk->pppar.m_ix2[i]+1,0,opt)*nx+IB(rtk->pppar.m_ix2[j]+1,0,opt)]
						 /rtk->opt.lam[rtk->pppar.m_ix2[i]][0]/rtk->opt.lam[rtk->pppar.m_ix2[j]][0];
			rtk->pppar.m_matZp2[i*num2+j]=(i==j?1.0:0.0);
		}
	}

	lambda_reduction(num2,X2,Q2,m_matLp,rtk->pppar.m_matZp2,m_matDp,rtk->pppar.m_zflp2);

	while (num2-rtk->pppar.m_ip>=4)
	{
		nn=num2-rtk->pppar.m_ip;
		/*LAMBDA method is applied to fix narrow-lane ambiguity*/
		sr=1.0;
		for (i=rtk->pppar.m_ip;i<num2;i++)
			sr*=pBootStrapping(sqrt(m_matDp[i]));

		for (i=0;i<nn;i++) 
			for (j=0;j<nn;j++)
				Mtp1[i*nn+j]=m_matLp[(i+rtk->pppar.m_ip)*num2+(j+rtk->pppar.m_ip)];

		search(nn, 2, Mtp1, m_matDp + rtk->pppar.m_ip, rtk->pppar.m_zflp2 + rtk->pppar.m_ip, rtk->pppar.m_zfip + rtk->pppar.m_ip, s);

		rtk->pppar.m_solStat.fRRatio=s[1]/s[0];
		if (rtk->pppar.m_solStat.fRRatio>999.99) rtk->pppar.m_solStat.fRRatio=999.99;
		rtk->pppar.m_solStat.fSRate_boot=sr;

		trace(2,"fixed amb: fRRatio=%8.4f fSRate=%8.4f\n",rtk->pppar.m_solStat.fRRatio,rtk->pppar.m_solStat.fSRate_boot);
		for (i = 0; i < num2; i++)
		{
		   trace(2,"amb%d %8.4f\n",i+1,rtk->pppar.m_zfip[i]);
		}

		if (rtk->pppar.m_solStat.fRRatio<rtk->pppar.m_prcOptAr.fMinRRatio||rtk->pppar.m_solStat.fSRate_boot<rtk->pppar.m_prcOptAr.fMinSr)
		{
			if (rtk->pppar.m_prcOptAr.bPAR==0)
				break;
			else
				rtk->pppar.m_ip++;
		}
		else
		{
			break;
		}
	}



	if (num2-rtk->pppar.m_ip<4) return 0;
	if (rtk->pppar.m_solStat.fRRatio<rtk->pppar.m_prcOptAr.fMinRRatio) return 0;
	if (rtk->pppar.m_solStat.fSRate_boot<rtk->pppar.m_prcOptAr.fMinSr) return 0;

	rtk->pppar.m_solStat.nFixedAmb_L1p=num2-rtk->pppar.m_ip;

	return 2;
}

static int Try_fix_zamb(const int nn,rtk_t *rtk)
{
	int i, allfixed = 1;
	for (i = 0; i < nn; i++)
	{
		if (fabs(frac(rtk->pppar.m_zavp[i])) ==0.0)
		{
			rtk->pppar.m_matZavp[i] = 1.0e-4;
		}
		else if (fabs(frac(rtk->pppar.m_zavp[i])) < 1.0e-4)
		{
			rtk->pppar.m_zavp[i] = myRound(rtk->pppar.m_zavp[i]);
			rtk->pppar.m_matZavp[i] = 1.0e-4;
		}
		else
		{
			allfixed = 0;
		}
	}

	return allfixed;
}

static int TransformAmbsToNormSpace(const int nn, double* matZ, double *zavp, double *amb,rtk_t *rtk)
{
	double m_matZp1[MAXFAMB*MAXFAMB];
	double m_zavp[MAXFAMB];
	int i,info;
	matcpy(m_matZp1, matZ, nn, nn);
	matcpy(m_zavp, zavp, nn, 1);
	if (!(info = matinv(m_matZp1, nn)))
	{
		matmul("TN", nn, 1, nn, 1.0, m_matZp1, m_zavp, 0.0, amb);

		for (i = 0; i < nn; i++)
		{
			if (fabs(frac(amb[i])) < 1.0e-3 )
			{
				amb[i] = myRound(amb[i]);
				rtk->pppar.m_matZavp[i] = 1.0e-4;
			}	
		}
		return 1;
	}

	return 0;
}


extern void extL1amb_float_PartialFixing(rtk_t *rtk, const int *rsat, double *sdvc)
{
	prcopt_t *opt=&rtk->opt;
	PPPAr_t  *pppar=&rtk->pppar;
	double C1,C2,*DP,*Qy,*y,*D;
	int i,j,sysId,nx=rtk->nx;
	double l1bias_rsat,l1bias_sat;

	D=mat(nx,nx);
	DP=mat(nx,nx);
	Qy=mat(nx,nx);
	y=mat(nx,1);

	for (i=0;i<nx*nx;i++)       D[i]=0.0;
	for (i=0;i<pppar->m_na;i++) D[i+i*nx]=1.0;
	for (i=0;i<MAXSAT;i++)      rtk->ssat[i].fix[0]=0;

	for (j=0; j < 3; j++)
	{
	  if (rsat[j]==-1) continue;

	  if (j==1)
	  {
		 C1 =  1.0 +  opt->lam[rsat[j] - 1][0] / opt->lam[rsat[j] - 1][2];
		 C2 = -1.0 / (opt->lam[rsat[j] - 1][2] / opt->lam[rsat[j] - 1][0] - 1.0);
	  }
	  else
	  {
		 C1 =  1.0 +  opt->lam[rsat[j] - 1][0] / opt->lam[rsat[j] - 1][1];
		 C2 = -1.0 / (opt->lam[rsat[j] - 1][1] / opt->lam[rsat[j] - 1][0] - 1.0);
      }

	  pppar->m_satAmb2[rsat[j] - 1].l1amb[0] = (pppar->m_satAmb2[rsat[j] - 1].ifamb[0])*C1 - pppar->m_satAmb2[rsat[j] - 1].mwamb[1] * C2;
	  pppar->m_satAmb2[rsat[j] - 1].l1amb[1] =  myRound(pppar->m_satAmb2[rsat[j] - 1].l1amb[0]);
	  pppar->m_satAmb2[rsat[j] - 1].l1sig    = 1.0e-5;
	  pppar->m_satAmb2[rsat[j] - 1].ifsig    = sqrt(fabs(sdvc[IB(rsat[j], 0, opt)*(nx + 1)])) / opt->lam[rsat[j] - 1][0];
	}

	for (i=0;i<MAXSAT;i++)
	{
		   sysId=satsysId(i+1);
		   if (sysId==-1)       continue;
		   if (pppar->m_satAmb2[i].bfxied[0]==0 || i+1==rsat[sysId]) continue;

		   rtk->ssat[i].fix[0]=1;
		   D[IB(rsat[sysId],0,opt)+IB(i+1,0,opt)*nx]=-1.0;
		   D[IB(i+1,0,opt)+IB(i+1,0,opt)*nx]=+1.0;

		   if (sysId==1)
		   {
			  C1= 1.0+opt->lam[i][0]/opt->lam[i][2];
			  C2=-1.0/(opt->lam[i][2]/opt->lam[i][0]-1.0);
		   }
		   else
		   {
			  C1= 1.0+opt->lam[i][0]/opt->lam[i][1];
			  C2=-1.0/(opt->lam[i][1]/opt->lam[i][0]-1.0);
		   }

		   pppar->m_satAmb2[i].l1amb[0]=( pppar->m_satAmb2[i].ifamb[0]-pppar->m_satAmb2[rsat[sysId]-1].ifamb[0])*C1
										 +pppar->m_satAmb2[i].mwamb[1]*C2;
	}

	/* transform zero to single-differenced phase-bias (y=D'*x, Qy=D'*P*D) */
	matmul("TN",nx, 1,nx,1.0,D ,rtk->x,0.0,y );
	matmul("TN",nx,nx,nx,1.0,D ,rtk->P,0.0,DP);
	matmul("NN",nx,nx,nx,1.0,DP,D     ,0.0,Qy);
	matcpy(sdvc,Qy,nx,nx);

	for (i=0;i<MAXSAT;i++)
	{
		sysId=satsysId(i+1);
		if (sysId              ==-1) continue;
		if (rtk->ssat[i].vsat[0]==0) continue;
		pppar->m_satAmb2[i].ifsig=sqrt(fabs(sdvc[IB(i+1,0,opt)*(nx+1)]))/opt->lam[i][0];
		if (sysId==1)
		 pppar->m_satAmb2[i].l1sig=pppar->m_satAmb2[i].ifsig*(1.0+opt->lam[i][0]/opt->lam[i][2]);
		else
		 pppar->m_satAmb2[i].l1sig=pppar->m_satAmb2[i].ifsig*(1.0+opt->lam[i][0]/opt->lam[i][1]);

		 trace(2,"extL1amb: sat %2d refsat %2d l1amb %8.3f l1sig %6.3f ifamb %8.3f mwamb %8.3f\n",
		 i+1, rsat[sysId], pppar->m_satAmb2[i].l1amb[0],pppar->m_satAmb2[i].l1sig,
						   pppar->m_satAmb2[i].ifamb[0],pppar->m_satAmb2[i].mwamb[1]);
	}

 /*	for (i = 0; i < MAXSAT; i++)
	{
		sysId=satsysId(i+1);
		if (sysId                ==-1) continue;
		if (rtk->ssat[i].vsat[0] == 0) continue;

		if (pppar->m_ambDatabase.m_varambL1[i] != 0.0 && pppar->m_ambDatabase.m_varambL1[i] < SQR(pppar->m_satAmb2[i].l1sig))
		{
			pppar->m_satAmb2[i].l1amb[0]     =       pppar->m_ambDatabase.m_ambL1[i]
												   - pppar->m_ambDatabase.m_ambL1[pppar->m_ambDatabase.refsatL1[sysId] - 1];
			pppar->m_satAmb2[i].l1sig        = SQRT( pppar->m_ambDatabase.m_ambL1[i]);
		}
	} */

	free(D); free(DP); free(Qy); free(y);
}

static void extL1amb_float_IntegerWeightedAve(rtk_t *rtk, const int *rsat, double *sdvc)
{
	prcopt_t *opt = &rtk->opt;
	PPPAr_t  *pppar=&rtk->pppar;
	double C1, C2, *DP, *Qy, *y, *D;
	int i,j, nx = rtk->nx;
	double l1bias_rsat, l1bias_sat;

	D = mat(nx, nx);
	DP = mat(nx, nx);
	Qy = mat(nx, nx);
	y = mat(nx, 1);

	for (j = 0; j < 3; j++)
	{
		if (rsat[j]==-1) return;
		for (i = 0; i < nx*nx;       i++) D[i] = 0.0;
		for (i = 0; i < pppar->m_na; i++) D[i + i*nx] = 1.0;
		for (i = 0; i < MAXSAT;      i++) rtk->ssat[i].fix[0] = 0;
		C1 =  1.0 +  opt->lam[rsat[j] - 1][0] / opt->lam[rsat[j] - 1][1];
		C2 = -1.0 /  opt->lam[rsat[j] - 1][1] /(opt->lam[rsat[j] - 1][0] - 1.0);

		for (i = 0; i < MAXSAT; i++)
		{
		   if (i + 1 == rsat[j]) continue;
		   D[IB(rsat[j], 0, opt) + IB(i + 1, 0, opt)*nx] = -1.0;
		   D[IB(i + 1, 0  , opt) + IB(i + 1, 0, opt)*nx] = +1.0;

		   if (opt->lam[i][0]==0.0 || opt->lam[i][1]==0.0)  continue;

		   C1 =  1.0 + opt->lam [i][0] / opt->lam[i][1];
		   C2 = -1.0 / (opt->lam[i][1] / opt->lam[i][0] - 1.0);

		   pppar->m_satAmb1[i].l1amb[0] = ( pppar->m_satAmb1[i].ifamb[0] - pppar->m_satAmb1[rsat[j] - 1].ifamb[0])*C1
										  + pppar->m_satAmb1[i].mwamb[1] * C2;


		}
	}

	/* transform zero to single-differenced phase-bias (y=D'*x, Qy=D'*P*D) */
	matmul("TN", nx,  1, nx, 1.0,  D, rtk->x, 0.0,  y);
	matmul("TN", nx, nx, nx, 1.0,  D, rtk->P, 0.0, DP);
	matmul("NN", nx, nx, nx, 1.0, DP,      D, 0.0, Qy);
	matcpy(sdvc, Qy, nx, nx);
	free(D); free(DP); free(Qy); free(y);
}

static void holdAmb(rtk_t *rtk)
{
	double dt,fact;
	int i,j,nv=0,nx=rtk->nx;
	double dlam_gps=CLIGHT/FREQ1;
	double dl1_if_gps=FREQ1/(FREQ1+FREQ2)*dlam_gps;
	double R[MAXFAMB*MAXFAMB];
	double v[100],var[100],*H;

	H=mat(MAXFAMB,nx);
	for (i=0;i<MAXFAMB;i++)
		for (j=0;j<nx;j++)
			H[i*nx+j]=0.0;

	for (i=rtk->pppar.m_ip;i<rtk->pppar.m_nfi;i++)
	{
		fact=0.0;
		for (j=0;j<rtk->pppar.m_nfi;j++)
		{
			dt=rtk->pppar.m_matZp2[i*rtk->pppar.m_nfi+j];
			H[nv*nx+IB(rtk->pppar.m_ix2[j]+1,0,&rtk->opt)] =dt;
			H[nv*nx+IB(rtk->pppar.iRsGNSS[j],0,&rtk->opt)]-=dt;
			fact+=1.0;
		}
		v[nv] =rtk->pppar.m_zfip[i]-rtk->pppar.m_zflp2[i];
		v[nv]*=dl1_if_gps;

		var[nv]=SQR(rtk->pppar.m_prcOptAr.fAmbErr_hold[0]*dlam_gps);
		var[nv]*=fact;

		nv++;
	}
	for (i=0;i<nv*nv;i++) R[i]=0.0;
	for (i=0;i<nv   ;i++) R[i+i*nv]=var[i];

	/* update states with constraints */
	if ((j=filter_fixedamb(rtk->x,rtk->P, rtk->xa, rtk->Pa,H,v,R,nx,nv)))
	{
		sprintf(rtk->opt.chMsg,"holdAmb filter error (info=%d)\n",j);
	}

	trace(2,"PPP float pos=%10.4f %10.4f %10.4f fixed pos=%10.4f %10.4f %10.4f\n",
	rtk->x[0],rtk->x[1],rtk->x[2],rtk->xa[0],rtk->xa[1],rtk->xa[2] );

	free(H);
}

static void holdWeightedAveAmb(rtk_t *rtk)
{
	double dt, fact, deltaPosHor, deltaPosVer, deltapos_thres1,deltapos_thres2;
	int i, j, Usefloatsolu1,Usefloatsolu2, nv = 0, nx = rtk->nx;
	double dlam_gps = CLIGHT / FREQ1;
	double dl1_if_gps = FREQ1 / (FREQ1 + FREQ2)*dlam_gps;
	double R[MAXFAMB*MAXFAMB];
	double v[100], var[100], *H;

	H = mat(MAXFAMB, nx);
	for (i = 0; i < MAXFAMB; i++)
		for (j = 0; j < nx; j++)
			H[i*nx + j] = 0.0;

	for (i = 0; i < rtk->pppar.m_nwa; i++)
	{
		fact = 0.0;
		for (j = 0; j < rtk->pppar.m_nwa; j++)
		{
			dt = rtk->pppar.m_matZp1[i*rtk->pppar.m_nwa + j];
			H[nv*nx + IB(rtk->pppar.m_ix1[j] + 1, 0, &rtk->opt)]  = dt;
			H[nv*nx + IB(rtk->pppar.iRsGNSS[j], 0, &rtk->opt)] -= dt;
			fact += 1.0;
		}
		v[nv] = rtk->pppar.m_zavp[i] - rtk->pppar.m_zflp1[i];
		v[nv] *= dl1_if_gps;

		var[nv] = SQR(rtk->pppar.m_matZavp[i] * dlam_gps);
		var[nv] *= fact;

		nv++;
	}
	for (i = 0; i < nv*nv; i++) R[i] = 0.0;
	for (i = 0; i < nv   ; i++) R[i + i * nv] = var[i];

	/* update states with constraints */
	if ((j = filter_fixedamb(rtk->x, rtk->P, rtk->xw, rtk->Pw, H, v, R, nx, nv)))
	{
	  sprintf(rtk->opt.chMsg, "holdAmb filter error (info=%d)\n", j);
	}

	rtk->convergetime++;

	deltaPosHor = SQRT(SQR(rtk->xw[0]- rtk->x[0]) + SQR(rtk->xw[1] - rtk->x[1]));

	deltaPosVer = fabs(rtk->xw[2] - rtk->x[2]);

	deltapos_thres1 = 0.10;
	deltapos_thres2 = 0.10;

	trace(2,"convergetime: %6.1f\n",rtk->convergetime);

	Usefloatsolu2 = (deltaPosHor > deltapos_thres2 || deltaPosVer > deltapos_thres2);

	Usefloatsolu1 = (deltaPosHor > deltapos_thres1 || deltaPosVer > deltapos_thres1) && !rtk->pppar.m_database && rtk->convergetime <= 300;

	if ( rtk->convergetime <= 600 || Usefloatsolu1 || Usefloatsolu2 )
	{
		rtk->sol.stat = SOLQ_PPP;
		for (i = 0; i < 3; i++)
		{
			rtk->sol.rr[i] = rtk->x[i];
			rtk->sol.qr[i] = (float)rtk->P[i + i * rtk->na];
		}
		rtk->sol.qr[3] = (float)rtk->P[1];
		rtk->sol.qr[4] = (float)rtk->P[1 + 2 * rtk->na];
		rtk->sol.qr[5] = (float)rtk->P[2];
	}
	else 
	{
		rtk->sol.stat = SOLQ_WAF;
		for (i = 0; i < 3; i++)
		{
			rtk->sol.rr[i] = rtk->xw[i];
			rtk->sol.qr[i] = (float)rtk->Pw[i + i * rtk->na];
		}
		rtk->sol.qr[3] = (float)rtk->Pw[1];
		rtk->sol.qr[4] = (float)rtk->Pw[1 + 2 * rtk->na];
		rtk->sol.qr[5] = (float)rtk->Pw[2];
	}

	trace(2,"PPP pos float=%10.4f %10.4f %10.4f wia=%10.4f %10.4f %10.4f\n",
	rtk->x[0],rtk->x[1],rtk->x[2],rtk->xw[0],rtk->xw[1],rtk->xw[2] );

	free(H);
}

static void PPPARInit(rtk_t *rtk)
{
	int i,sysId;
	for (i = 0; i < MAXSAT; i++)
	{
		sysId=satsysId(i+1);
		if (sysId ==-1) continue;

		rtk->pppar.m_ambDatabase.refsatL1[sysId] = 0;
		rtk->pppar.m_ambDatabase.refsatWL[sysId] = 0;
		rtk->pppar.m_ambDatabase.m_ratio       = 0.0;
		rtk->pppar.m_ambDatabase.m_varambWL[i] = 0.0;
		rtk->pppar.m_ambDatabase.m_ambWL   [i] = 0.0;
		rtk->pppar.m_ambDatabase.m_ixWL    [i] = 0;
		rtk->pppar.m_ambDatabase.m_varambWL[i] = 0.0;
		rtk->pppar.m_ambDatabase.m_ambL1   [i] = 0.0;
		rtk->pppar.m_ambDatabase.m_ixL1    [i] = 0;
		rtk->pppar.m_ambDatabase.m_varambL1[i] = 0.0;
	}
}

static int ambfix(rtk_t *rtk, const int *rsat)
{
	int i,WL_fixed=0,L1_fixed_status=0,nx=rtk->nx;
	int successfixed=0;
	double *m_mat_sdP1;
	double *m_mat_sdP2;

	if (rtk->convergetime == 0)
		PPPARInit(rtk);

	m_mat_sdP1 = mat(nx,nx);
	m_mat_sdP2 = mat(nx, nx);
	rtk->pppar.m_solStat.bFixed=0;
	rtk->pppar.m_na=IB(1,0,&rtk->opt);

	WL_fixed=WLfix_round(rsat,rtk);

	successfixed = 0;

	if (WL_fixed==1)
	{

		extL1amb_float_PartialFixing(rtk,rsat,m_mat_sdP2);
	}

	L1_fixed_status = L1fix_lambda(rtk, rsat, m_mat_sdP1, m_mat_sdP2);

	if (rtk->sol.stat == SOLQ_PPP)
		successfixed = 0;

	if ( L1_fixed_status == 2 )
	{
		rtk->pppar.m_solStat.nSeqFixedEp++;
		if (rtk->opt.modear>ARMODE_PPPAR_OFF && rtk->pppar.m_solStat.nSeqFixedEp>=3)
		{
			successfixed = 2;
			holdAmb(rtk);
		}
	}

	if (successfixed<2) 
	{
		for (i=0;i<MAXSAT;i++)
		{
			rtk->pppar.m_satAmb2[i].nfix=0;
		}
	}
	
	free(m_mat_sdP1);  free(m_mat_sdP2);
	rtk->pppar.m_solStat.bFixed=successfixed;
	return successfixed;
}

extern void pppamb(rtk_t *rtk, const int *rsat)
{
	rtk->pppar.m_nObs=getAmbProperty(rtk);
 	ambfix(rtk,rsat);
}


