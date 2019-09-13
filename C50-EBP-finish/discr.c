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
/*									 */
/*	Evaluation of a test on a discrete valued attribute		 */
/*      ---------------------------------------------------		 */
/*									 */
/*************************************************************************/

#include "defns.i"
#include "extern.i"


/*************************************************************************/
/*									 */
/*	Set Info[] and Gain[] for discrete partition of cases		 */
/*									 */
/*************************************************************************/


void EvalDiscreteAtt(Attribute Att, CaseCount Cases)
/*   ---------------  */
{
    CaseCount	KnownCases;
    int		ReasonableSubsets=0;
    DiscrValue	v;
    double	BaseInfo;

    SetDiscrFreq(Att);
    KnownCases = Cases - GEnv.ValFreq[0];

    /*  Check reasonable subsets  */

    ForEach(v, 1, MaxAttVal[Att])
    {
	if ( GEnv.ValFreq[v] >= MINITEMS ) ReasonableSubsets++;
    }

    if ( ReasonableSubsets < 2 )
    {
	Verbosity(2, fprintf(Of, "\tAtt %s: poor split\n", AttName[Att]))
	return;
    }

    BaseInfo = ( ! GEnv.ValFreq[0] ? GlobalBaseInfo :
		     DiscrKnownBaseInfo(KnownCases, MaxAttVal[Att]) );

    Gain[Att] = ComputeGain(BaseInfo, GEnv.ValFreq[0] / Cases, MaxAttVal[Att],
			    KnownCases);
    Info[Att] = TotalInfo(GEnv.ValFreq, 0, MaxAttVal[Att]) / Cases;

    Verbosity(2,
    {
    	fprintf(Of, "\tAtt %s", AttName[Att]);
    	Verbosity(3,
	    PrintDistribution(Att, 0, MaxAttVal[Att], GEnv.Freq, GEnv.ValFreq,
			      true))
    	fprintf(Of, "\tinf %.3f, gain %.3f\n", Info[Att], Gain[Att]);
    })
}



/*************************************************************************/
/*									 */
/*	Set Info[] and Gain[] for ordered split on cases		 */
/*									 */
/*************************************************************************/


void EvalOrderedAtt(Attribute Att, CaseCount Cases)
/*   --------------  */
{
    CaseCount	KnownCases;
    double	*HoldFreqRow, SplitFreq[4];
    ClassNo	c;
    int		Tries=0;
    DiscrValue	v, BestV;
    double	BaseInfo, ThisGain, BestInfo, BestGain=None;

    SetDiscrFreq(Att);
    KnownCases = Cases - GEnv.ValFreq[0];

    BaseInfo = ( ! GEnv.ValFreq[0] ? GlobalBaseInfo :
		     DiscrKnownBaseInfo(KnownCases, MaxAttVal[Att]) );

    Verbosity(2, fprintf(Of, "\tAtt %s", AttName[Att]))
    Verbosity(3, PrintDistribution(Att, 0, MaxAttVal[Att], GEnv.Freq,
				   GEnv.ValFreq, true))

    /*  Move elts of Freq[] starting with the third up one place
	and aggregate class frequencies  */

    HoldFreqRow = GEnv.Freq[MaxAttVal[Att]+1];
    ForEach(c, 1, MaxClass)
    {
	HoldFreqRow[c] = 0;
    }
    SplitFreq[0] = GEnv.ValFreq[0];
    SplitFreq[1] = GEnv.ValFreq[1];
    SplitFreq[2] = GEnv.ValFreq[2];
    SplitFreq[3] = 0;

    for ( v = MaxAttVal[Att] ; v > 2 ; v-- )
    {
	GEnv.Freq[v+1] = GEnv.Freq[v];
	ForEach(c, 1, MaxClass)
	{
	    HoldFreqRow[c] += GEnv.Freq[v][c];
	}
	SplitFreq[3] += GEnv.ValFreq[v];
    }

    GEnv.Freq[3] = HoldFreqRow;

    /*  Try various cuts, saving the one with maximum gain  */

    ForEach(v, 3, MaxAttVal[Att])
    {
	if ( GEnv.ValFreq[v] > 0 &&
	     SplitFreq[2] >= MINITEMS && SplitFreq[3] >= MINITEMS )
	{
	    Tries++;
	    ThisGain =
		ComputeGain(BaseInfo, GEnv.ValFreq[0] / Cases, 3, KnownCases);

	    if ( ThisGain > BestGain )
	    {
		BestGain = ThisGain;
		BestInfo = TotalInfo(SplitFreq, 0, 3) / Cases;
		BestV    = v-1;
	    }

	    Verbosity(3,
	    {   fprintf(Of, "\t\tFrom %s (gain %.3f)",
			AttValName[Att][v], ThisGain);
		PrintDistribution(Att, 0, 3, GEnv.Freq, GEnv.ValFreq, false);
	    })
	}

	/*  Move val v from right branch to left branch  */

	ForEach(c, 1, MaxClass)
	{
	    GEnv.Freq[2][c] += GEnv.Freq[v+1][c];
	    GEnv.Freq[3][c] -= GEnv.Freq[v+1][c];
	}
	SplitFreq[2] += GEnv.ValFreq[v];
	SplitFreq[3] -= GEnv.ValFreq[v];
    }

    if ( Tries > 1 ) BestGain -= Log(Tries) / Cases;

    /*  If a test on the attribute is able to make a gain,
	set the best break point, gain and information  */

    if ( BestGain <= 0 )
    {
	Verbosity(2, fprintf(Of, "\tno gain\n"))
    }
    else
    {
	Gain[Att] = BestGain;
	Info[Att] = BestInfo;
	Bar[Att]  = BestV;

	Verbosity(2,
	    fprintf(Of, "\tcut=%g, inf %.3f, gain %.3f\n",
		   Bar[Att], Info[Att], Gain[Att]))
    }
}



