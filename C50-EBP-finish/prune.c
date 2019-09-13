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
/*	Prune a decision tree and predict its error rate		 */
/*	------------------------------------------------		 */
/*									 */
/*************************************************************************/


#include "defns.i"
#include "extern.i"


#define	  LocalVerbosity(x,s)	if (Sh >= 0) {Verbosity(x,s)}
#define	  Intab(x)		Indent(x, 0)

#define	  UPDATE		1	/* flag: change tree */
#define	  REGROW		2	/*       regrow branches */
#define	  REPORTPROGRESS	4	/*	 original tree */
#define	  UNITWEIGHTS		8	/*	 UnitWeights is true*/

Set		*PossibleValues;

double		MaxExtraErrs,		/* limit for global prune */
		TotalExtraErrs;		/* extra errors from ties */
Tree		*XT;			/* subtrees with lowest cost comp */
int		NXT;			/* number ditto */
float		MinCC;			/* cost compexity for XT */
Boolean		RecalculateErrs;	/* if missing values */




/*************************************************************************/
/*									 */
/*	Prune tree T							 */
/*									 */
/*************************************************************************/


void Prune(Tree T)
/*   -----  */
{
    Attribute	Att;
    int		i, Options;
    Boolean	Regrow;

    Verbosity(2, fprintf(Of, "\n"))

    Regrow = ( Trial == 0 || Now == WINNOWATTS );

    /*  Local pruning phase  */


    Options = ( Now == WINNOWATTS ? (UPDATE|REGROW) :
		Regrow ? (UPDATE|REGROW|REPORTPROGRESS) :
			 (UPDATE|REPORTPROGRESS) );
    if ( UnitWeights ) Options |= UNITWEIGHTS;

    EstimateErrs(T, 0, MaxCase, 0, Options);

    if ( MCost )
    {
	/*  Remove any effects of WeightMul and reset leaf classes  */

	RestoreDistribs(T);
    }
    else
    {
	/*  Insert information on parents and recalculate errors, noting
	    whether fractional cases might have appeared (for GlobalPrune)  */

	RecalculateErrs = false;
	InsertParents(T, Nil);

	/*  Possible global pruning phase  */

	if ( GLOBAL && Now != WINNOWATTS )
	{
	    GlobalPrune(T);
	}
    }

    /*  Remove impossible values from subsets and ordered splits.
	First record possible values for discrete attributes  */

    PossibleValues = AllocZero(MaxAtt+1, Set);
    ForEach(Att, 1, MaxAtt)
    {
	if ( Ordered(Att) || Discrete(Att) && SUBSET )
	{
	    PossibleValues[Att] = AllocZero((MaxAttVal[Att]>>3)+1, Byte);
	    ForEach(i, 1, MaxAttVal[Att])
	    {
		SetBit(i, PossibleValues[Att]);
	    }
	}
    }

    CheckSubsets(T, true);

    FreeVector((void **) PossibleValues, 1, MaxAtt);	PossibleValues = Nil;

    /*  For multibranch splits, merge non-occurring values.  For trees
	(first boosting trial only), also merge leaves of same class  */

    if ( ! SUBSET )
    {
	CompressBranches(T);
    }
}



/*************************************************************************/
/*									 */
/*	Estimate the errors in a given subtree				 */
/*									 */
/*************************************************************************/


