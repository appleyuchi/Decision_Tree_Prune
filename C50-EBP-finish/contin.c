/*************************************************************************/
/*									 */
/*  Copyright 2010 Rulequest Research Pty Ltd.				 */
/*									 */
/*  This file is part of C5.0 GPL Edition, a single-threaded version	 */
/*  of C5.0 release 2.07.						 */
/*									 */
/*  C5.0 GPL Edition is free software: you can redistribute it and/or	 */
/*  modify it under the terms of the GNU General Public License as	 */
/*  published by the Free Software Foundation, either version 3 of the	 */
/*  License, or (at your option) any later version.			 */
/*									 */
/*  C5.0 GPL Edition is distributed in the hope that it will be useful,	 */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of	 */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU	 */
/*  General Public License for more details.				 */
/*									 */
/*  You should have received a copy of the GNU General Public License	 */
/*  (gpl.txt) along with C5.0 GPL Edition.  If not, see 		 */
/*									 */
/*      <http://www.gnu.org/licenses/>.					 */
/*									 */
/*************************************************************************/



/*************************************************************************/
/*                                                                	 */
/*	Evaluation of a test on a continuous valued attribute	  	 */
/*	-----------------------------------------------------	  	 */
/*								  	 */
/*************************************************************************/

#include "defns.i"
#include "extern.i"

#define	PartInfo(n) (-(n)*Log((n)/GEnv.Cases))


/*************************************************************************/
/*								  	 */
/*	Continuous attributes are treated as if they have possible	 */
/*	values 0 (unknown), 1 (not applicable), 2 (less than cut) and	 */
/*	3 (greater than cut).						 */
/*	This routine finds the best cut for cases Fp through Lp and	 */
/*	sets Info[], Gain[] and Bar[]					 */
/*								  	 */
/*************************************************************************/


