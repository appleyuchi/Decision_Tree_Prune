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
/*	Carry out crossvalidation trials				 */
/*	--------------------------------				 */
/*									 */
/*************************************************************************/

#include "defns.i"
#include "extern.i"


DataRec	*Blocked=Nil;
float	**Result=Nil;	/* Result[f][0] = tree/ruleset size
				    [1] = tree/ruleset errors
				    [2] = tree/ruleset cost  */



/*************************************************************************/
/*									 */
/*	Outer function (differs from xval script)			 */
/*									 */
/*************************************************************************/


void CrossVal()
/*   --------  */
{
    CaseNo	i, Size, Start=0, Next, SaveMaxCase;
    int		f, SmallTestBlocks, t, SaveTRIALS;
    ClassNo	c;
    static CaseNo *ConfusionMat=Nil;
    static int    SaveFOLDS=0;

    /*  Check for left-overs after interrupt  */

    if ( Result )
    {
	FreeVector((void **) Result, 0, SaveFOLDS-1);
	Free(ConfusionMat);
    }

    if ( FOLDS > MaxCase+1 )
    {
	fprintf(Of, T_FoldsReduced);
	FOLDS = MaxCase+1;
    }

    Result	 = AllocZero((SaveFOLDS = FOLDS), float *);
    Blocked	 = Alloc(MaxCase+1, DataRec);
    ConfusionMat = AllocZero((MaxClass+1)*(MaxClass+1), CaseNo);

    Prepare();

    SaveMaxCase = MaxCase;
    SaveTRIALS  = TRIALS;

    /*  First test blocks may be smaller than the others  */

    SmallTestBlocks = FOLDS - ((MaxCase+1) % FOLDS);
    Size = (MaxCase + 1) / FOLDS;

    ForEach(f, 0, FOLDS-1)
    {
	fprintf(Of, "\n\n[ " T_Fold " %d ]\n", f+1);
	Result[f] = AllocZero(3, float);

	if ( f == SmallTestBlocks ) Size++;
	MaxCase = SaveMaxCase - Size;

	ForEach(i, 0, MaxCase)
	{
	    Case[i] = Blocked[Start];
	    Start = (Start + 1) % (SaveMaxCase + 1);
	}

	ConstructClassifiers();

	/*  Check size (if appropriate) and errors  */

	if ( TRIALS == 1 )
	{
	    Result[f][0] = ( RULES ? RuleSet[0]->SNRules :
				     TreeSize(Pruned[0]) );
	    Next = Start;
	    ForEach(i, 0, Size-1)
	    {
		Case[i] = Blocked[Next];
		c = ( RULES ? RuleClassify(Blocked[Next], RuleSet[0]) :
			      TreeClassify(Blocked[Next], Pruned[0]) );
		if ( c != Class(Blocked[Next]) )
		{
		    Result[f][1] += 1.0;
		    if ( MCost )
		    {
			Result[f][2] += MCost[c][Class(Blocked[Next])];
		    }
		}

		/*  Add to confusion matrix for target classifier  */

		ConfusionMat[ Class(Blocked[Next])*(MaxClass+1)+c ]++;

		Next = (Next + 1) % (SaveMaxCase + 1);
	    }
	}
	else
	{
	    Result[f][0] = -1;
	    Next = Start;
	    Default = ( RULES ? RuleSet[0]->SDefault : Pruned[0]->Leaf );
	    ForEach(i, 0, Size-1)
	    {
		Case[i] = Blocked[Next];
		c = BoostClassify(Blocked[Next], TRIALS-1);
		if ( c != Class(Blocked[Next]) )
		{
		    Result[f][1] += 1.0;
		    if ( MCost )
		    {
			Result[f][2] += MCost[c][Class(Blocked[Next])];
		    }
		}

		/*  Add to confusion matrix for target classifier  */

		ConfusionMat[ Class(Blocked[Next])*(MaxClass+1)+c ]++;

		Next = (Next + 1) % (SaveMaxCase + 1);
	    }
	}

	Result[f][1] = (100.0 * Result[f][1]) / Size;
	Result[f][2] /= Size;

	fprintf(Of, T_EvalHoldOut, Size);
	MaxCase = Size-1;
	Evaluate(0);

	/*  Free space used by classifiers  */

	ForEach(t, 0, MaxTree)
	{
	    FreeClassifier(t);
	}
	MaxTree = -1;

	TRIALS = SaveTRIALS;
    }

    /*  Print summary of crossvalidation  */

    MaxCase = SaveMaxCase;

    Summary();
    PrintConfusionMatrix(ConfusionMat);

    /*  Free local storage  */

    ForEach(i, 0, MaxCase)
    {
	Case[i] = Blocked[i];
    }

    FreeVector((void **) Result, 0, FOLDS-1);		Result = Nil;
    Free(Blocked);					Blocked = Nil;
    Free(ConfusionMat);					ConfusionMat = Nil;
}



