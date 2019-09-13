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
/*	Manage construction of classifiers, including boosting		 */
/*	------------------------------------------------------		 */
/*									 */
/*	C5.0 uses a modified form of boosting as follows:		 */
/*	*  Multiplicative weight adjustment is used for cases that are	 */
/*	   classified correctly, but misclassified cases use a form	 */
/*	   of additive weight adjustment.				 */
/*	*  In later boosting trials, cases that cannot possibly be	 */
/*	   classified correctly are dropped.  (This follows Freund's	 */
/*	   "Brown-Boost" approach, since the number of trials is known.) */
/*	*  The voting weight of a boosted classifier is determined by	 */
/*	   the confidence of its classification, based on the boosted	 */
/*	   weights of the training cases.				 */
/*									 */
/*	Variable misclassification costs are also supported.  When	 */
/*	there are two classes, the misclassification costs are used	 */
/*	to reweight the training cases, and the reweighting is reversed	 */
/*	after the classifier is constructed.  When classifying a case,	 */
/*	the estimated class probabilities and cost matrix are used to	 */
/*	determine the predicted class with lowest expected cost.	 */
/*									 */
/*************************************************************************/


#include "defns.i"
#include "extern.i"

/*************************************************************************/
/*									 */
/*	Grow single tree or sequence of boosted trees			 */
/*									 */
/*************************************************************************/


