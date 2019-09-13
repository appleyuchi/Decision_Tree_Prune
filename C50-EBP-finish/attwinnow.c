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
/*	Routines for winnowing attributes				 */
/*	---------------------------------				 */
/*									 */
/*************************************************************************/


#include "defns.i"
#include "extern.i"

float		*AttImp=Nil;		/* att importance */
Boolean		*Split=Nil,		/* atts used in unpruned tree */
		*Used=Nil;		/* atts used in pruned tree */


/*************************************************************************/
/*									 */
/*	Winnow attributes by constructing a tree from half the data.	 */
/*	Remove those that are never used as splits and those that	 */
/*	increase error on the remaining data, and check that the new	 */
/*	error cost does not increase					 */
/*									 */
/*************************************************************************/


void WinnowAtts()
/*   ----------  */
{
    Attribute	Att, Removed=0, Best;
    CaseNo	i, Bp, Ep;
    float	Base;
    Boolean	First=true, *Upper;
    ClassNo	c;
    extern Attribute	*DList;
    extern int		NDList;

    /*  Save original case order  */

    SaveCase = Alloc(MaxCase+1, DataRec);
    ForEach(i, 0, MaxCase)
    {
	SaveCase[i] = Case[i];
    }

    /*  Split data into two halves with equal class frequencies  */

    Upper = AllocZero(MaxClass+1, Boolean);

    Bp = 0;
    Ep = MaxCase;
    ForEach(i, 0, MaxCase)
    {
	c = Class(SaveCase[i]);

	if ( Upper[c] )
	{
	    Case[Ep--] = SaveCase[i];
	}
	else
	{
	    Case[Bp++] = SaveCase[i];
	}

	Upper[c] = ! Upper[c];
    }

    Free(Upper);

    /*  Use first 50% of the cases for building a winnowing tree
	and remaining 50% for measuring attribute importance  */

    AttImp = AllocZero(MaxAtt+1, float);
    Split  = AllocZero(MaxAtt+1, Boolean);
    Used   = AllocZero(MaxAtt+1, Boolean);

    Base = TrialTreeCost(true);

    /*  Remove attributes when doing so would reduce error cost  */

    ForEach(Att, 1, MaxAtt)
    {
	if ( AttImp[Att] < 0 )
	{
	    SpecialStatus[Att] ^= SKIP;
	    Removed++;
	}
    }

    /*  If any removed, rebuild tree and reinstate if error increases  */

    if ( Removed && TrialTreeCost(false) > Base )
    {
	ForEach(Att, 1, MaxAtt)
	{
	    if ( AttImp[Att] < 0 )
	    {
		AttImp[Att] = 1;
		SpecialStatus[Att] ^= SKIP;
		Verbosity(1, fprintf(Of, "  re-including %s\n", AttName[Att]))
	    }
	}

	Removed=0;
    }

    /*  Discard unused attributes  */

    ForEach(Att, 1, MaxAtt)
    {
	if ( Att != ClassAtt && ! Skip(Att) && ! Split[Att] )
	{
	    SpecialStatus[Att] ^= SKIP;
	    Removed++;
	}
    }

    /*  Print summary of winnowing  */

    if ( ! Removed )
    {
	fprintf(Of, T_NoWinnow);
    }
    else
    {
	fprintf(Of, T_AttributesWinnowed, Removed, Plural(Removed));

	/*  Print remaining attributes ordered by importance  */

	while ( true )
	{
	    Best = 0;
	    ForEach(Att, 1, MaxAtt)
	    {
		if ( AttImp[Att] >= 1 &&
		     ( ! Best || AttImp[Att] > AttImp[Best] ) )
		{
		    Best = Att;
		}
	    }
	    if ( ! Best ) break;

	    if ( First )
	    {
		fprintf(Of, T_EstImportance);
		First = false;
	    }
	    if ( AttImp[Best] >= 1.005 )
	    {
		fprintf(Of, "%7d%%  %s\n",
			    (int) ((AttImp[Best] - 1) * 100 + 0.5),
			    AttName[Best]);
	    }
	    else
	    {
		fprintf(Of, "     <1%%  %s\n", AttName[Best]);
	    }
	    AttImp[Best] = 0;
	}
    }

    if ( Removed )
    {
	Winnowed = true;

	/*  Reset DList  */

	NDList = 0;
	ForEach(Att, 1, MaxAtt)
	{
	    if ( DFreq[Att] && ! Skip(Att) )
	    {
		DList[NDList++] = Att;
	    }
	}
    }

    /*  Restore case order and clean up  */

    ForEach(i, 0, MaxCase)
    {
	Case[i] = SaveCase[i];
    }

    FreeUnlessNil(SaveCase);				SaveCase = Nil;
    FreeUnlessNil(AttImp);				AttImp = Nil;
    FreeUnlessNil(Split);				Split = Nil;
    FreeUnlessNil(Used);				Used = Nil;

    Now = 0;
}



