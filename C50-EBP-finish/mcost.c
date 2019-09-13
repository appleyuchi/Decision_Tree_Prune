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
/*	Read variable misclassification costs				 */
/*	-------------------------------------				 */
/*									 */
/*************************************************************************/


#include "defns.i"
#include "extern.i"


void GetMCosts(FILE *Cf)
/*   ---------  */
{
    ClassNo	Pred, Real, p, r;
    char	Name[1000];
    CaseNo	i;
    float	Val, Sum=0;

    LineNo = 0;

    /*  Read entries from cost file  */

    while ( ReadName(Cf, Name, 1000, ':') )
    {
	if ( ! (Pred = Which(Name, ClassName, 1, MaxClass)) )
	{
	    Error(BADCOSTCLASS, Name, "");
	}

	if ( ! ReadName(Cf, Name, 1000, ':') ||
	     ! (Real = Which(Name, ClassName, 1, MaxClass)) )
	{
	    Error(BADCOSTCLASS, Name, "");
	}

	if ( ! ReadName(Cf, Name, 1000, ':') ||
	     sscanf(Name, "%f", &Val) != 1 || Val < 0 )
	{
	    Error(BADCOST, "", "");
	    Val = 1;
	}

	if ( Pred > 0 && Real > 0 && Pred != Real && Val != 1 )
	{
	    /*  Have a non-trivial cost entry  */

	    if ( ! MCost )
	    {
		/*  Set up cost matrices  */

		MCost = Alloc(MaxClass+1, float *);
		ForEach(p, 1, MaxClass)
		{
		    MCost[p] = Alloc(MaxClass+1, float);
		    ForEach(r, 1, MaxClass)
		    {
			MCost[p][r] = ( p == r ? 0.0 : 1.0 );
		    }
		}
	    }

	    MCost[Pred][Real] = Val;
	}
    }
    fclose(Cf);

    /*  Don't need weights etc. for predict or interpret, or
	if not using cost weighting  */

    if ( ! (CostWeights = MaxClass == 2 && MaxCase >= 0 && MCost) )
    {
	return;
    }

    /*  Determine class frequency distribution  */

    ClassFreq = AllocZero(MaxClass+1, double);

    if ( CWtAtt )
    {
	AvCWt = 1;			/* relative weights not yet set */
	ForEach(i, 0, MaxCase)
	{
	    ClassFreq[Class(Case[i])] += RelCWt(Case[i]);
	}
    }
    else
    {
	ForEach(i, 0, MaxCase)
	{
	    ClassFreq[Class(Case[i])]++;
	}
    }

    /*  Find normalised weight multipliers  */

    WeightMul = Alloc(3, float);

    Sum = (ClassFreq[1] * MCost[2][1] + ClassFreq[2] * MCost[1][2]) /
	  (ClassFreq[1] + ClassFreq[2]);

    WeightMul[1] = MCost[2][1] / Sum;
    WeightMul[2] = MCost[1][2] / Sum;

    /*  Adjust MINITEMS to take account of case reweighting  */

    MINITEMS *= Min(WeightMul[1], WeightMul[2]);

    Free(ClassFreq);					ClassFreq = Nil;
}
