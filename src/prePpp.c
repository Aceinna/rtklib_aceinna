#include <sys/types.h>
/*#include <dirent.h>*/
#include "rtklib.h"

/*init cPPPAr struct*/
extern void initcPPPAr(PPPAr_t *pppar, int armode)
{
	int i;

	pppar->m_na=0;
	pppar->m_nObs=0;

	if (armode==0)
		pppar->m_prcOptAr.iArMode=ARMODE_PPPAR_OFF;
	else
		pppar->m_prcOptAr.iArMode=ARMODE_PPPAR_HOLD;

	pppar->m_prcOptAr.bPAR=1;
	pppar->m_prcOptAr.bFFRTUsed=0;
	pppar->m_prcOptAr.nLockEpMin[0]=5;
	pppar->m_prcOptAr.nLockEpMin[1]=6;
	pppar->m_prcOptAr.fMinSr      =0.98;
	pppar->m_prcOptAr.fMinElAr_deg=12.0;
	pppar->m_prcOptAr.fMinRRatio=3;
	pppar->m_prcOptAr.fMaxPdop =25.0;
	pppar->m_prcOptAr.fMaxFrac[0]     = 0.35;
	pppar->m_prcOptAr.fMaxFrac[1]     = 0.35 ;
	pppar->m_prcOptAr.fMaxSig [0]     = 0.75 ;
	pppar->m_prcOptAr.fMaxSig [1]     = 2.0  ;
	pppar->m_prcOptAr.fMaxSig [2]     = 1.0  ;
	pppar->m_prcOptAr.fAmbErr_hold[0] = 0.06 ;
	pppar->m_prcOptAr.fAmbErr_hold[1] = 0.05 ;
	pppar->m_solStat.nSeqFixedEp=0;

	for (i=0;i<MAXSAT;i++) 
	{
		pppar->m_upd.wlbias[i]=9999.999;
		pppar->m_ix1[i] = 0;
		pppar->m_ix2[i] = 0;
	}
	for (i=0;i<50;i++)
	{
		pppar->iRsGNSS[i]=0;
	}
}

/*get default configure*/
extern void getdefaultcfg(prcopt_t *prcopt)
{

	int i;
    gtime_t gt0={0};
	ssatEx_t ssat_EX0={{0}};


	prcopt->sample = 1.0;
    prcopt->maxinno   = 30.0;

	for (i=0;i<MAXSAT;i++)
	{
		prcopt->ssat_Ex[i]=ssat_EX0;
		prcopt->ssat_Ex[i].var_ion=0.09;  /*the a priori variance for ionospheric pseudo-observables, the default is 0.09 m^2*/
	}

	prcopt->prcOpt_Ex.bElevCheckEx=0;


	prcopt->prcOpt_Ex.tropMF=TROPMF_GMF;
	prcopt->prcOpt_Ex.isGPT=1;            /*GPT model is used or not (0:standard atmosphere,1:GPT)*/
	prcopt->prcOpt_Ex.ionopnoise=1;       /*0: static  1: random walk  2: random walk (new)  3:white noise*/
	prcopt->prcOpt_Ex.ion_const=0;        /*0: off  1: on*/
	prcopt->prcOpt_Ex.ion_const_mode=3;   /*1:constant constraint  2:spatial-temporal constraint  3:stepwise-relaxed constraint*/

    /*cycle slip set */
	prcopt->prcOpt_Ex.csThresGF =0.0;
	prcopt->prcOpt_Ex.csThresMW =0.0;
	prcopt->prcOpt_Ex.bUsed_gfCs=1;
	prcopt->prcOpt_Ex.bUsed_mwCs=1;

	prcopt->prcOpt_Ex.tPrcUnit=0.0;

	/*measurement error ratio*/
	prcopt->prcOpt_Ex.errRatioGLO=100.0;
	prcopt->prcOpt_Ex.errRatioBDS=500.0;
	prcopt->prcOpt_Ex.errRatioGAL=100.0;
	prcopt->prcOpt_Ex.errRatioQZS=100.0;

	prcopt->prcOpt_Ex.tropMF     = 1;
	prcopt->prcOpt_Ex.isGPT      = 1;

	/*process-noise std for random walk new */
	prcopt->prcOpt_Ex.prn_iono=0.001;

	/*time set */
	prcopt->prcOpt_Ex.bTsSet=0;
	prcopt->prcOpt_Ex.bTeSet=0;
	prcopt->tNow            =gt0;
	prcopt->t_30min         =gt0;
	prcopt->isb_30min       =0.0;
	prcopt->sowNow          =0.0;
	prcopt->nBadEpSPP=0;
	prcopt->bOKSPP=1;

	for (i=0;i<MAXSAT;i++)
	{
		prcopt->sFlag[i].sys=satsys(i+1,&prcopt->sFlag[i].prn);
		satno2id(i+1,prcopt->sFlag[i].id);
	}

	for (i=0;i<=MAXPRNGLO;i++) prcopt->fnGlo[i]=0;
	prcopt->err[0]   	= 100.0;
	prcopt->err[1]      = 0.003;
	prcopt->prn[0] = 1.0e-07;
	prcopt->prn[1] = 4.0e-02;
	prcopt->prn[2] = 1.0e-04;

}