void EvalContinuousAtt(Attribute Att, CaseNo Fp, CaseNo Lp)
/*   -----------------  */
{
    CaseNo	i, j, BestI, Tries=0;
    double	LowInfo, LHInfo, LeastInfo=1E38,
		w, BestGain, BestInfo, ThreshCost=1;
    ClassNo	c;
    ContValue	Interval;

    Verbosity(3, fprintf(Of, "\tAtt %s\n", AttName[Att]))

    Gain[Att] = None;
    PrepareForContin(Att, Fp, Lp);

    /*  Special case when very few known values  */

    if ( GEnv.ApplicCases < 2 * MINITEMS )
    {
	Verbosity(2,
	    fprintf(Of, "\tAtt %s\tinsufficient cases with known values\n",
			AttName[Att]))
	return;
    }

    /*  Try possible cuts between cases i and i+1, and determine the
	information and gain of the split in each case  */

    /*  We have to be wary of splitting a small number of cases off one end,
	as this has little predictive power.  The minimum split GEnv.MinSplit is
	the maximum of MINITEMS or (the minimum of 25 and 10% of the cases
	per class)  */

    GEnv.MinSplit = 0.10 * GEnv.KnownCases / MaxClass;
    if ( GEnv.MinSplit > 25 ) GEnv.MinSplit = 25;
    if ( GEnv.MinSplit < MINITEMS ) GEnv.MinSplit = MINITEMS;

    /*	Find first possible cut point and initialise scan parameters  */

    i = PrepareForScan(Lp);

    /*  Repeatedly check next possible cut  */

    for ( ; i <= GEnv.Ep ; i++ )
    {
	c = GEnv.SRec[i].C;
	w = GEnv.SRec[i].W;
	assert(c >= 1 && c <= MaxClass);

	GEnv.LowCases   += w;
	GEnv.Freq[2][c] += w;
	GEnv.Freq[3][c] -= w;

	GEnv.HighVal = GEnv.SRec[i+1].V;
	if ( GEnv.HighVal > GEnv.LowVal )
	{
	    Tries++;

	    GEnv.LowClass  = GEnv.HighClass;
	    GEnv.HighClass = GEnv.SRec[i+1].C;
	    for ( j = i+2 ;
		  GEnv.HighClass && j <= GEnv.Ep && GEnv.SRec[j].V == GEnv.HighVal ;
		  j++ )
	    {
		if ( GEnv.SRec[j].C != GEnv.HighClass ) GEnv.HighClass = 0;
	    }

	    if ( ! GEnv.LowClass || GEnv.LowClass != GEnv.HighClass || j > GEnv.Ep )
	    {
		LowInfo = TotalInfo(GEnv.Freq[2], 1, MaxClass);

		/*  If cannot improve on best so far, count remaining
		    possible cuts and break  */

		if ( LowInfo >= LeastInfo )
		{
		    for ( i++ ; i <= GEnv.Ep ; i++ )
		    {
			if ( GEnv.SRec[i+1].V > GEnv.SRec[i].V )
			{
			    Tries++;
			}
		    }
		    break;
		}

		LHInfo = LowInfo + TotalInfo(GEnv.Freq[3], 1, MaxClass);
		if ( LHInfo < LeastInfo )
		{
		    LeastInfo = LHInfo;
		    BestI     = i;

		    BestInfo = (GEnv.FixedSplitInfo
				+ PartInfo(GEnv.LowCases)
				+ PartInfo(GEnv.ApplicCases - GEnv.LowCases))
			       / GEnv.Cases;
		}

		Verbosity(3,
		{
		    fprintf(Of, "\t\tCut at %.3f  (gain %.3f):",
			   (GEnv.LowVal + GEnv.HighVal) / 2,
			   (1 - GEnv.UnknownRate) *
			   (GEnv.BaseInfo - (GEnv.NAInfo + LHInfo) / GEnv.KnownCases));
		    PrintDistribution(Att, 2, 3, GEnv.Freq, GEnv.ValFreq, true);
		})
	    }

	    GEnv.LowVal = GEnv.HighVal;
	}
    }

    BestGain = (1 - GEnv.UnknownRate) *
	       (GEnv.BaseInfo - (GEnv.NAInfo + LeastInfo) / GEnv.KnownCases);

    /*  The threshold cost is the lesser of the cost of indicating the
	cases to split between or the interval containing the split  */

    if ( BestGain > 0 )
    {
	Interval = (GEnv.SRec[Lp].V - GEnv.SRec[GEnv.Xp].V) /
		   (GEnv.SRec[BestI+1].V - GEnv.SRec[BestI].V);
	ThreshCost = ( Interval < Tries ? Log(Interval) : Log(Tries) )
		     / GEnv.Cases;
    }

    BestGain -= ThreshCost;

    /*  If a test on the attribute is able to make a gain,
	set the best break point, gain and information  */

    if ( BestGain <= 0 )
    {
	Verbosity(2, fprintf(Of, "\tAtt %s\tno gain\n", AttName[Att]))
    }
    else
    {
	Gain[Att] = BestGain;
	Info[Att] = BestInfo;

	GEnv.LowVal  = GEnv.SRec[BestI].V;
	GEnv.HighVal = GEnv.SRec[BestI+1].V;

	/*  Set threshold, making sure that rounding problems do not
	    cause it to reach upper value  */

	if ( (Bar[Att] = (ContValue) (0.5 * (GEnv.LowVal + GEnv.HighVal)))
	     >= GEnv.HighVal )
	{
	    Bar[Att] = GEnv.LowVal;
	}

	Verbosity(2,
	    fprintf(Of, "\tAtt %s\tcut=%.3f, inf %.3f, gain %.3f\n",
		   AttName[Att], Bar[Att], Info[Att], Gain[Att]))
    }
}



/*************************************************************************/
/*                                                                	 */
/*	Estimate max gain ratio available from any cut, using sample	 */
/*	of SampleFrac of all cases					 */
/*                                                                	 */
/*************************************************************************/


