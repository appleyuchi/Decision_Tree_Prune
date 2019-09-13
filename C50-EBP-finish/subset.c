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
/*      Evaluation of discrete attribute subsets			 */
/*      ----------------------------------------			 */
/*									 */
/*************************************************************************/


#include "defns.i"
#include "extern.i"



/*************************************************************************/
/*									 */
/*	Set up tables for subsets					 */
/*									 */
/*************************************************************************/


void InitialiseBellNumbers()
/*   ---------------------  */
{
    DiscrValue	 n, k;

    /*  Table of Bell numbers (used for subset test penalties)  */

    Bell = AllocZero(MaxDiscrVal+1, double *);
    ForEach(n, 1, MaxDiscrVal)
    {
	Bell[n] = AllocZero(n+1, double);
	ForEach(k, 1, n)
	{
	    Bell[n][k] = ( k == 1 || k == n ? 1 :
			   Bell[n-1][k-1] + k * Bell[n-1][k] );
	}
    }
}



/*************************************************************************/
/*									 */
/*	Evaluate subsetting a discrete attribute and form the chosen	 */
/*	subsets Subset[Att][], setting Subsets[Att] to the number of	 */
/*	subsets, and the Info[] and Gain[] of a test on the attribute	 */
/*									 */
/*************************************************************************/