void ConstructClassifiers()
/*   --------------------  */
{
    CaseNo	i, Errs, Cases, Bp, Excl=0;
    double	ErrWt, ExclWt=0, OKWt, ExtraErrWt, NFact, MinWt=1.0, a, b;
    ClassNo	c, Pred, Real, Best;
    static	ClassNo	*Wrong=Nil;
    int		BaseLeaves;
    Boolean	NoStructure, CheckExcl;
    float	*BVote;

    /*  Clean up after possible interrupt  */

    FreeUnlessNil(Wrong);

    Wrong = Alloc(MaxCase+1, ClassNo);

    if ( TRIALS > 1 )
    {
	/*  BVoteBlock contains each case's class votes  */

	BVoteBlock = AllocZero((MaxCase+1) * (MaxClass+1), float);
    }

    /*  Preserve original case order  */

    SaveCase = Alloc(MaxCase+1, DataRec);
    memcpy(SaveCase, Case, (MaxCase+1) * sizeof(DataRec));

    /*  If using case weighting, find average  */

    if ( CWtAtt )
    {
	SetAvCWt();
    }

    InitialiseWeights();

    /*  Adjust minimum weight if using cost weighting  */

    if ( CostWeights )
    {
	ForEach(c, 1, MaxClass)
	{
	    if ( WeightMul[c] < MinWt ) MinWt = WeightMul[c];
	}
    }

    LEAFRATIO = Bp = 0;
    SetMinGainThresh();

    /*  Main loop for growing the sequence of boosted classifiers  */

    ForEach(Trial, 0, TRIALS-1 )
    {
	if ( TRIALS > 1 )
	{
	    fprintf(Of, "\n-----  " F_Trial " %d:  -----\n", Trial);
	}

	NotifyStage(FORMTREE);
	Progress(-(MaxCase+1.0));

	/*  Update count here in case tree construction is interrupted  */

	MaxTree = Trial;
	Raw[MaxTree] = Pruned[MaxTree] = Nil;
	if ( RULES ) RuleSet[MaxTree] = Nil;

	memset(Tested, 0, MaxAtt+1);		/* reset tested attributes */

	FormTree(Bp, MaxCase, 0, &Raw[Trial]);

	/*  Prune the raw tree to minimise expected misclassification cost  */

	Verbosity(1, if ( ! RULES ) PrintTree(Raw[Trial], "Before pruning:"))

	NotifyStage(SIMPLIFYTREE);
	Progress(-(MaxCase+1));

	/*  If still need raw tree, copy it; otherwise set initial
	    pruned tree to raw tree  */

	if ( VERBOSITY && ! RULES )
	{
	    Pruned[Trial] = CopyTree(Raw[Trial]);
	    if ( MCost )
	    {
		RestoreDistribs(Raw[Trial]);
	    }
	}
	else
	{
	    Pruned[Trial] = Raw[Trial];
	    Raw[Trial] = Nil;
	}

	memcpy(Case, SaveCase, (MaxCase+1) * sizeof(DataRec)); /* restore */

	Prune(Pruned[Trial]);

	AdjustAllThresholds(Pruned[Trial]);

	/*  Record tree parameters for later  */

	if ( ! Trial )
	{
	    BaseLeaves = ( RULES || SUBSET ? TreeSize(Pruned[0]) :
					     ExpandedLeafCount(Pruned[0]) );
	}
	NoStructure = ! Pruned[Trial]->NodeType;

	if ( PROBTHRESH )
	{
	    SoftenThresh(Pruned[Trial]);
	}

	memcpy(Case, SaveCase, (MaxCase+1) * sizeof(DataRec)); /* restore */

	if ( RULES )
	{
	    RuleSet[Trial] = FormRules(Pruned[Trial]);
	    NoStructure |= ! RuleSet[Trial]->SNRules;

	    PrintRules(RuleSet[Trial], T_Rules);
	    fprintf(Of, "\n" T_Default_class ": %s\n",
			ClassName[RuleSet[Trial]->SDefault]);

	    FreeTree(Pruned[Trial]);			Pruned[Trial] = Nil;
	}
	else
	{
	    PrintTree(Pruned[Trial], T_Tree);
	}

	if ( Trial == TRIALS-1 ) continue;

	/*  Check errors, adjust boost voting, and shift dropped cases
	    to the front  */

	ErrWt = Errs = OKWt = Bp = 0;
	CheckExcl = ( Trial+1 > TRIALS / 2.0 );

	ForEach(i, 0, MaxCase)
	{
	    /*  Has this case been dropped already?  */

	    if ( Weight(Case[i]) <= 0 )
	    {
		Case[i]  = Case[Bp];
		Wrong[i] = Wrong[Bp];
		Bp++;
		continue;
	    }

	    Pred = ( RULES ? RuleClassify(Case[i], RuleSet[Trial]) :
			     TreeClassify(Case[i], Pruned[Trial]) );

	    Real = Class(Case[i]);

	    /*  Update boosting votes for this case.  (Note that cases
		must have been reset to their original order.)  */

	    BVote = BVoteBlock + i * (MaxClass+1);
	    BVote[Pred] += Confidence;

	    Best = BVote[0];
	    if ( BVote[Pred] > BVote[Best] ) BVote[0] = Best = Pred;

	    if ( CheckExcl )
	    {
		/*  Check whether this case should be dropped because
		    the vote for the correct class cannot be increased
		    sufficiently in the remaining trials  */

		if ( BVote[Best] > BVote[Real] + (TRIALS-1) - Trial )
		{
		    Excl++;
		    ExclWt += Weight(Case[i]);

		    Weight(Case[i]) = 0;
		    Case[i]  = Case[Bp];
		    Wrong[i] = Wrong[Bp];
		    Bp++;

		    continue;
		}
	    }

	    if ( Pred != Real )
	    {
		Wrong[i] = Pred;
		ErrWt   += Weight(Case[i]);
		Errs++;
	    }
	    else
	    {
		Wrong[i] = 0;
		OKWt    += Weight(Case[i]);
	    }
	}

	Cases  = (MaxCase+1) - Excl;

	/*  Special termination conditions  */

	if ( ErrWt < 0.1 )
	{
	    TRIALS = Trial + 1;
	    fprintf(Of, TX_Reduced1(TRIALS), TRIALS);
	}
	else
	if ( Trial && NoStructure || ErrWt / Cases >= 0.49 )
	{
	    TRIALS = ( Trial ? Trial : 1 );
	    fprintf(Of, TX_Reduced2(TRIALS), TRIALS);
	}
	else
	{
	    /*  Adjust case weights.  Total weight of misclassified cases
		set to midway between current weight and half total weight.
		Take account of any dropped cases  */

	    ExtraErrWt = 0.25 * (OKWt - ErrWt);		/* half */
	    a = (OKWt - ExtraErrWt) / OKWt;
	    b = ExtraErrWt / Errs;

	    /*  Normalise so that sum of weights equals number of cases  */

	    NFact = Cases / (OKWt + ErrWt);

	    MinWt *= a * NFact;

	    ForEach(i, Bp, MaxCase)
	    {
		if ( Wrong[i] )
		{
		    Weight(Case[i]) = NFact * (Weight(Case[i]) + b);
		}
		else
		{
		    Weight(Case[i]) *= NFact * a;

		    /*  Necessary for accumulated arithmetic errors  */

		    if ( Weight(Case[i]) < 1E-3 ) Weight(Case[i]) = 1E-3;
		}
	    }

	    /*  Set the leaf ratio for subsequent boosting trials.
		The idea is to constrain the tree to roughly the size
		of the initial tree by limiting the number of leaves
		per training case.  This limitation is not strict
		since even a tiny number of cases can give a leaf  */

	    if ( Trial == 0 )
	    {
		LEAFRATIO = 1.1 * BaseLeaves / (MaxCase + 1.0);
	    }

	    /*  Trim cases for larger datasets  */

	    if ( MaxCase > 4000 && MinWt <= 0.2 )
	    {
		a = 0;
		ForEach(i, Bp, MaxCase)
		{
		    if ( Weight(Case[i]) <= MinWt + 1E-3 )
		    {
			a += Weight(Case[i]);
			Swap(i, Bp);
			Bp++;
		    }
		}
	    }
	}

	UnitWeights = false;
    }

    FreeUnlessNil(SaveCase);				SaveCase = Nil;

    /*  Decide whether boosting should be abandoned  */

    if ( BOOST && TRIALS <= 2 )
    {
	fprintf(Of, T_Abandoned);
	TRIALS = 1;
    }

    /*  Save trees or rulesets  */

    if ( ! XVAL )
    {
	if ( ! RULES )
	{
	    ForEach(Trial, 0, TRIALS-1)
	    {
		SaveTree(Pruned[Trial], ".tree");
	    }
	}
	else
	{
	    ForEach(Trial, 0, TRIALS-1)
	    {
		SaveRules(RuleSet[Trial], ".rules");
	    }
	}

	fclose(TRf);
    }
    TRf = 0;

    Free(Wrong);					Wrong = Nil;
    FreeUnlessNil(BVoteBlock);				BVoteBlock = Nil;
}