void EstimateMaxGR(Attribute Att, CaseNo Fp, CaseNo Lp)
/*   -------------  */
{
    CaseNo	i, j;
    double	LHInfo, w, SplitInfo, ThisGain, GR;
    ClassNo	c;

    EstMaxGR[Att] = 0;

    if ( Skip(Att) || Att == ClassAtt ) return;

    PrepareForContin(Att, Fp, Lp);

    /*  Special case when very few known values  */

    if ( GEnv.ApplicCases < 2 * MINITEMS * SampleFrac )
    {
	return;
    }

    /*  Try possible cuts between cases i and i+1.  Use conservative
	value of GEnv.MinSplit to allow for sampling  */

    GEnv.MinSplit = 0.10 * GEnv.KnownCases / MaxClass;
    if ( GEnv.MinSplit > 25 ) GEnv.MinSplit = 25;
    if ( GEnv.MinSplit < MINITEMS ) GEnv.MinSplit = MINITEMS;

    GEnv.MinSplit *= SampleFrac * 0.33;

    i = PrepareForScan(Lp);

    /*  Repeatedly check next possible cut  */

    for ( ; i <= GEnv.Ep ; i++ )
    {
	c = GEnv.SRec[i].C;
	w = GEnv.SRec[i].W;
	assert(c >= 1 && c <= MaxClass);

	GEnv.LowCases   += w;
	GEnv.Freq[2][c] += w;
	GEnv.Freq[3][c] -= w;

	GEnv.HighVal = GEnv.SRec[i+1].V;
	if ( GEnv.HighVal > GEnv.LowVal )
	{
	    GEnv.LowClass  = GEnv.HighClass;
	    GEnv.HighClass = GEnv.SRec[i+1].C;
	    for ( j = i+2 ;
		  GEnv.HighClass && j <= GEnv.Ep && GEnv.SRec[j].V == GEnv.HighVal ;
		  j++ )
	    {
		if ( GEnv.SRec[j].C != GEnv.HighClass ) GEnv.HighClass = 0;
	    }

	    if ( ! GEnv.LowClass || GEnv.LowClass != GEnv.HighClass || j > GEnv.Ep )
	    {
		LHInfo = TotalInfo(GEnv.Freq[2], 1, MaxClass)
			 + TotalInfo(GEnv.Freq[3], 1, MaxClass);

		SplitInfo = (GEnv.FixedSplitInfo
			    + PartInfo(GEnv.LowCases)
			    + PartInfo(GEnv.ApplicCases - GEnv.LowCases)) / GEnv.Cases;

		ThisGain = (1 - GEnv.UnknownRate) *
			   (GEnv.BaseInfo - (GEnv.NAInfo + LHInfo) / GEnv.KnownCases);
		if ( ThisGain > Gain[Att] ) Gain[Att] = ThisGain;

		/*  Adjust GR to make it more conservative upper bound  */

		GR = (ThisGain + 1E-5) / SplitInfo;
		if ( GR > EstMaxGR[Att] )
		{
		    EstMaxGR[Att] = GR;
		}

		Verbosity(3,
		{
		    fprintf(Of, "\t\tCut at %.3f  (gain %.3f):",
			   (GEnv.LowVal + GEnv.HighVal) / 2, ThisGain);
		    PrintDistribution(Att, 2, 3, GEnv.Freq, GEnv.ValFreq, true);
		})
	    }

	    GEnv.LowVal = GEnv.HighVal;
	}
    }

    Verbosity(2,
	fprintf(Of, "\tAtt %s: max GR estimate %.3f\n",
		    AttName[Att], EstMaxGR[Att]))
}



/*************************************************************************/
/*								  	 */
/*	Routine to set some preparatory values used by both		 */
/*	EvalContinuousAtt and EstimateMaxGR				 */
/*								  	 */
/*************************************************************************/


