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
/*	Soften thresholds for continuous attributes			 */
/*	-------------------------------------------			 */
/*									 */
/*************************************************************************/


#include "defns.i"
#include "extern.i"


/*************************************************************************/
/*									 */
/*	Soften all thresholds for continuous attributes in tree T	 */
/*									 */
/*************************************************************************/


void SoftenThresh(Tree T)
/*   ------------  */
{
    ResubErrs(T, 0, MaxCase);

    FindBounds(T, 0, MaxCase);
}



/*************************************************************************/
/*									 */
/*	Find resubstitution errors for tree T				 */
/*									 */
/*************************************************************************/


void ResubErrs(Tree T, CaseNo Fp, CaseNo Lp)
/*   ---------  */
{
    CaseNo	i, Bp, Ep, Missing;
    CaseCount	Cases=0, KnownCases, BranchCases, MissingCases;
    double	Factor;
    DiscrValue	v;
    Boolean	PrevUnitWeights;
    Attribute	Att;

    if ( ! T->NodeType )
    {
	T->Errors = T->Cases - T->ClassDist[T->Leaf];
	return;
    }

    /*  Estimate errors for each branch  */

    Att = T->Tested;
    Missing = (Ep = Group(0, Fp, Lp, T)) - Fp + 1;

    if ( CostWeights )
    {
	MissingCases = SumNocostWeights(Fp, Ep);
	KnownCases   = SumNocostWeights(Ep+1, Lp);
    }
    else
    {
	MissingCases = CountCases(Fp, Ep);
	KnownCases   = Cases - MissingCases;
    }

    PrevUnitWeights = UnitWeights;
    if ( Missing ) UnitWeights = false;

    T->Errors = 0;
    Bp = Fp;

    ForEach(v, 1, T->Forks)
    {
	Ep = Group(v, Bp + Missing, Lp, T);

	/*  Bp -> first value in missing + remaining values
	    Ep -> last value in missing + current group  */

	BranchCases = CountCases(Bp + Missing, Ep);

	Factor = ( ! Missing ? 0 :
		   ! CostWeights ? BranchCases / KnownCases :
		   SumNocostWeights(Bp + Missing, Ep) / KnownCases );

	if ( BranchCases + Factor * MissingCases >= MinLeaf )
	{
	    if ( Missing )
	    {
		ForEach(i, Bp, Bp + Missing - 1)
		{
		    Weight(Case[i]) *= Factor;
		}
	    }

	    ResubErrs(T->Branch[v], Bp, Ep);

	    T->Errors += T->Branch[v]->Errors;

	    /*  Restore weights if changed  */

	    if ( Missing )
	    {
		for ( i = Ep ; i >= Bp ; i-- )
		{
		    if ( Unknown(Case[i], Att) )
		    {
			Weight(Case[i]) /= Factor;
			Swap(i, Ep);
			Ep--;
		    }
		}
	    }

	    Bp = Ep+1;
	}
    }

    UnitWeights = PrevUnitWeights;
}



/*************************************************************************/
/*								  	 */
/*	Calculate upper and lower bounds for each test on a continuous	 */
/*	attribute in tree T, using cases from Fp to Lp.			 */
/*								  	 */
/*	The lower bound is set so that the error rate of the GT branch	 */
/*	on the cases between the bound and the threshold is double that	 */
/*	of the correct (LE) branch; the upper bound is set similarly.	 */
/*								  	 */
/*************************************************************************/