/*************************************************************************/
/*								 	 */
/*	Initialise the weight of each case				 */
/*								 	 */
/*************************************************************************/


void InitialiseWeights()
/*   -----------------  */
{
    CaseNo	i;

    if ( CostWeights )
    {
	/*  Make weights proportional to average error cost  */

	ForEach(i, 0, MaxCase)
	{
	    Weight(Case[i]) = WeightMul[Class(Case[i])];
	}
	UnitWeights = false;
    }
    else
    {
	ForEach(i, 0, MaxCase)
	{
	    Weight(Case[i]) = 1.0;
	}
	UnitWeights = true;
    }

    /*  Adjust when using case weights  */

    if ( CWtAtt )
    {
	ForEach(i, 0, MaxCase)
	{
	    Weight(Case[i]) *= RelCWt(Case[i]);
	}
	UnitWeights = false;
    }
}



/*************************************************************************/
/*								 	 */
/*	Determine average case weight, ignoring cases with unknown,	 */
/*	non-applicable, or negative values of CWtAtt.			 */
/*								 	 */
/*************************************************************************/


void SetAvCWt()
/*   --------  */
{
    CaseNo	i, NCWt=0;
    ContValue	CWt;

    AvCWt = 0;
    ForEach(i, 0, MaxCase)
    {
	if ( ! NotApplic(Case[i], CWtAtt) && ! Unknown(Case[i], CWtAtt) &&
	     (CWt = CVal(Case[i], CWtAtt)) > 0 )
	{
	    NCWt++;
	    AvCWt += CWt;
	}
    }

    AvCWt = ( NCWt > 0 ? AvCWt / NCWt : 1 );
}



/*************************************************************************/
/*									 */
/*	Print report of errors for each of the trials			 */
/*									 */
/*************************************************************************/

char *Multi[]  = {	F_Trial,
			F_UTrial,
			"" },

     *StdR[]   = {	"   Before Pruning   ",
			"  ----------------  ",
			"  " F_SizeErrors "  " },

     *StdP[]   = {	"  " F_DecisionTree16 "  ",
			"  ----------------  ",
			"  " F_SizeErrors "  " },

     *StdPC[]  = {	"  " F_DecisionTree23 "  ",
			"  -----------------------  ",
			"  " F_SizeErrorsCost "  " },

     *Extra[]  = {	"  " F_Rules16,
			"  ----------------",
			"  " F_NoErrors },

     *ExtraC[] = {	"  " F_Rules23,
			"  -----------------------",
			"  " F_NoErrorsCost };


void Evaluate(int Flags)
/*   --------  */
{
    if ( TRIALS == 1 )
    {
	EvaluateSingle(Flags);
    }
    else
    {
	EvaluateBoost(Flags);
    }
}