void PrepareForContin(Attribute Att, CaseNo Fp, CaseNo Lp)
/*   ----------------  */
{
    CaseNo	i;
    ClassNo	c;
    DiscrValue	v;

    /*  Reset frequency tables  */

    ForEach(v, 0, 3)
    {
	ForEach(c, 1, MaxClass)
	{
	    GEnv.Freq[v][c] = 0;
	}
	GEnv.ValFreq[v] = 0;
    }

    /*  Omit and count unknown and N/A values */

    GEnv.Cases = 0;

    if ( SomeMiss[Att] || SomeNA[Att] )
    {
	GEnv.Xp = Lp+1;

	ForEach(i, Fp, Lp)
	{
	    assert(Class(Case[i]) >= 1 && Class(Case[i]) <= MaxClass);

	    GEnv.Cases += Weight(Case[i]);

	    if ( Unknown(Case[i], Att) )
	    {
		GEnv.Freq[ 0 ][ Class(Case[i]) ] += Weight(Case[i]);
	    }
	    else
	    if ( NotApplic(Case[i], Att) )
	    {
		GEnv.Freq[ 1 ][ Class(Case[i]) ] += Weight(Case[i]);
	    }
	    else
	    {
		GEnv.Freq[ 3 ][ Class(Case[i]) ] += Weight(Case[i]);
		GEnv.Xp--;
		GEnv.SRec[GEnv.Xp].V = CVal(Case[i], Att);
		GEnv.SRec[GEnv.Xp].W = Weight(Case[i]);
		GEnv.SRec[GEnv.Xp].C = Class(Case[i]);
	    }
	}

	ForEach(c, 1, MaxClass)
	{
	    GEnv.ValFreq[0] += GEnv.Freq[0][c];
	    GEnv.ValFreq[1] += GEnv.Freq[1][c];
	}

	GEnv.NAInfo = TotalInfo(GEnv.Freq[1], 1, MaxClass);
	GEnv.FixedSplitInfo = PartInfo(GEnv.ValFreq[0]) + PartInfo(GEnv.ValFreq[1]);

	Verbosity(3, PrintDistribution(Att, 0, 1, GEnv.Freq, GEnv.ValFreq, true))
    }
    else
    {
	GEnv.Xp = Fp;

	ForEach(i, Fp, Lp)
	{
	    GEnv.SRec[i].V = CVal(Case[i], Att);
	    GEnv.SRec[i].W = Weight(Case[i]);
	    GEnv.SRec[i].C = Class(Case[i]);

	    GEnv.Freq[3][Class(Case[i])] += Weight(Case[i]);
	}

	ForEach(c, 1, MaxClass)
	{
	    GEnv.Cases += GEnv.Freq[3][c];
	}

	GEnv.NAInfo = GEnv.FixedSplitInfo = 0;
    }

    GEnv.KnownCases  = GEnv.Cases - GEnv.ValFreq[0];
    GEnv.ApplicCases = GEnv.KnownCases - GEnv.ValFreq[1];

    GEnv.UnknownRate = 1.0 - GEnv.KnownCases / GEnv.Cases;

    Cachesort(GEnv.Xp, Lp, GEnv.SRec);

    /*  If unknowns or using sampling, must recompute base information  */

    if ( GEnv.ValFreq[0] > 0 || SampleFrac < 1 )
    {
	/*  Determine base information using GEnv.Freq[0] as temp buffer  */

	ForEach(c, 1, MaxClass)
	{
	    GEnv.Freq[0][c] = GEnv.Freq[1][c] + GEnv.Freq[3][c];
	}

	GEnv.BaseInfo = TotalInfo(GEnv.Freq[0], 1, MaxClass) / GEnv.KnownCases;
    }
    else
    {
	GEnv.BaseInfo = GlobalBaseInfo;
    }
}



/*************************************************************************/
/*								  	 */
/*	Set low and high bounds for scan and initial class		 */
/*	(used by EvalContinuousAtt and EstimateMaxGR)			 */
/*								  	 */
/*************************************************************************/