void EvalSubset(Attribute Att, CaseCount Cases)
/*   ----------  */
{
    DiscrValue	V1, V2, V3, BestV1, BestV2, InitialBlocks, First=1, Prelim=0;
    ClassNo	c;
    double	BaseInfo, ThisGain, ThisInfo, Penalty, UnknownRate,
		Val, BestVal, BestGain, BestInfo, PrevGain, PrevInfo;
    int		MissingValues=0;
    CaseCount	KnownCases;
    Boolean	Better;

    /*  First compute Freq[][], ValFreq[], base info, and the gain
	and total info of a split on discrete attribute Att  */

    SetDiscrFreq(Att);

    GEnv.ReasonableSubsets = 0;
    ForEach(c, 1, MaxAttVal[Att])
    {
	if ( GEnv.ValFreq[c] >= MINITEMS ) GEnv.ReasonableSubsets++;
    }

    if ( ! GEnv.ReasonableSubsets )
    {
	Verbosity(2,
	    fprintf(Of, "\tAtt %s: poor initial split\n", AttName[Att]))

	return;
    }

    KnownCases  = Cases - GEnv.ValFreq[0];
    UnknownRate = GEnv.ValFreq[0] / Cases;

    BaseInfo = ( ! GEnv.ValFreq[0] ? GlobalBaseInfo :
		     DiscrKnownBaseInfo(KnownCases, MaxAttVal[Att]) );

    PrevGain = ComputeGain(BaseInfo, UnknownRate, MaxAttVal[Att], KnownCases);
    PrevInfo = TotalInfo(GEnv.ValFreq, 0, MaxAttVal[Att]) / Cases;
    BestVal  = PrevGain / PrevInfo;

    Verbosity(2, fprintf(Of, "\tAtt %s", AttName[Att]))
    Verbosity(3, PrintDistribution(Att, 0, MaxAttVal[Att], GEnv.Freq,
				   GEnv.ValFreq, true))
    Verbosity(2,
	fprintf(Of, "\tinitial inf %.3f, gain %.3f, val=%.3f\n",
		PrevInfo, PrevGain, BestVal))

    /*  Eliminate unrepresented attribute values from Freq[] and ValFreq[]
	and form a separate subset for each represented attribute value.
	Unrepresented N/A values are ignored  */

    GEnv.Bytes = (MaxAttVal[Att]>>3) + 1;
    ClearBits(GEnv.Bytes, Subset[Att][0]);

    GEnv.Blocks = 0;
    ForEach(V1, 1, MaxAttVal[Att])
    {
	if ( GEnv.ValFreq[V1] > Epsilon || V1 == 1 && SomeNA[Att] )
	{
	    if ( ++GEnv.Blocks < V1 )
	    {
		GEnv.ValFreq[GEnv.Blocks] = GEnv.ValFreq[V1];
		ForEach(c, 1, MaxClass)
		{
		    GEnv.Freq[GEnv.Blocks][c] = GEnv.Freq[V1][c];
		}
	    }
	    ClearBits(GEnv.Bytes, GEnv.WSubset[GEnv.Blocks]);
	    SetBit(V1, GEnv.WSubset[GEnv.Blocks]);
	    CopyBits(GEnv.Bytes, GEnv.WSubset[GEnv.Blocks],
		     Subset[Att][GEnv.Blocks]);

	    /*  Cannot merge N/A values with other blocks  */

	    if ( V1 == 1 ) First = 2;
	}
	else
	if ( V1 != 1 )
	{
	    SetBit(V1, Subset[Att][0]);
	    MissingValues++;
	}
    }

    /*  Set n-way branch as initial test  */

    Gain[Att]    = PrevGain;
    Info[Att]    = PrevInfo;
    Subsets[Att] = InitialBlocks = GEnv.Blocks;

    /*  As a preliminary step, merge values with identical distributions  */

    ForEach(V1, First, GEnv.Blocks-1)
    {
	ForEach(V2, V1+1, GEnv.Blocks)
	{
	    if ( SameDistribution(V1, V2) )
	    {
		Prelim = V1;
		AddBlock(V1, V2);
	    }
	}

	/*  Eliminate any merged values  */

	if ( Prelim == V1 )
	{
	    V3 = V1;

	    ForEach(V2, V1+1, GEnv.Blocks)
	    {
		if ( GEnv.ValFreq[V2] && ++V3 != V2 )
		{
		    MoveBlock(V3, V2);
		}
	    }

	    GEnv.Blocks = V3;
	}
    }

    if ( Prelim )
    {
	PrevInfo = TotalInfo(GEnv.ValFreq, 0, GEnv.Blocks) / Cases;

	Penalty  = ( finite(Bell[InitialBlocks][GEnv.Blocks]) ?
			Log(Bell[InitialBlocks][GEnv.Blocks]) :
			(InitialBlocks-GEnv.Blocks+1) * Log(GEnv.Blocks) );

	Val = (PrevGain - Penalty / Cases) / PrevInfo;
	Better = ( GEnv.Blocks >= 2 && GEnv.ReasonableSubsets >= 2 &&
		   Val >= BestVal );

	Verbosity(2,
	{
	    fprintf(Of, "\tprelim merges -> inf %.3f, gain %.3f, val %.3f%s%s",
			PrevInfo, PrevGain, Val,
		        ( Better ? " **" : "" ),
			(VERBOSITY > 2 ? "" : "\n" ));
	    Verbosity(3, PrintDistribution(Att, 0, GEnv.Blocks, GEnv.Freq,
					   GEnv.ValFreq, false))
	})

	if ( Better )
	{
	    Subsets[Att] = GEnv.Blocks;

	    ForEach(V1, 1, GEnv.Blocks)
	    {
		CopyBits(GEnv.Bytes, GEnv.WSubset[V1], Subset[Att][V1]);
	    }

	    Info[Att] = PrevInfo;
	    Gain[Att] = PrevGain - Penalty / KnownCases;
	    BestVal   = Val;
	}
    }
		
    /*  Determine initial information and entropy values  */

    ForEach(V1, 1, GEnv.Blocks)
    {
	GEnv.SubsetInfo[V1] = -GEnv.ValFreq[V1] * Log(GEnv.ValFreq[V1] / Cases);
	GEnv.SubsetEntr[V1] = TotalInfo(GEnv.Freq[V1], 1, MaxClass);
    }

    ForEach(V1, First, GEnv.Blocks-1)
    {
	ForEach(V2, V1+1, GEnv.Blocks)
	{
	    EvaluatePair(V1, V2, Cases);
	}
    }

    /*  Examine possible pair mergers and hill-climb  */

    while ( GEnv.Blocks > 2 )
    {
	BestV1 = 0;
	BestGain = -Epsilon;

	/*  For each possible pair of values, calculate the gain and
	    total info of a split in which they are treated as one.
	    Keep track of the pair with the best gain.  */

	ForEach(V1, First, GEnv.Blocks-1)
	{
	    ForEach(V2, V1+1, GEnv.Blocks)
	    {
		if ( GEnv.ReasonableSubsets == 2 &&
		     GEnv.ValFreq[V1] >= MINITEMS-Epsilon &&
		     GEnv.ValFreq[V2] >= MINITEMS-Epsilon )
		{
		    continue;
		}

		ThisGain = PrevGain -
			   ((1-UnknownRate) / KnownCases) *
			     (GEnv.MergeEntr[V1][V2] -
			       (GEnv.SubsetEntr[V1] + GEnv.SubsetEntr[V2]));
		ThisInfo = PrevInfo + (GEnv.MergeInfo[V1][V2] -
			   (GEnv.SubsetInfo[V1] + GEnv.SubsetInfo[V2])) / Cases;
		Verbosity(3,
		    fprintf(Of, "\t    combine %d %d info %.3f gain %.3f\n",
			    V1, V2, ThisInfo, ThisGain))

		/*  See whether this merge has the best gain so far  */

		if ( ThisGain > BestGain+Epsilon )
		{
		    BestGain = ThisGain;
		    BestInfo = ThisInfo;
		    BestV1   = V1;
		    BestV2   = V2;
		}
	    }
	}

	if ( ! BestV1 ) break;

	PrevGain = BestGain;
	PrevInfo = BestInfo;

	/*  Determine penalty as log of Bell number.  If number is too
	    large, use an approximation of log  */

	Penalty  = ( finite(Bell[InitialBlocks][GEnv.Blocks-1]) ?
			Log(Bell[InitialBlocks][GEnv.Blocks-1]) :
			(InitialBlocks-GEnv.Blocks+1) * Log(GEnv.Blocks-1) );

	Val = (BestGain - Penalty / Cases) / BestInfo;

	Merge(BestV1, BestV2, Cases);

	Verbosity(2,
	    fprintf(Of, "\tform subset ");
	    PrintSubset(Att, GEnv.WSubset[BestV1]);
	    fprintf(Of, ": %d subsets, inf %.3f, gain %.3f, val %.3f%s\n",
		   GEnv.Blocks, BestInfo, BestGain, Val,
		   ( Val > BestVal ? " **" : "" ));
	    Verbosity(3,
		PrintDistribution(Att, 0, GEnv.Blocks, GEnv.Freq, GEnv.ValFreq,
				  false))
	    )

	if ( Val >= BestVal )
	{
	    Subsets[Att] = GEnv.Blocks;

	    ForEach(V1, 1, GEnv.Blocks)
	    {
		CopyBits(GEnv.Bytes, GEnv.WSubset[V1], Subset[Att][V1]);
	    }

	    Info[Att] = BestInfo;
	    Gain[Att] = BestGain - Penalty / KnownCases;
	    BestVal   = Val;
	}
    }

    /*  Add missing values as another branch  */

    if ( MissingValues )
    {
	Subsets[Att]++;
	CopyBits(GEnv.Bytes, Subset[Att][0], Subset[Att][Subsets[Att]]);
    }

    Verbosity(2,
	fprintf(Of, "\tfinal inf %.3f, gain %.3f, val=%.3f\n",
		Info[Att], Gain[Att], Gain[Att] / (Info[Att] + 1E-3)))
}