void EvaluateSingle(int Flags)
/*   --------------  */
{
    ClassNo	RealClass, PredClass;
    int		x, u, SaveUtility;
    CaseNo	*ConfusionMat, *Usage, i, RawErrs=0, Errs=0;
    double	ECost=0, Tests;
    Boolean	CMInfo, UsageInfo;

    CMInfo    = Flags & CMINFO;
    UsageInfo = Flags & USAGEINFO;

    if ( CMInfo )
    {
	ConfusionMat = AllocZero((MaxClass+1)*(MaxClass+1), CaseNo);
    }

    if ( UsageInfo )
    {
	Usage = AllocZero(MaxAtt+1, CaseNo);
    }

    Tests = Max(MaxCase+1, 1);	/* in case no useful test data! */

    if ( UTILITY && RULES )
    {
	SaveUtility = UTILITY;

	UTILITY = Min(UTILITY, RuleSet[0]->SNRules);

	UtilErr  = AllocZero(UTILITY, int);
	UtilBand = Alloc(UTILITY, int);
	if ( MCost )
	{
	    UtilCost = AllocZero(UTILITY, double);
	}

	ForEach(u, 1, UTILITY-1)
	{
	    UtilBand[u] = rint(RuleSet[0]->SNRules * u / (float) UTILITY);
	}
    }
	    
    fprintf(Of, "\n");
    ForEach(x, 0, 2)
    {
	putc('\t', Of);
	if ( RULES )
	{
	    fprintf(Of, "%s", ( MCost ? ExtraC[x] : Extra[x] ));
	}
	else
	{
	    Verbosity(1, fprintf(Of, "%s", StdR[x]))
	    fprintf(Of, "%s", ( MCost ? StdPC[x] : StdP[x] ));
	}
	putc('\n', Of);
    }
    putc('\n', Of);

    ForEach(i, 0, MaxCase)
    {
	RealClass = Class(Case[i]);
	assert(RealClass > 0 && RealClass <= MaxClass);

	memset(Tested, 0, MaxAtt+1);	/* for usage */

	if ( RULES )
	{
	    PredClass = RuleClassify(Case[i], RuleSet[0]);
	}
	else
	{
	    Verbosity(1,
		PredClass = TreeClassify(Case[i], Raw[0]);
		if ( PredClass != RealClass )
		{
		    RawErrs++;
		})

	    PredClass = TreeClassify(Case[i], Pruned[0]);
	}
	assert(PredClass > 0 && PredClass <= MaxClass);

	if ( PredClass != RealClass )
	{
	    Errs++;
	    if ( MCost ) ECost += MCost[PredClass][RealClass];
	}

	if ( CMInfo )
	{
	    ConfusionMat[RealClass*(MaxClass+1)+PredClass]++;
	}

	if ( UsageInfo )
	{
	    RecordAttUsage(Case[i], Usage);
	}
    }

    putc('\t', Of);

    if ( RULES )
    {
	fprintf(Of, "  %4d %4d(%4.1f%%)",
	       RuleSet[0]->SNRules, Errs, 100 * Errs / Tests);
    }
    else
    {
	/*  Results for unpruned tree  */

	Verbosity(1,
	{
	    fprintf(Of, "  %4d %4d(%4.1f%%)  ",
		   TreeSize(Raw[0]), RawErrs, 100 * RawErrs / Tests);
	})

	/*  Results for pruned tree  */

	fprintf(Of, "  %4d %4d(%4.1f%%)",
	       TreeSize(Pruned[0]), Errs, 100 * Errs / Tests);
    }

    if ( MCost )
    {
	fprintf(Of, "%7.2f", ECost / Tests);
    }

    fprintf(Of, "   <<\n");

    if ( CMInfo )
    {
	PrintConfusionMatrix(ConfusionMat);
	Free(ConfusionMat);
    }

    if ( UsageInfo )
    {
	PrintUsageInfo(Usage);
	Free(Usage);
    }

    if ( UtilErr )
    {
	if ( ! XVAL )
	{
	    fprintf(Of, "\n" T_Rule_utility_summary ":\n\n"
			"\t" F_Rules "\t      " F_Errors "%s\n"
			"\t" F_URules "\t      " F_UErrors "%s\n",
			    ( MCost ? "   " F_Cost : "" ),
			    ( MCost ? "   " F_UCost : "" ));

	    ForEach(u, 1, UTILITY-1)
	    {
		fprintf(Of, "\t%s%d\t %4d(%4.1f%%)",
			    ( UtilBand[u] == 1 ? "" : "1-" ), UtilBand[u],
			    UtilErr[u], 100 * UtilErr[u] / Tests);
		if ( MCost )
		{
		    fprintf(Of, "%7.2f", UtilCost[u] / Tests);
		}
		fprintf(Of, "\n");
	    }
	}

	Free(UtilErr);					UtilErr = Nil;
	FreeUnlessNil(UtilCost);			UtilCost = Nil;
	Free(UtilBand);					UtilBand = Nil;

	UTILITY = SaveUtility;
    }
}