CaseNo PrepareForScan(CaseNo Lp)
/*     --------------  */
{
    CaseNo	i, j;
    ClassNo	c;
    double	w;

    /*  Find last possible split  */

    GEnv.HighCases = GEnv.LowCases = 0;

    for ( GEnv.Ep = Lp ; GEnv.Ep >= GEnv.Xp && GEnv.HighCases < GEnv.MinSplit ; GEnv.Ep-- )
    {
	GEnv.HighCases += GEnv.SRec[GEnv.Ep].W;
    }

    /*  Skip cases before first possible cut  */

    for ( i = GEnv.Xp ;
	  i <= GEnv.Ep &&
	  ( GEnv.LowCases + GEnv.SRec[i].W < GEnv.MinSplit - 1E-5 ||
	    GEnv.SRec[i].V == GEnv.SRec[i+1].V ) ;
	  i++ )
    {
	c = GEnv.SRec[i].C;
	w = GEnv.SRec[i].W;
	assert(c >= 1 && c <= MaxClass);

	GEnv.LowCases   += w;
	GEnv.Freq[2][c] += w;
	GEnv.Freq[3][c] -= w;
    }

    /*  Find the class key for the first interval  */

    GEnv.HighClass = GEnv.SRec[i].C;
    for ( j = i-1; GEnv.HighClass && j >= GEnv.Xp ; j-- )
    {
	if ( GEnv.SRec[j].C != GEnv.HighClass ) GEnv.HighClass = 0;
    }
    assert(GEnv.HighClass <= MaxClass);
    assert(j+1 >= GEnv.Xp);

    GEnv.LowVal = GEnv.SRec[i].V;

    return i;
}



/*************************************************************************/
/*                                                                	 */
/*	Change a leaf into a test on a continuous attribute           	 */
/*                                                                	 */
/*************************************************************************/


void ContinTest(Tree Node, Attribute Att)
/*   ----------  */
{
    Sprout(Node, 3);

    Node->NodeType = BrThresh;
    Node->Tested   = Att;
    Node->Cut 	   =
    Node->Lower	   =
    Node->Upper    = Bar[Att];
}



/*************************************************************************/
/*                                                                	 */
/*	Adjust thresholds of all continuous attributes so that cuts	 */
/*	are values that appear in the data				 */
/*                                                                	 */
/*************************************************************************/


void AdjustAllThresholds(Tree T)
/*   -------------------  */
{
    Attribute	Att;
    CaseNo	Ep;

    ForEach(Att, 1, MaxAtt)
    {
	if ( Continuous(Att) )
	{
	    Ep = -1;
	    AdjustThresholds(T, Att, &Ep);
	}
    }
}



void AdjustThresholds(Tree T, Attribute Att, CaseNo *Ep)
/*   ----------------  */
{
    DiscrValue	v;
    CaseNo	i;

    if ( T->NodeType == BrThresh && T->Tested == Att )
    {
	if ( *Ep == -1 )
	{
	    ForEach(i, 0, MaxCase)
	    {
		if ( ! Unknown(Case[i], Att) && ! NotApplic(Case[i], Att) )
		{
		    (&GEnv)->SRec[++(*Ep)].V = CVal(Case[i], Att);
		}
	    }
	    Cachesort(0, *Ep, (&GEnv)->SRec);

	    if ( PossibleCuts && Trial == 0 )
	    {
		int Cuts=0;

		ForEach(i, 1, *Ep)
		{
		    if ( (&GEnv)->SRec[i].V != (&GEnv)->SRec[i-1].V ) Cuts++;
		}
		PossibleCuts[Att] = Cuts;
	    }
	}

	T->Cut = T->Lower = T->Upper = GreatestValueBelow(T->Cut, Ep);
    }

    if ( T->NodeType )
    {
	ForEach(v, 1, T->Forks)
	{
	    AdjustThresholds(T->Branch[v], Att, Ep);
	}
    }
}



/*************************************************************************/
/*                                                                	 */
/*	Return the greatest value of attribute Att below threshold Th  	 */
/*	(Assumes values of Att have been sorted.)			 */
/*                                                                	 */
/*************************************************************************/


ContValue GreatestValueBelow(ContValue Th, CaseNo *Ep)
/*	  ------------------  */
{
    CaseNo	Low, Mid, High;

    Low  = 0;
    High = *Ep;

    while ( Low < High )
    {
	Mid = (Low + High + 1) / 2;

	if ( (&GEnv)->SRec[Mid].V > Th )
	{
	    High = Mid - 1;
	}
	else
	{
	    Low = Mid;
	}
    }

    return (&GEnv)->SRec[Low].V;
}