/*************************************************************************/
/*									 */
/*	Combine the distribution figures of subsets x and y.		 */
/*	Update Freq, ValFreq, SubsetInfo, SubsetEntr, MergeInfo, and	 */
/*	MergeEntr.							 */
/*									 */
/*************************************************************************/


void Merge(DiscrValue x, DiscrValue y, CaseCount Cases)
/*   -----  */
{
    ClassNo	c;
    double	Entr=0;
    CaseCount	KnownCases=0;
    int		R, C;

    AddBlock(x, y);

    ForEach(c, 1, MaxClass)
    {
	Entr -= GEnv.Freq[x][c] * Log(GEnv.Freq[x][c]);
	KnownCases += GEnv.Freq[x][c];
    }

    GEnv.SubsetInfo[x] = - GEnv.ValFreq[x] * Log(GEnv.ValFreq[x] / Cases);
    GEnv.SubsetEntr[x] = Entr + KnownCases * Log(KnownCases);

    /*  Eliminate y from working blocks  */

    ForEach(R, y, GEnv.Blocks-1)
    {
	MoveBlock(R, R+1);

	GEnv.SubsetInfo[R] = GEnv.SubsetInfo[R+1];
	GEnv.SubsetEntr[R] = GEnv.SubsetEntr[R+1];

	ForEach(C, 1, GEnv.Blocks)
	{
	    GEnv.MergeInfo[R][C] = GEnv.MergeInfo[R+1][C];
	    GEnv.MergeEntr[R][C] = GEnv.MergeEntr[R+1][C];
	}
    }

    ForEach(C, y, GEnv.Blocks-1)
    {
	ForEach(R, 1, GEnv.Blocks-1)
	{
	    GEnv.MergeInfo[R][C] = GEnv.MergeInfo[R][C+1];
	    GEnv.MergeEntr[R][C] = GEnv.MergeEntr[R][C+1];
	}
    }
    GEnv.Blocks--;

    /*  Update information for newly-merged block  */

    ForEach(C, 1, GEnv.Blocks)
    {
	if ( C != x ) EvaluatePair(x, C, Cases);
    }
}



/*************************************************************************/
/*									 */
/*	Calculate the effect of merging subsets x and y			 */
/*									 */
/*************************************************************************/