void EvaluateBoost(int Flags)
/*   -------------  */
{
    ClassNo	RealClass, PredClass;
    int		t;
    CaseNo	*ConfusionMat, *Usage, i, *Errs, BoostErrs=0;
    double	*ECost, BoostECost=0, Tests;
    Boolean	CMInfo, UsageInfo;

    CMInfo    = Flags & CMINFO;
    UsageInfo = Flags & USAGEINFO;

    if ( CMInfo )
    {
	ConfusionMat = AllocZero((MaxClass+1)*(MaxClass+1), CaseNo);
    }

    if ( UsageInfo )
    {
	Usage = AllocZero(MaxAtt+1, CaseNo);
    }

    Tests = Max(MaxCase+1, 1);	/* in case no useful test data! */
    Errs = AllocZero(TRIALS, CaseNo);
    ECost = AllocZero(TRIALS, double);

    fprintf(Of, "\n");
    ForEach(t, 0, 2)
    {
	fprintf(Of, "%s\t", Multi[t]);
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

    /*  Set global default class for boosting  */

    Default = ( RULES ? RuleSet[0]->SDefault : Pruned[0]->Leaf );

    ForEach(i, 0, MaxCase)
    {
	RealClass = Class(Case[i]);

	memset(Tested, 0, MaxAtt+1);	/* for usage */

	PredClass = BoostClassify(Case[i], TRIALS-1);
	if ( PredClass != RealClass )
	{
	    BoostErrs++;
	    if ( MCost ) BoostECost += MCost[PredClass][RealClass];
	}

	if ( CMInfo )
	{
	    ConfusionMat[RealClass*(MaxClass+1)+PredClass]++;
	}

	if ( UsageInfo )
	{
	    RecordAttUsage(Case[i], Usage);
	}

	/*  Keep track of results for each trial  */

	ForEach(t, 0, TRIALS-1)
	{
	    if ( TrialPred[t] != RealClass )
	    {
		Errs[t]++;
		if ( MCost ) ECost[t] += MCost[TrialPred[t]][RealClass];
	    }
	}
    }

    /*  Print results for individual trials  */

    ForEach(t, 0, TRIALS-1)
    {
	fprintf(Of, "%4d\t", t);

	if ( RULES )
	{
	    fprintf(Of, "  %4d %4d(%4.1f%%)",
		   RuleSet[t]->SNRules, Errs[t], 100 * Errs[t] / Tests);
	}
	else
	{
	    fprintf(Of, "  %4d %4d(%4.1f%%)",
		   TreeSize(Pruned[t]), Errs[t], 100 * Errs[t] / Tests);
	}

	if ( MCost )
	{
	    fprintf(Of, "%7.2f", ECost[t] / Tests);
	}

	putc('\n', Of);
    }

    /*  Print boosted results  */

    if ( RULES )
    {
	fprintf(Of, F_Boost "\t  %9d(%4.1f%%)",
	    BoostErrs, 100 * BoostErrs / Tests);
    }
    else
    {
	fprintf(Of, F_Boost "\t       %4d(%4.1f%%)",
		BoostErrs, 100 * BoostErrs / Tests);
    }

    if ( MCost )
    {
	fprintf(Of, "%7.2f", BoostECost / Tests);
    }

    fprintf(Of, "   <<\n");

    if ( CMInfo )
    {
	PrintConfusionMatrix(ConfusionMat);
	Free(ConfusionMat);
    }

    if ( UsageInfo )
    {
	PrintUsageInfo(Usage);
	Free(Usage);
    }

    Free(Errs);
    Free(ECost);
}



/*************************************************************************/
/*								 	 */
/*	Record atts used when classifying last case			 */
/*								 	 */
/*************************************************************************/


void RecordAttUsage(DataRec Case, int *Usage)
/*   --------------  */
{
    Attribute	Att;
    int		i;

    /*  Scan backwards to allow for information from defined attributes  */

    for ( Att = MaxAtt ; Att > 0 ; Att-- )
    {
	if ( Tested[Att] && ! Unknown(Case, Att) )
	{
	    Usage[Att]++;

	    if ( AttDef[Att] )
	    {
		ForEach(i, 1, AttDefUses[Att][0])
		{
		    Tested[AttDefUses[Att][i]] = true;
		}
	    }
	}
    }
}