/*************************************************************************/
/*									 */
/*	Compute frequency tables Freq[][] and ValFreq[] for attribute	 */
/*	Att for current cases						 */
/*									 */
/*************************************************************************/


void SetDiscrFreq(Attribute Att)
/*   ------------  */
{
    ClassNo	c;
    DiscrValue	v;
    int		x;

    /*  Determine the frequency of each possible value for the
	given attribute  */

    ForEach(v, 0, MaxAttVal[Att])
    {
	GEnv.ValFreq[v] = 0;

	x = v * MaxClass;
	ForEach(c, 1, MaxClass)
	{
	    GEnv.ValFreq[v] += (GEnv.Freq[v][c] = DFreq[Att][x + (c-1)]);
	}
    }
}



/*************************************************************************/
/*									 */
/*	Return the base info for cases with known values of a discrete	 */
/*	attribute, using the frequency table Freq[][]			 */
/*	 								 */
/*************************************************************************/


double DiscrKnownBaseInfo(CaseCount KnownCases, DiscrValue MaxVal)
/*     ------------------  */
{
    ClassNo	c;
    CaseCount	ClassCount;
    DiscrValue	v;

    if ( KnownCases < 1E-5 ) return 0.0;

    ForEach(c, 1, MaxClass)
    {
	ClassCount = 0;
	ForEach(v, 1, MaxVal)
	{
	    ClassCount += GEnv.Freq[v][c];
	}
	GEnv.ClassFreq[c] = ClassCount;
    }

    return TotalInfo(GEnv.ClassFreq, 1, MaxClass) / KnownCases;
}



/*************************************************************************/
/*									 */
/*	Construct and return a node for a test on a discrete attribute	 */
/*									 */
/*************************************************************************/


void DiscreteTest(Tree Node, Attribute Att)
/*   ------------  */
{
    int		S, Bytes;
    DiscrValue	v, CutV;

    if ( Ordered(Att) )
    {
	Sprout(Node, 3);

	Node->NodeType	= BrSubset;
	Node->Tested	= Att;

	Bytes = (MaxAttVal[Att]>>3) + 1;
	Node->Subset = AllocZero(4, Set);

	ForEach(S, 1, 3)
	{
	    Node->Subset[S] = AllocZero(Bytes, Byte);
	}

	Node->Cut = CutV = Bar[Att] + 0.1;

	SetBit(1, Node->Subset[1]);
	ForEach(v, 2, MaxAttVal[Att])
	{
	    S = ( v <= CutV ? 2 : 3 );
	    SetBit(v, Node->Subset[S]);
	}
    }
    else
    {
	Sprout(Node, MaxAttVal[Att]);

	Node->NodeType = BrDiscr;
	Node->Tested   = Att;
    }
}