void EvaluatePair(DiscrValue x, DiscrValue y, CaseCount Cases)
/*   ------------  */
{
    ClassNo	c;
    double	Entr=0;
    CaseCount	KnownCases=0, F;

    if ( y < x )
    {
	c = y;
	y = x;
	x = c;
    }

    F = GEnv.ValFreq[x] + GEnv.ValFreq[y];
    GEnv.MergeInfo[x][y] = - F * Log(F / Cases);

    ForEach(c, 1, MaxClass)
    {
	F = GEnv.Freq[x][c] + GEnv.Freq[y][c];
	Entr -= F * Log(F);
	KnownCases += F;
    }
    GEnv.MergeEntr[x][y] = Entr + KnownCases * Log(KnownCases);
}



/*************************************************************************/
/*									 */
/*	Check whether two values have same class distribution		 */
/*									 */
/*************************************************************************/


Boolean SameDistribution(DiscrValue V1, DiscrValue V2)
/*	----------------  */
{
    ClassNo	c;
    CaseCount	D1, D2;

    D1 = GEnv.ValFreq[V1];
    D2 = GEnv.ValFreq[V2];

    ForEach(c, 1, MaxClass)
    {
	if ( fabs(GEnv.Freq[V1][c] / D1 - GEnv.Freq[V2][c] / D2) > 0.001 )
	{
	    return false;
	}
    }

    return true;
}



/*************************************************************************/
/*									 */
/*	Add frequency and subset information from block V2 to V1	 */
/*									 */
/*************************************************************************/


void AddBlock(DiscrValue V1, DiscrValue V2)
/*   --------  */
{
    ClassNo	c;
    int		b;

    if ( GEnv.ValFreq[V1] >= MINITEMS-Epsilon &&
	 GEnv.ValFreq[V2] >= MINITEMS-Epsilon )
    {
	GEnv.ReasonableSubsets--;
    }
    else
    if ( GEnv.ValFreq[V1] < MINITEMS-Epsilon &&
	 GEnv.ValFreq[V2] < MINITEMS-Epsilon &&
	 GEnv.ValFreq[V1] + GEnv.ValFreq[V2] >= MINITEMS-Epsilon )
    {
	GEnv.ReasonableSubsets++;
    }

    ForEach(c, 1, MaxClass)
    {
	GEnv.Freq[V1][c] += GEnv.Freq[V2][c];
    }
    GEnv.ValFreq[V1] += GEnv.ValFreq[V2];
    GEnv.ValFreq[V2] = 0;
    ForEach(b, 0, GEnv.Bytes-1)
    {
	GEnv.WSubset[V1][b] |= GEnv.WSubset[V2][b];
    }
}



/*************************************************************************/
/*									 */
/*	Move frequency and subset information from block V2 to V1	 */
/*									 */
/*************************************************************************/


void MoveBlock(DiscrValue V1, DiscrValue V2)
/*   ---------  */
{
    ClassNo	c;

    ForEach(c, 1, MaxClass)
    {
	GEnv.Freq[V1][c] = GEnv.Freq[V2][c];
    }
    GEnv.ValFreq[V1] = GEnv.ValFreq[V2];
    CopyBits(GEnv.Bytes, GEnv.WSubset[V2], GEnv.WSubset[V1]);
}



/*************************************************************************/
/*									 */
/*	Print the values of attribute Att which are in the subset Ss	 */
/*									 */
/*************************************************************************/


void PrintSubset(Attribute Att, Set Ss)
/*   -----------  */
{
    DiscrValue	V1;
    Boolean	First=true;

    ForEach(V1, 1, MaxAttVal[Att])
    {
	if ( In(V1, Ss) )
	{
	    if ( First )
	    {
		First = false;
	    }
	    else
	    {
		fprintf(Of, ", ");
	    }

	    fprintf(Of, "%s", AttValName[Att][V1]);
	}
    }
}



/*************************************************************************/
/*									 */
/*	Construct and return a node for a test on a subset of values	 */
/*									 */
/*************************************************************************/


void SubsetTest(Tree Node, Attribute Att)
/*   -----------  */
{
    int	S, Bytes;

    Sprout(Node, Subsets[Att]);

    Node->NodeType = BrSubset;
    Node->Tested   = Att;

    Bytes = (MaxAttVal[Att]>>3) + 1;
    Node->Subset = AllocZero(Subsets[Att]+1, Set);
    ForEach(S, 1, Node->Forks)
    {
	Node->Subset[S] = Alloc(Bytes, Byte);
	CopyBits(Bytes, Subset[Att][S], Node->Subset[S]);
    }
}