/*************************************************************************/
/*									 */
/*	Build trial tree and check error cost on remaining data.	 */
/*	If first time, note split attributes and check effect of	 */
/*	removing every attribute					 */
/*									 */
/*************************************************************************/


float TrialTreeCost(Boolean FirstTime)
/*    -------------  */
{
    Attribute	Att;
    float	Base, Cost, SaveMINITEMS;
    CaseNo	SaveMaxCase, Cut;
    int		SaveVERBOSITY;

    Verbosity(1,
	fprintf(Of, ( FirstTime ? "\nWinnow cycle:\n" : "\nCheck:\n" )))

    /*  Build and prune trial tree  */

    SaveMaxCase   = MaxCase;
    SaveVERBOSITY = VERBOSITY;
    SaveMINITEMS  = MINITEMS;
    MINITEMS      = Max(MINITEMS / 2, 2.0);

    Cut = (MaxCase+1) / 2 - 1;

    InitialiseWeights();
    LEAFRATIO = 0;
    VERBOSITY = 0;
    MaxCase   = Cut;

    memset(Tested, 0, MaxAtt+1);		/* reset tested attributes */

    SetMinGainThresh();
    FormTree(0, Cut, 0, &WTree);

    if ( FirstTime )
    {
	/*  Find attributes used in unpruned tree  */

	ScanTree(WTree, Split);
    }

    Prune(WTree);

    VERBOSITY = SaveVERBOSITY;
    MaxCase   = SaveMaxCase;
    MINITEMS  = SaveMINITEMS;

    Verbosity(2,
	PrintTree(WTree, "Winnowing tree:");
	fprintf(Of, "\n  training error cost %g\n", ErrCost(WTree, 0, Cut)))

    Base = ErrCost(WTree, Cut+1, MaxCase);

    Verbosity(1,
	fprintf(Of, "  initial error cost %g\n", Base))

    if ( FirstTime )
    {
	/*  Check each attribute used in pruned tree  */

	ScanTree(WTree, Used);

	ForEach(Att, 1, MaxAtt)
	{

	    if ( ! Used[Att] )
	    {
		Verbosity(1,
		    if ( Att != ClassAtt && ! Skip(Att) )
		    {
			fprintf(Of, "  %s not used\n", AttName[Att]);
		    })

		if ( Split[Att] )
		{
		    AttImp[Att] = 1;
		}

		continue;
	    }

	    /*  Determine error cost if this attribute omitted  */

	    SpecialStatus[Att] ^= SKIP;

	    Cost = ErrCost(WTree, Cut+1, MaxCase);

	    AttImp[Att] = ( Cost < Base ? -1 : Cost / Base );
	    Verbosity(1,
		fprintf(Of, "  error cost without %s = %g%s\n",
			    AttName[Att], Cost,
			    ( Cost < Base ? " - excluded" : "" )))

	    SpecialStatus[Att] ^= SKIP;
	}
    }

    if ( WTree )
    {
	FreeTree(WTree);				WTree = Nil;
    }

    return Base;
}



/*************************************************************************/
/*									 */
/*	Determine the error rate or cost of T on cases Fp through Lp	 */
/*									 */
/*************************************************************************/


float ErrCost(Tree T, CaseNo Fp, CaseNo Lp)
/*    -------  */
{
    CaseNo	i;
    float	ErrCost=0;
    ClassNo	Pred;

    if ( MCost )
    {
	ForEach(i, Fp, Lp)
	{
	    if ( (Pred = TreeClassify(Case[i], T)) != Class(Case[i]) )
	    {
		ErrCost += MCost[Pred][Class(Case[i])];
	    }
	}
    }
    else
    {
	ForEach(i, Fp, Lp)
	{
	    if ( TreeClassify(Case[i], T) != Class(Case[i]) )
	    {
		ErrCost += 1.0;
	    }
	}
    }

    return ErrCost;
}



/*************************************************************************/
/*									 */
/*	Find attributes used in tree T					 */
/*									 */
/*************************************************************************/


void ScanTree(Tree T, Boolean *Used)
/*   --------  */
{
    DiscrValue	v;

    if ( T->NodeType )
    {
	Used[T->Tested] = true;

	ForEach(v, 1, T->Forks)
	{
	    ScanTree(T->Branch[v], Used);
	}
    }
}