void FindBounds(Tree T, CaseNo Fp, CaseNo Lp)
/*   --------  */
{
    int		v;
    CaseNo	i, j, Kp, Bp, Ap, Missing, SplitI;
    CaseCount	w, LEErrs, GTErrs, KnownCases, SE;
    ClassNo	RealClass;
    Attribute	Att;
    Boolean	PrevUnitWeights;
    double	Factor;

    /*  Stop when get to a leaf  */

    if ( ! T->NodeType ) return;

    Kp = Group(0, Fp, Lp, T) + 1;
    Missing = Kp - Fp;

    Att = T->Tested;
    KnownCases = CountCases(Kp, Lp);

    /*  Soften a threshold for a continuous attribute  */

    if ( T->NodeType == BrThresh )
    {
	Verbosity(1, fprintf(Of, "\nTest %s <> %g\n", AttName[Att], T->Cut))

	/*  Skip N/A values  */

	Ap = Group(1, Kp, Lp, T) + 1;

	Quicksort(Ap, Lp, Att);

	/*  Locate cut point and overall errors of the LE and GT branches  */

	SplitI = Ap;
	LEErrs = GTErrs = 0;
	ForEach(i, Ap, Lp)
	{
	    if ( CVal(Case[i], Att) <= T->Cut ) SplitI = i;
	}

	T->Mid = (CVal(Case[SplitI], Att) + CVal(Case[SplitI+1], Att)) / 2;

	/*  Consider cutoff points below and above the threshold.
	    The errors on the cases between the cutoff and the threshold
	    are computed for both the LE and GT branches.  The additional
	    errors must be less than 0.5SE and, further, the errors
	    on the "other" branch must not exceed twice the errors
	    on the "real" branch, both after Laplace adjustment  */

	SE = sqrt(T->Errors * (KnownCases - T->Errors) / (KnownCases + 1E-3))
	     * 2;

	LEErrs = GTErrs = 0;
	j = SplitI;
	for ( i = SplitI ; i > Ap ; i-- )
	{
	    RealClass = Class(Case[i]);

	    w = Weight(Case[i]);
	    GTErrs += w * ( TreeClassify(Case[i], T->Branch[3]) != RealClass );
	    LEErrs += w * ( TreeClassify(Case[i], T->Branch[2]) != RealClass );

	    if ( CVal(Case[i-1], Att) < CVal(Case[i], Att) )
	    {
		if ( GTErrs > 2 * LEErrs + 1 || GTErrs - LEErrs > 0.5 * SE )
		{
		    break;
		}

		j = i-1;
	    }
	}
	T->Lower = Min(T->Mid, CVal(Case[j], Att));

	LEErrs = GTErrs = 0;
	j = SplitI+1;
	for ( i = SplitI+1 ; i < Lp ; i++ )
	{
	    RealClass = Class(Case[i]);

	    w = Weight(Case[i]);
	    LEErrs += w * ( TreeClassify(Case[i], T->Branch[2]) != RealClass );
	    GTErrs += w * ( TreeClassify(Case[i], T->Branch[3]) != RealClass );

	    if ( CVal(Case[i], Att) < CVal(Case[i+1], Att) )
	    {
		if ( LEErrs > 2 * GTErrs + 1 || LEErrs - GTErrs > 0.5 * SE )
		{
		    break;
		}

		j = i+1;
	    }
	}
	T->Upper = Max(T->Mid, CVal(Case[j], Att));

	Verbosity(1,
	    fprintf(Of, "\tLower = %g, Upper = %g\n", T->Lower, T->Upper))
    }

    /*  Recursively scan each branch  */

    PrevUnitWeights = UnitWeights;
    if ( Missing > 0 ) UnitWeights = false;

    Bp = Fp;

    ForEach(v, 1, T->Forks)
    {
	Kp = Group(v, Bp + Missing, Lp, T);

	/*  Bp -> first value in missing + remaining values
	    Kp -> last value in missing + current group  */

	if ( Bp + Missing <= Kp &&
	     (Factor = CountCases(Bp + Missing, Kp) / KnownCases) > 1E-6 )
	{
	    if ( Missing )
	    {
		ForEach(i, Bp, Bp + Missing - 1)
		{
		    Weight(Case[i]) *= Factor;
		}
	    }

	    FindBounds(T->Branch[v], Bp, Kp);

	    /*  Restore weights if changed  */

	    if ( Missing )
	    {
		for ( i = Kp ; i >= Bp ; i-- )
		{
		    if ( Unknown(Case[i], Att) )
		    {
			Weight(Case[i]) /= Factor;
			Swap(i, Kp);
			Kp--;
		    }
		}
	    }

	    Bp = Kp+1;
	}
    }

    UnitWeights = PrevUnitWeights;
}
