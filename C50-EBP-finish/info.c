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
/*	Calculate information, information gain, and print dists	 */
/*	--------------------------------------------------------	 */
/*									 */
/*************************************************************************/


#include "defns.i"
#include "extern.i"


/*************************************************************************/
/*									 */
/*	Given Freq[][] and ValFreq[], compute the information gain.	 */
/*									 */
/*************************************************************************/


double ComputeGain(double BaseInfo, float UnknFrac, DiscrValue MaxVal,
		   CaseCount TotalCases)
/*     -----------  */
{
    DiscrValue	v;
    double	ThisInfo=0.0;

    /*  Check whether all values are unknown or the same  */

    if ( ! TotalCases ) return None;

    /*  Compute total info after split, by summing the
	info of each of the subsets formed by the test  */

    ForEach(v, 1, MaxVal)
    {
	ThisInfo += TotalInfo(GEnv.Freq[v], 1, MaxClass);
    }
    ThisInfo /= TotalCases;

    /*  Set the gain in information for all cases, adjusted for unknowns  */

    return ( BaseInfo <= ThisInfo ? 0.0 :
	     (1 - UnknFrac) * (BaseInfo - ThisInfo) );

}



/*************************************************************************/
/*									 */
/*	Compute the total information in V[ MinVal..MaxVal ]		 */
/*									 */
/*************************************************************************/


double TotalInfo(double V[], DiscrValue MinVal, DiscrValue MaxVal)
/*     ---------  */
{
    DiscrValue	v;
    double	Sum=0.0, TotalCases=0;
    CaseCount	N;

    ForEach(v, MinVal, MaxVal)
    {
	N = V[v];

	Sum += N * Log(N);
	TotalCases += N;
    }

    return TotalCases * Log(TotalCases) - Sum;
}



/*************************************************************************/
/*									 */
/*	Print distribution table for given attribute			 */
/*									 */
/*************************************************************************/


void PrintDistribution(Attribute Att, DiscrValue MinVal, DiscrValue MaxVal,
		       double **Freq, double *ValFreq, Boolean ShowNames)
/*   -----------------  */
{
    DiscrValue v;
    ClassNo c;
    String Val;

    fprintf(Of, "\n\t\t\t ");
    ForEach(c, 1, MaxClass)
    {
	fprintf(Of, "%7.6s", ClassName[c]);
    }
    fprintf(Of, "\n");

    ForEach(v, MinVal, MaxVal)
    {
	if ( ShowNames )
	{
	    Val = ( ! v ? "unknown" :
		    MaxAttVal[Att] ? AttValName[Att][v] :
		    v == 1 ? "N/A" :
		    v == 2 ? "below" : "above" );
	    fprintf(Of, "\t\t[%-7.7s:", Val);
	}
	else
	{
	    fprintf(Of, "\t\t[%-7d:", v);
	}

	ForEach(c, 1, MaxClass)
	{
	    fprintf(Of, " %6.1f", Freq[v][c]);
	}

	fprintf(Of, "]\n");
    }
}