/*************************************************************************/
/*                                                                       */
/*      Prepare data for crossvalidation (similar to xval-prep.c)	 */
/*                                                                       */
/*************************************************************************/


void Prepare()
/*   -------  */
{
    CaseNo	i, First=0, Last, *Temp, Hold, Next=0;
    ClassNo	Group;

    Temp = Alloc(MaxCase+1, CaseNo);
    ForEach(i, 0, MaxCase)
    {
	Temp[i] = i;
    }

    Shuffle(Temp);

    /*  Sort into class groups  */

    while ( First <= MaxCase )
    {
	Last = First;
	Group = Class(Case[Temp[First]]);

	ForEach(i, First+1, MaxCase)
	{
	    if ( Class(Case[Temp[i]]) == Group )
	    {
		Last++;
		Hold = Temp[Last];
		Temp[Last] = Temp[i];
		Temp[i] = Hold;
	    }
	}

	First = Last+1;
    }

    /*  Organize into stratified blocks  */

    ForEach(First, 0, FOLDS-1)
    {
	for ( i = First ; i <= MaxCase ; i += FOLDS )
	{
	    Blocked[Next++] = Case[Temp[i]];
	}
    }

    Free(Temp);
}



/*************************************************************************/
/*                                                                       */
/*      Shuffle the data cases                                           */
/*                                                                       */
/*************************************************************************/


void Shuffle(int *Vec)
/*   -------  */
{
    int	This=0, Alt, Left=MaxCase+1, Hold;

    ResetKR(KRInit);

    while ( Left )
    {
	Alt = This + (Left--) * KRandom();

	Hold 	    = Vec[This];
	Vec[This++] = Vec[Alt];
	Vec[Alt]    = Hold;
    }
}



/*************************************************************************/
/*									 */
/*	Summarise a crossvalidation					 */
/*									 */
/*************************************************************************/


char
     *FoldHead[] = { F_Fold, F_UFold, "" };

void Summary()
/*   -------  */
{
    int		i, f, t;
    Boolean	PrintSize=true;
    float	Sum[3], SumSq[3];
    extern char	*StdP[], *StdPC[], *Extra[], *ExtraC[];

    for ( i = 0 ; i < 3 ; i++ )
    {
	Sum[i] = SumSq[i] = 0;
    }

    ForEach(f, 0, FOLDS-1)
    {
	if ( Result[f][0] < 1 ) PrintSize = false;
    }

    fprintf(Of, "\n\n[ " T_Summary " ]\n\n");

    ForEach(t, 0, 2)
    {
	fprintf(Of, "%s", FoldHead[t]);
	putc('\t', Of);
	if ( RULES )
	{
	    fprintf(Of, "%s", ( MCost ? ExtraC[t] : Extra[t] ));
	}
	else
	{
	    fprintf(Of, "%s", ( MCost ? StdPC[t] : StdP[t] ));
	}
	putc('\n', Of);
    }
    putc('\n', Of);

    ForEach(f, 0, FOLDS-1)
    {
	fprintf(Of, "%4d\t", f+1);

	if ( PrintSize )
	{
	    fprintf(Of, " %5g", Result[f][0]);
	}
	else
	{
	    fprintf(Of, "     *");
	}
	fprintf(Of, " %10.1f%%", Result[f][1]);

	if ( MCost )
	{
	    fprintf(Of, "%7.2f", Result[f][2]);
	}
	fprintf(Of, "\n");

	for ( i = 0 ; i < 3 ; i++ )
	{
	    Sum[i] += Result[f][i];
	    SumSq[i] += Result[f][i] * Result[f][i];
	}
    }

    fprintf(Of, "\n  " T_Mean "\t");

    if ( ! PrintSize )
    {
	fprintf(Of, "      ");
    }
    else
    {
	fprintf(Of, "%6.1f", Sum[0] / FOLDS);
    }

    fprintf(Of, " %10.1f%%", Sum[1] / FOLDS);

    if ( MCost )
    {
	fprintf(Of, "%7.2f", Sum[2] / FOLDS);
    }

    fprintf(Of, "\n  " T_SE "\t");

    if ( ! PrintSize )
    {
	fprintf(Of, "      ");
    }
    else
    {
	fprintf(Of, "%6.1f", SE(Sum[0], SumSq[0], FOLDS));
    }

    fprintf(Of, " %10.1f%%", SE(Sum[1], SumSq[1], FOLDS));

    if ( MCost )
    {
	fprintf(Of, "%7.2f", SE(Sum[2], SumSq[2], FOLDS));
    }
    fprintf(Of, "\n");
}



float SE(float sum, float sumsq, int no)
/*    --  */
{
    float mean;

    mean = sum / no;

    return sqrt( ((sumsq - no * mean * mean) / (no - 1)) / no );
}