void EstimateErrs(Tree T, CaseNo Fp, CaseNo Lp, int Sh, int Flags)
/*   ------------  */
{
    CaseNo	i, Bp, Ep, Missing;
    CaseCount	Cases=0, KnownCases, *BranchCases, MissingCases,
		*SmallBranches, SmallBranchCases=0,
		Errs, SaveErrs, TreeErrs, LeafErrs, ExtraLeafErrs=0, BestBrErrs;
    double	Factor, *LocalClassDist;
    DiscrValue	v, BestBr=0;
    ClassNo	c, BestClass=1;
    int		UnitWeights;			/* local value */
    Tree	Br;
    Attribute	Att;


    if ( Fp > Lp ) return;

    UnitWeights = (Flags & UNITWEIGHTS);

    LocalClassDist = Alloc(MaxClass+1, double);

    FindClassFreq(LocalClassDist, Fp, Lp);

    /*  Record new class distribution if updating the tree  */

    if ( (Flags & UPDATE) )
    {
	ForEach(c, 1, MaxClass)
	{
	    T->ClassDist[c] = LocalClassDist[c];
	    Cases += LocalClassDist[c];

	    if ( LocalClassDist[c] > LocalClassDist[BestClass] ) BestClass = c;
	}
    }
    else
    {
	ForEach(c, 1, MaxClass)
	{
	    Cases += LocalClassDist[c];

	    if ( LocalClassDist[c] > LocalClassDist[BestClass] ) BestClass = c;
	}
    }

    LeafErrs = Cases - LocalClassDist[BestClass];
    ExtraLeafErrs = ExtraErrs(Cases, LeafErrs, BestClass);

    Free(LocalClassDist);

    if ( (Flags & UPDATE) )
    {
	T->Cases = Cases;
	T->Leaf  = BestClass;
    }

    if ( ! T->NodeType )	/*  leaf  */
    {
	if ( (Flags & UPDATE) && (Flags & REPORTPROGRESS) &&
	     Now == SIMPLIFYTREE &&
	     T->Cases > 0 )
	{
	    Progress(T->Cases);
	}

	T->Errors = LeafErrs + ExtraLeafErrs;

	if ( (Flags & UPDATE) )
	{
	    if ( Sh >= 0 )
	    {
		LocalVerbosity(3,
		    Intab(Sh);
		    fprintf(Of, "%s (%.2f:%.2f/%.2f)\n", ClassName[T->Leaf],
			    T->Cases, LeafErrs, T->Errors))
	    }
	}

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

    SmallBranches = AllocZero(MaxClass+1, CaseCount);
    BranchCases   = Alloc(T->Forks+1, CaseCount);

    if ( Missing ) UnitWeights = 0;

    TreeErrs = 0;
    Bp = Fp;

    ForEach(v, 1, T->Forks)
    {
	Ep = Group(v, Bp + Missing, Lp, T);

	/*  Bp -> first value in missing + remaining values
	    Ep -> last value in missing + current group  */

	BranchCases[v] = CountCases(Bp + Missing, Ep);

	Factor = ( ! Missing ? 0 :
		   ! CostWeights ? BranchCases[v] / KnownCases :
		   SumNocostWeights(Bp + Missing, Ep) / KnownCases );

	if ( (BranchCases[v] += Factor * MissingCases) >= MinLeaf )
	{
	    if ( Missing )
	    {
		ForEach(i, Bp, Bp + Missing - 1)
		{
		    Weight(Case[i]) *= Factor;
		}
	    }

	    EstimateErrs(T->Branch[v], Bp, Ep, Sh+1, ((Flags&7) | UnitWeights));

	    /*  Group small branches together for error estimation  */

	    if ( BranchCases[v] < MINITEMS )
	    {
		ForEach(i, Bp, Ep)
		{
		    SmallBranches[ Class(Case[i]) ] += Weight(Case[i]);
		}

		SmallBranchCases += BranchCases[v];
	    }
	    else
	    {
		TreeErrs += T->Branch[v]->Errors;
	    }

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

    /*  Add error estimate from small branches, if any  */

    if ( SmallBranchCases )
    {
	BestClass = 1;
	ForEach(c, 2, MaxClass)
	{
	    if ( SmallBranches[c] > SmallBranches[BestClass] ) BestClass = c;
	}

	Errs = SmallBranchCases - SmallBranches[BestClass];
	TreeErrs += Errs + ExtraErrs(SmallBranchCases, Errs, BestClass);
    }
    Free(SmallBranches);
    Free(BranchCases);

    if ( ! (Flags & UPDATE) )
    {
	T->Errors = TreeErrs;
	return;
    }

    /*  See how the largest candidate branch would perform.  A branch
	is a candidate if it is not a leaf, contains at least 10% of
	the cases, and does not test the same continuous attribute.
	This test is skipped for boosted trees  */

    ForEach(v, 1, T->Forks)
    {
	if ( ! T->Branch[v]->NodeType ||
	     T->Branch[v]->Cases < 0.1 * T->Cases ||
	     T->Branch[v]->Tested == Att && Continuous(Att) )
	{
	    continue;
	}

	if ( ! BestBr || T->Branch[v]->Cases > T->Branch[BestBr]->Cases )
	{
	    BestBr = v;
	}
    }

    if ( BestBr )
    {
	SaveErrs = T->Branch[BestBr]->Errors;
	EstimateErrs(T->Branch[BestBr], Fp, Lp, -1, 0);
	BestBrErrs = T->Branch[BestBr]->Errors;
	T->Branch[BestBr]->Errors = SaveErrs;
    }
    else
    {
	BestBrErrs = MaxCase+1;
    }

    LocalVerbosity(2,
	Intab(Sh);
	fprintf(Of, "%s:  [%d%%  N=%.2f  tree=%.2f  leaf=%.2f+%.2f",
		AttName[T->Tested],
		(int) ((TreeErrs * 100) / (T->Cases + 0.001)),
		T->Cases, TreeErrs, LeafErrs, ExtraLeafErrs);
	if ( BestBr )
	{
	    fprintf(Of, "  br[%d]=%.2f", BestBr, BestBrErrs);
	}
	fprintf(Of, "]\n"))

    /*  See whether tree should be replaced with leaf or best branch  */

    if ( LeafErrs + ExtraLeafErrs <= BestBrErrs + 0.1 &&
	 LeafErrs + ExtraLeafErrs <= TreeErrs + 0.1 )
    {
	LocalVerbosity(2,
	    Intab(Sh);
	    fprintf(Of, "Replaced with leaf %s\n", ClassName[T->Leaf]))

	UnSprout(T);
	T->Errors = LeafErrs + ExtraLeafErrs;
    }
    else
    if ( BestBrErrs <= TreeErrs + 0.1 )
    {
	LocalVerbosity(2,
	    Intab(Sh);
	    fprintf(Of, "Replaced with branch %d\n", BestBr))

	/*  Free unused bits of tree  */

	ForEach(v, 1, T->Forks)
	{
	    if ( v != BestBr ) FreeTree(T->Branch[v]);
	}
	Br = T->Branch[BestBr];
	Free(T->Branch);
	Free(T->ClassDist);
	if ( T->NodeType == BrSubset )
	{
	    FreeVector((void **) T->Subset, 1, T->Forks);
	}

	/*  Copy the branch up  */

	memcpy((char *) T, (char *) Br, sizeof(TreeRec));
	Free(Br);

	Factor = T->Cases / Cases;
	T->Cases = Cases;

	/*  If not within a rebuilt tree, not a cascaded test, and
	    sufficient new cases to justify the effort, rebuild the branch  */

	if ( T->NodeType && (Flags & REGROW) && Factor < 0.95 )
	{
	    ForEach(v, 1, T->Forks)
	    {
		FreeTree(T->Branch[v]);			T->Branch[v] = Nil;
	    }

	    SetGlobalUnitWeights(Flags & UNITWEIGHTS);

	    Divide(T, Fp, Lp, 0);
	}

	EstimateErrs(T, Fp, Lp, Sh, UPDATE);
    }
    else
    {
	T->Errors = TreeErrs;
    }
}



/*************************************************************************/
/*									 */
/*	Phase 2 (global) pruning.					 */
/*	Prune minimum cost complexity subtrees until "error"		 */
/*	(measured by sum of branch errors) increases by 1SE		 */
/*									 */
/*************************************************************************/


void GlobalPrune(Tree T)
/*   -----------  */
{
    int		DeltaLeaves, x;
    double	BaseErrs, DeltaErrs;
    CaseNo	i;
    Tree	ST;

    /*  If fractional cases may have been used, calculate errors
	directly by checking training data  */

    if ( RecalculateErrs )
    {
	BaseErrs = 0;
	ForEach(i, 0, MaxCase)
	{
	    if ( TreeClassify(Case[i], T) != Class(Case[i]) )
	    {
		BaseErrs += Weight(Case[i]);
	    }
	}
    }
    else
    {
	BaseErrs = T->Errors;
    }

    XT = Alloc(T->Leaves, Tree);

    /*  Additional error limit set at 1SE  */

    MaxExtraErrs = sqrt(BaseErrs * (1 - BaseErrs / (MaxCase + 1)));

    while ( MaxExtraErrs > 0 )
    {
	TotalExtraErrs = 0;

	MinCC = 1E38;
	NXT   = 0;

	/*  Find all subtrees with lowest cost complexity  */

	FindMinCC(T);

	Verbosity(2,
	    if ( NXT > 0 && TotalExtraErrs > MaxExtraErrs )
		fprintf(Of, "%d tied with MinCC=%.3f; total extra errs %.1f\n",
			NXT, MinCC, TotalExtraErrs))

	if ( ! NXT || TotalExtraErrs > MaxExtraErrs ) break;

	/*  Make subtree into a leaf  */

	ForEach(x, 0, NXT-1)
	{
	    ST = XT[x];

	    UnSprout(ST);

	    /*  Update errors and leaves for ST and ancestors  */

	    DeltaErrs   = (ST->Cases - ST->ClassDist[ST->Leaf]) - ST->Errors;
	    DeltaLeaves = 1 - ST->Leaves;
	    while ( ST )
	    {
		ST->Errors += DeltaErrs;
		ST->Leaves += DeltaLeaves;
		ST = ST->Parent;
	    }

	    MaxExtraErrs -= DeltaErrs;

	    Verbosity(2,
		fprintf(Of, "global: %d leaves, %.1f errs\n",
			DeltaLeaves, DeltaErrs))
	}
	Verbosity(2, fprintf(Of, "\tremaining=%.1f\n", MaxExtraErrs))
    }

    Free(XT);
}



/*************************************************************************/
/*									 */
/*	Scan tree computing cost complexity of each subtree and		 */
/*	record lowest in global variable XT				 */
/*									 */
/*************************************************************************/


void FindMinCC(Tree T)
/*   ---------  */
{
    DiscrValue	v;
    double	ExtraErrs, CC, SaveMinCC, SaveTotalExtraErrs;
    int		SaveNXT;

    if ( T->NodeType )
    {
	/*  Save current situation  */

	SaveTotalExtraErrs = TotalExtraErrs;
	SaveMinCC          = MinCC;
	SaveNXT		   = NXT;

	/*  Scan subtrees  */

	ForEach(v, 1, T->Forks)
	{
	    if ( T->Branch[v]->Cases > 0.1 )
	    {
		FindMinCC(T->Branch[v]);
	    }
	}
	
	/*  Compute CC for this subtree and check whether minimum  */

	ExtraErrs = (T->Cases - T->ClassDist[T->Leaf]) - T->Errors;

	CC = ExtraErrs / (T->Leaves - 1);

	if ( ExtraErrs <= MaxExtraErrs )
	{
	    /*  Have to be careful of ties in descendants, because
		they would inflate TotalExtraErrs.  Any such ties
		should be discarded  */

	    if ( CC < MinCC ||
		 CC <= MinCC && CC < SaveMinCC /* changed by descendants */ )
	    {
		/*  This is the first of a possible group of ties  */

		MinCC = CC;
		NXT   = 1;
		XT[0] = T;
		TotalExtraErrs = ExtraErrs;
	    }
	    else
	    if ( CC <= MinCC )
	    {
		/*  This is a tie.  Discard any ties among descendants  */

		if ( NXT > SaveNXT )
		{
		    TotalExtraErrs = SaveTotalExtraErrs;
		    NXT		   = SaveNXT;
		}

		XT[NXT++] = T;
		TotalExtraErrs += ExtraErrs;
	    }
	}
    }
}



/*************************************************************************/
/*									 */
/*	Annotate tree with information on parent and leaves		 */
/*									 */
/*************************************************************************/


void InsertParents(Tree T, Tree P)
/*   -------------  */
{
    DiscrValue	v;

    T->Parent = P;
    T->Errors = T->Leaves = 0;

    if ( T->NodeType )
    {
	ForEach(v, 1, T->Forks)
	{
	    InsertParents(T->Branch[v], T);
	    T->Errors += T->Branch[v]->Errors;
	    T->Leaves += T->Branch[v]->Leaves;
	}

	if ( SomeMiss[T->Tested] ) RecalculateErrs = true;
    }
    else
    if ( T->Cases > 1E-3 )
    {
	T->Errors = T->Cases - T->ClassDist[T->Leaf];
	T->Leaves = 1;
    }
}



/*************************************************************************/
/*									 */
/*	Remove unnecessary subset tests on missing values		 */
/*									 */
/*************************************************************************/


void CheckSubsets(Tree T, Boolean PruneDefaults)
/*   ------------  */
{
    Set		HoldValues;
    int		v, vv, x, Bytes, b, First, Any=0;
    Attribute	A;
    Tree	LeafBr;
    ClassNo	c;

    if ( T->NodeType == BrSubset )
    {
	A = T->Tested;

	Bytes = (MaxAttVal[A]>>3) + 1;
	HoldValues = Alloc(Bytes, Byte);

	/*  For non-ordered attributes the last (default) branch contains
	    all values that do not appear in the data.  See whether this
	    branch can be simplified or omitted  */

	if ( ! Ordered(A) && PruneDefaults )
	{
	    ForEach(b, 0, Bytes-1)
	    {
		T->Subset[T->Forks][b] &= PossibleValues[A][b];
		Any |= T->Subset[T->Forks][b];
	    }

	    if ( ! Any )
	    {
		FreeTree(T->Branch[T->Forks]);
		Free(T->Subset[T->Forks]);
		T->Forks--;
	    }
	}

	/*  Process each subtree, leaving only values in branch subset  */

	CopyBits(Bytes, PossibleValues[A], HoldValues);

	ForEach(v, 1, T->Forks)
	{
	    /*  Remove any impossible values from ordered subsets  */

	    if ( Ordered(A) )
	    {
		ForEach(vv, 1, MaxAttVal[A])
		{
		    if ( In(vv, T->Subset[v]) && ! In(vv, HoldValues) )
		    {
			ResetBit(vv, T->Subset[v]);
		    }
		}
	    }

	    CopyBits(Bytes, T->Subset[v], PossibleValues[A]);

	    CheckSubsets(T->Branch[v], PruneDefaults);
	}

	CopyBits(Bytes, HoldValues, PossibleValues[A]);

	Free(HoldValues);

	/*  See whether branches other than N/A can be merged.
	    This cannot be done for ordered attributes since the
	    values in the subset represent an interval  */

	if ( ! Ordered(A) )
	{
	    First = ( In(1, T->Subset[1]) ? 2 : 1 );
	    for ( v = First ; v < T->Forks ; v++ )
	    {
		if ( T->Branch[v]->NodeType ) continue;
		LeafBr = T->Branch[v];

		/*  Consider branches vv that could be merged with branch v  */

		for ( vv = v+1 ; vv <= T->Forks ;  )
		{
		    if ( ! T->Branch[vv]->NodeType &&
			 T->Branch[vv]->Leaf == LeafBr->Leaf &&
			 ( PruneDefaults || T->Branch[vv]->Cases > 0 ) )
		    {
			/*  Branch vv can be merged with branch v  */

			if ( T->Branch[vv]->Cases )
			{
			    /*  Add class distribution from branch vv,
				or replace if branch v has no cases  */

			    ForEach(c, 1, MaxClass)
			    {
				if ( ! LeafBr->Cases )
				{
				    LeafBr->ClassDist[c] =
					T->Branch[vv]->ClassDist[c];
				}
				else
				{
				    LeafBr->ClassDist[c] +=
					T->Branch[vv]->ClassDist[c];
				}
			    }
			    LeafBr->Cases  += T->Branch[vv]->Cases;
			    LeafBr->Errors += T->Branch[vv]->Errors;
			}

			/*  Merge values and free branch vv  */

			ForEach(b, 0, Bytes-1)
			{
			    T->Subset[v][b] |= T->Subset[vv][b];
			}
			FreeTree(T->Branch[vv]);
			Free(T->Subset[vv]);

			T->Forks--;
			ForEach(x, vv, T->Forks)
			{
			    T->Branch[x] = T->Branch[x+1];
			    T->Subset[x] = T->Subset[x+1];
			}
		    }
		    else
		    {
			vv++;
		    }
		}
	    }
	}
    }
    else
    if ( T->NodeType )
    {
	ForEach(v, 1, T->Forks)
	{
	    CheckSubsets(T->Branch[v], PruneDefaults);
	}
    }
}



/*************************************************************************/
/*									 */
/*	Compute Coeff, used by RawExtraErrs() to adjust resubstitution	 */
/*	error rate to upper limit of the confidence level.  Coeff is	 */
/*	the square of the number of standard deviations corresponding	 */
/*	to the selected confidence level.  (Taken from Documenta Geigy	 */
/*	Scientific Tables (Sixth Edition), p185 (with modifications).)	 */
/*									 */
/*************************************************************************/


float Val[] = {  0,  0.001, 0.005, 0.01, 0.05, 0.10, 0.20, 0.40, 1.00},
      Dev[] = {4.0,  3.09,  2.58,  2.33, 1.65, 1.28, 0.84, 0.25, 0.00},
      Coeff;


void InitialiseExtraErrs()
/*   -------------------  */
{
    int i=1;

    /*  Compute and retain the coefficient value, interpolating from
	the values in Val and Dev  */

    while ( CF > Val[i] ) i++;

    Coeff = Dev[i-1] +
	      (Dev[i] - Dev[i-1]) * (CF - Val[i-1]) /(Val[i] - Val[i-1]);
    Coeff = Coeff * Coeff;
    CF = Max(CF, 1E-6);
}


/*************************************************************************/
/*									 */
/*	Calculate extra errors to correct the resubstitution error	 */
/*	rate at a leaf with N cases, E errors, predicted class C.	 */
/*	If CostWeights are used, N and E are normalised by removing	 */
/*	the effects of cost weighting and then reapplying weights to	 */
/*	the result.							 */
/*									 */
/*************************************************************************/


float ExtraErrs(CaseCount N, CaseCount E, ClassNo C)
/*    ---------  */
{
    ClassNo	EC;
    CaseCount	NormC, NormEC;

    if ( ! CostWeights )
    {
	return RawExtraErrs(N, E);
    }

    EC = 3 - C;				/* the other class */
    NormC = (N - E) / WeightMul[C];	/* normalised cases of class C */
    NormEC = E / WeightMul[EC];		/* ditto the other class */

    return WeightMul[EC] * RawExtraErrs(NormC + NormEC, NormEC);
}



float RawExtraErrs(CaseCount N, CaseCount E)
/*    ------------  */
{
    float	Val0, Pr;

    if ( E < 1E-6 )
    {
	return N * (1 - exp(log(CF) / N));
    }
    else
    if ( N > 1 && E < 0.9999 )
    {
	Val0 = N * (1 - exp(log(CF) / N));
	return Val0 + E * (RawExtraErrs(N, 1.0) - Val0);
    }
    else
    if ( E + 0.5 >= N )
    {
	return 0.67 * (N - E);
    }
    else
    {
	Pr = (E + 0.5 + Coeff/2
		+ sqrt(Coeff * ((E + 0.5) * (1 - (E + 0.5)/N) + Coeff/4)) )
	     / (N + Coeff);
	return (N * Pr - E);
    }
}



/*************************************************************************/
/*									 */
/*	If there are differential misclassification costs, the weights	 */
/*	may have been artificially adjusted.  Fix the distributions so	 */
/*	that they represent the "true" (possibly boosted) weights	 */
/*									 */
/*************************************************************************/


void RestoreDistribs(Tree T)
/*   ---------------  */
{
    DiscrValue	v;
    ClassNo	c;

    if ( T->NodeType )
    {
	ForEach(v, 1, T->Forks)
	{
	    RestoreDistribs(T->Branch[v]);
	}
    }

    if ( T->Cases >= MinLeaf )
    {
	if ( CostWeights )
	{
	    T->Cases = 0;
	    ForEach(c, 1, MaxClass)
	    {
		ClassSum[c] = (T->ClassDist[c] /= WeightMul[c]);
		T->Cases += T->ClassDist[c];
	    }
	}
	else
	{
	    ForEach(c, 1, MaxClass)
	    {
		ClassSum[c] = T->ClassDist[c];
	    }
	}

	T->Leaf   = SelectClass(1, true);
	T->Errors = T->Cases - T->ClassDist[T->Leaf];
    }
}



/*************************************************************************/
/*									 */
/*	See whether empty branches can be formed into subsets.		 */
/*	For the first trial only, and when not generating rulesets,	 */
/*	combine leaves with the same class.				 */
/*									 */
/*************************************************************************/


void CompressBranches(Tree T)
/*   ----------------  */
{
    DiscrValue	v, vv, S=0, *LocalSet;
    int		Bytes;
    Tree	Br, *OldBranch;
    ClassNo	c;
    Boolean	EmptyOnly;

    EmptyOnly = Trial || RULES;

    if ( T->NodeType )
    {
	/*  LocalSet[v] is the new branch number to which branch v belongs  */

	LocalSet = AllocZero(T->Forks+1, DiscrValue);

	ForEach(v, 1, T->Forks)
	{
	    Br = T->Branch[v];
	    CompressBranches(Br);

	    /*  Don't check if compression impossible  */

	    if ( v == 1 || T->Forks < 4 || Br->NodeType ||
		 EmptyOnly && Br->Cases >= MinLeaf )
	    {
		vv = v + 1;
	    }
	    else
	    {
		/*  Check whether some previous branch is mergeable.
		    For Trial 0, leaves are mergeable if they are
		    both empty or both non-empty and have the same class;
		    for later trials, they must both be empty  */

		for ( vv = 2 ; vv < v ; vv++ )
		{
		    if ( ! T->Branch[vv]->NodeType &&
			 ( EmptyOnly ? T->Branch[vv]->Cases < MinLeaf :
			     ( T->Branch[vv]->Cases < MinLeaf ) ==
				 ( Br->Cases < MinLeaf ) &&
			     T->Branch[vv]->Leaf == Br->Leaf ) )
		    {
			break;
		    }
		}
	    }

	    /*  If no merge was found, this becomes a new branch  */

	    LocalSet[v] = ( vv < v ? LocalSet[vv] : ++S );
	}

	if ( S < T->Forks )
	{
	    /*  Compress!  */

	    T->Subset   = Alloc(S+1, Set);
	    OldBranch   = T->Branch;
	    T->Branch	= Alloc(S+1, Tree);

	    Bytes = (MaxAttVal[T->Tested]>>3) + 1;
	    S = 0;

	    ForEach(v, 1, T->Forks)
	    {
		if ( LocalSet[v] > S )
		{
		    S++;
		    Br = T->Branch[S] = OldBranch[v];
		    if ( ! Br->ClassDist )
		    {
			Br->ClassDist = AllocZero(MaxClass+1, CaseCount);
		    }
		    T->Subset[S] = AllocZero(Bytes, Byte);

		    /*  Must include N/A even when no cases  -- otherwise
			reader gets the branches muddled  */

		    SetBit(v, T->Subset[S]);

		    ForEach(vv, v+1, T->Forks)
		    {
			if ( LocalSet[vv] == S )
			{
			    SetBit(vv, T->Subset[S]);

			    Br->Cases  += OldBranch[vv]->Cases;
			    Br->Errors += OldBranch[vv]->Errors;
			    ForEach(c, 1, MaxClass)
			    {
				Br->ClassDist[c] += OldBranch[vv]->ClassDist[c];
			    }
			}
		    }
		}
		else
		{
		    FreeTree(OldBranch[v]);
		}
	    }

	    T->NodeType = BrSubset;
	    T->Forks = S;
	    Free(OldBranch);
	}
	Free(LocalSet);
    }
}



void SetGlobalUnitWeights(int LocalFlag)
/*   --------------------  */
{
    UnitWeights = ( LocalFlag != 0 );
}
