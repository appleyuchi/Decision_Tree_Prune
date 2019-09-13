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
/*								  	 */
/*	Form a set of rules from a decision tree			 */
/*	----------------------------------------			 */
/*								  	 */
/*	The cases are partitioned into sublists:			 */
/*	  * Fail0: those cases that satisfy all undeleted conditions	 */
/*	  * Fail1: those that satisfy all but one of the above		 */
/*	  * FailMany: the remaining cases				 */
/*	Lists are implemented via Succ; Succ[i] is the number of the	 */
/*	case that follows case i.					 */
/*									 */
/*************************************************************************/


#include "defns.i"
#include "extern.i"

double		*Errors=Nil,		/* [Condition] */
		*Total=Nil;		/* [Condition] */

float		*Pessimistic=Nil,	/* [Condition] */
		*CondCost=Nil;		/* [Condition] */

Boolean		**CondFailedBy=Nil,	/* [Condition][CaseNo] */
		*Deleted=Nil;		/* [Condition] */

Condition	*Stack=Nil;

int		MaxDepth=0,		/* depth of tree */
		NCond,
		Bestd;

ClassNo		TargetClass;

short		*NFail=Nil,		/* NFail[i] = conditions failed by i */
		*LocalNFail=Nil;	/* copy used during rule pruning */

CaseNo		Fail0,
		Fail1,
		FailMany,
		*Succ=Nil;		/* case following case i */



/*************************************************************************/
/*								  	 */
/*	Process a tree to extract a ruleset				 */
/*								  	 */
/*************************************************************************/


CRuleSet FormRules(Tree T)
    /*	 ---------  */
{
    int		i;
    CRuleSet	RS;

    NotifyStage(FORMRULES);
    Progress(-(MaxCase+1.0));

    Verbosity(2, PrintTree(T, "Pruned tree:"))

    /*  Find essential parameters and allocate storage  */

    MaxDepth = TreeDepth(T);

    Errors	 = AllocZero(MaxDepth+2, double);
    Total	 = AllocZero(MaxDepth+2, double);

    Pessimistic	 = AllocZero(MaxDepth+2, float);
    CondCost	 = AllocZero(MaxDepth+2, float);

    CondFailedBy = AllocZero(MaxDepth+2, Boolean *);
    Deleted	 = AllocZero(MaxDepth+2, Boolean);

    Stack	 = AllocZero(MaxDepth+2, Condition);

    ForEach(i, 0, MaxDepth+1)
    {
	Stack[i]	= Alloc(1, CondRec);
	CondFailedBy[i] = AllocZero(MaxCase+1, Boolean);
    }

    NFail	 = AllocZero(MaxCase+1, short);
    LocalNFail	 = AllocZero(MaxCase+1, short);

    CovBy	 = AllocZero(MaxCase+2, int);

    List	 = Alloc(MaxCase+2, CaseNo);
    Succ	 = Alloc(MaxCase+1, CaseNo);

    CBuffer	 = Alloc(4 + (MaxCase+1) + (MaxCase+1)/128, Byte);

    NRules = RuleSpace = 0;
    FindClassFreq(ClassFreq, 0, MaxCase);

    if ( ! BranchBits )
    {
	GenerateLogs(Max(MaxCase+1, Max(MaxAtt, Max(MaxClass, MaxDiscrVal))));
	FindTestCodes();
    }

    SetupNCost();

    /*  Extract and prune paths from root to leaves  */

    NCond = 0;
    Scan(T);

    Default = T->Leaf;

    /*  Deallocate storage  */

    FreeFormRuleData();

    /*  Select final rules  */

    SiftRules((T->Errors + MaxClass-1) / (MaxCase+1 + MaxClass));

    FreeVector((void **) NCost, 0, MaxClass);		NCost = Nil;

    CheckActiveSpace(NRules);

    RS = Alloc(1, RuleSetRec);

    RS->SNRules  = NRules;
    RS->SRule    = Rule;				Rule = Nil;
    RS->SDefault = Default;

    ConstructRuleTree(RS);

    return RS;
}



/*************************************************************************/
/*								  	 */
/*	Set up normalised costs.  These are all 0/1 if MCost is not	 */
/*	defined or if cost weighting is used.  Otherwise, MCost is	 */
/*	divided by an estimated average error cost, determined as	 */
/*	follows:							 */
/*									 */
/*	Assume E errors.  The expected number of cases misclassified	 */
/*	as class C is E * P(C), with the real classes distributed	 */
/*	in accordance with their priors.  This gives an expected	 */
/*	total error cost of						 */
/*	    E * sum/C { P(C) * sum/D!=C { P(D)/(1-P(C)) * M[C][D] } }	 */
/*	and dividing by E gives an expected average cost.		 */
/*									 */
/*	The above tends to be pessimistic, so we reduce it somewhat.	 */
/*								  	 */
/*	Siftrules requires a row of NCost corresponding to predicted	 */
/*	class 0 (case not covered by any rule).  All costs in this row	 */
/*	are set to 1.							 */
/*								  	 */
/*************************************************************************/


void SetupNCost()
/*   ----------  */
{
    ClassNo	Real, Pred;
    double	AvErrCost=0, ProbPred, ProbReal;

    NCost = Alloc(MaxClass+1, float *);

    ForEach(Pred, 0, MaxClass)
    {
	NCost[Pred] = Alloc(MaxClass+1, float);

	if ( ! MCost || CostWeights || Pred == 0 )
	{
	    ForEach(Real, 1, MaxClass)
	    {
		NCost[Pred][Real] = ( Pred != Real );
	    }
	}
	else
 	{
	    ProbPred = ClassFreq[Pred] / (MaxCase+1);
	    ForEach(Real, 1, MaxClass)
	    {
		NCost[Pred][Real] = MCost[Pred][Real];
		if ( Real == Pred ) continue;

		ProbReal = ClassFreq[Real] / (MaxCase+1);
		AvErrCost +=
		    ProbPred * (ProbReal / (1 - ProbPred)) * MCost[Pred][Real];
	    }
	}
    }

    if ( MCost && ! CostWeights )
    {
	AvErrCost = (AvErrCost + 1) / 2;	/* reduced average cost */
	ForEach(Real, 1, MaxClass)
	{
	    ForEach(Pred, 1, MaxClass)
	    {
		NCost[Pred][Real] /= AvErrCost;
	    }
	}
    }
}



/*************************************************************************/
/*								  	 */
/*	Extract paths from tree T and prune them to form rules		 */
/*								  	 */
/*************************************************************************/


void Scan(Tree T)
/*   ----  */
{
    DiscrValue	v, Last;
    Condition	Term;

    if ( T->NodeType )
    {
	NCond++;
	Term = Stack[NCond];

	Term->NodeType = T->NodeType;
	Term->Tested   = T->Tested;
	Term->Cut      = T->Cut;

	ForEach(v, 1, T->Forks)
	{
	    /*  Skip branches with empty leaves  */

	    if ( T->Branch[v]->Cases < MinLeaf ) continue;

	    Term->TestValue = v;

	    if ( T->NodeType == BrSubset )
	    {
		if ( Elements(T->Tested, T->Subset[v], &Last) == 1 )
		{
		    /*  Subset contains a single element  */

		    Term->NodeType  = BrDiscr;
		    Term->TestValue = Last;
		}
		else
		{
		    Term->NodeType  = BrSubset;
		    Term->Subset    = T->Subset[v];
		    Term->TestValue = 1;
		}
	    }

	    CondCost[NCond] = CondBits(Term);

	    /*  Adjust number of failed conditions  */

	    PushCondition();

	    Scan(T->Branch[v]);

	    /*  Reset number of failed conditions  */

	    PopCondition();
	}

	NCond--;
    }

    /*  Make a rule from every node of the tree other than the root  */

    if ( NCond > 0 && T->Cases >= 1 )
    {

	memcpy(LocalNFail, NFail, (MaxCase + 1) * sizeof(short));

	TargetClass = T->Leaf;
	PruneRule(Stack, T->Leaf);

	if ( ! T->NodeType ) Progress(T->Cases);
    }
}



/*************************************************************************/
/*								  	 */
/*	Update NFail when a condition is added to/removed from Stack	 */
/*								  	 */
/*************************************************************************/


void PushCondition()
/*   -------------  */
{
    int i;

    ForEach(i, 0, MaxCase)
    {
	if ( (CondFailedBy[NCond][i] = ! Satisfies(Case[i], Stack[NCond])) )
	{
	    NFail[i]++;
	}
    }
}



void PopCondition()
/*   -------------  */
{
    int i;

    ForEach(i, 0, MaxCase)
    {
	if ( CondFailedBy[NCond][i] )
	{
	    NFail[i]--;
	}
    }
}



/*************************************************************************/
/*									 */
/*	Prune the rule given by the conditions Cond, and the number of	 */
/*	conditions NCond, and add the resulting rule to the current	 */
/*	ruleset if it is sufficiently accurate				 */
/*									 */
/*************************************************************************/

#define TI(a,b)		(((a)+(b)) * Log((a)+(b)) - (a) * Log(a) - (b) * Log(b))


void PruneRule(Condition Cond[], ClassNo TargetClass)
/*   ---------  */
{
    int		d, id, Bestid, Remaining=NCond;
    double	RealTotal, RealCorrect;
    CaseNo	i, LL=0;
    float	Prior;
    double	Base, Gain, Cost=0;

    ForEach(d, 0, NCond)
    {
	Deleted[d] = false;
	Total[d]   =
	Errors[d]  = 0;

	if ( d ) Cost += CondCost[d];
    }
    Cost -= LogFact[NCond];

    Base = TI(ClassFreq[TargetClass], MaxCase+1 - ClassFreq[TargetClass]);

    /*  Initialise all fail lists  */

    Bestd = 0;
    ProcessLists();

    ForEach(d, 1, NCond)
    {
	Total[d]  += Total[0];
	Errors[d] += Errors[0];
    }

    /*  Find conditions to delete  */

    Verbosity(1, fprintf(Of, "\n  Pruning rule for %s", ClassName[TargetClass]))

    while (true )
    {
	/*  Find the condition, deleting which would most improve
	    the pessimistic accuracy of the rule.
	    Note: d = 0 means all conditions are satisfied  */

	Bestd = id = 0;

	Gain = Base - TI(Total[0]-Errors[0], Errors[0])
		    - TI(ClassFreq[TargetClass]-Total[0]+Errors[0],
			 MaxCase+1-ClassFreq[TargetClass]-Errors[0]);

	Verbosity(1,
	    fprintf(Of, "\n       Err   Used   Pess\tAbsent condition\n"))

	ForEach(d, 0, NCond)
	{
	    if ( Deleted[d] ) continue;

	    if ( Errors[d] > Total[d] ) Errors[d] = Total[d];

	    Pessimistic[d] = ( Total[d] < Epsilon ? 0.5 :
			       (Errors[d] + 1) / (Total[d] + 2.0) );

	    Verbosity(1,
		fprintf(Of, "   %7.1f%7.1f  %4.1f%%",
		       Errors[d], Total[d], 100 * Pessimistic[d]))

	    if ( ! d )
	    {
		Verbosity(1,
		    fprintf(Of, "\t<base> %.1f/%.1f bits\n", Gain, Cost))
	    }
	    else
	    {
		id++;

		Verbosity(1, PrintCondition(Cond[d]))

		/*  Bestd identifies the condition with lowest pessimistic
		    error  estimate  */

		if ( ! Bestd || Pessimistic[d] <= Pessimistic[Bestd] )
		{
		    Bestd  = d;
		    Bestid = id;
		}
	    }
	}

	if ( Remaining == 1 || ! Bestd || 
	     ( THEORYFRAC * Cost <= Gain &&
	       Pessimistic[Bestd] > Pessimistic[0] ) )
	{
	    break;
	}

	Verbosity(1, fprintf(Of, "\teliminate test %d\n", Bestid))

	Deleted[Bestd] = true;
	Remaining--;
	Cost -= CondCost[Bestd] - LogFact[Remaining+1] + LogFact[Remaining];

	ForEach(d, 1, NCond)
	{
	    if ( d != Bestd )
	    {
		Total[d]  += Total[Bestd] - Total[0];
		Errors[d] += Errors[Bestd] - Errors[0];
	    }
	}
	Total[0]  = Total[Bestd];
	Errors[0] = Errors[Bestd];

	ProcessLists();
    }

    if ( Remaining && Total[0] > 0.99 && THEORYFRAC * Cost <= Gain )
    {
	Prior = ClassFreq[TargetClass] / (MaxCase+1.0);

	/*  Find list of cases covered by this rule and adjust coverage
	    if using costs  */

	if ( ! MCost )
	{
	    RealTotal   = Total[0];
	    RealCorrect = Total[0] - Errors[0];

	    for ( i = Fail0 ; i >= 0 ; i = Succ[i] )
	    {
		List[++LL] = i;
	    }
	}
	else
	if ( CostWeights )
	{
	    /*  Adjust distributions to reverse case weighting  */

	    Prior /= WeightMul[TargetClass];

	    RealTotal = 0;
	    for ( i = Fail0 ; i >= 0 ; i = Succ[i] )
	    {
		RealTotal += Weight(Case[i]) / WeightMul[Class(Case[i])];
		List[++LL] = i;
	    }
	    RealCorrect = (Total[0] - Errors[0]) / WeightMul[TargetClass];
	}
	else
	{
	    /*  Errors have been weighted by NCost -- undo  */

	    RealTotal   = Total[0];
	    RealCorrect = 0;
	    for ( i = Fail0 ; i >= 0 ; i = Succ[i] )
	    {
		RealCorrect += Weight(Case[i]) *
			       (Class(Case[i]) == TargetClass);
		List[++LL] = i;
	    }
	}
	List[0] = LL;

	if ( (RealCorrect + 1) / ((RealTotal + 2) * Prior) >= 0.95 )
	{
	    NewRule(Cond, NCond, TargetClass, Deleted, Nil,
		    RealTotal, RealCorrect, Prior);
	}
    }
}



/*************************************************************************/
/*								  	 */
/*	Change Fail0, Fail1, and FailMany.				 */
/*	If Bestd has not been set, initialise the lists; otherwise	 */
/*	record the changes for deleting condition Bestd and reduce	 */
/*	LocalNFail for cases that do not satisfy condition Bestd	 */
/*								  	 */
/*************************************************************************/


void ProcessLists()
/*   ------------  */
{
    CaseNo	i, iNext, *Prev;
    int		d;

    if ( ! Bestd )
    {
	/*  Initialise the fail list */

	Fail0 = Fail1 = FailMany = -1;

	ForEach(i, 0, MaxCase)
	{
	    if ( ! LocalNFail[i] )
	    {
		Increment(0, i, Total, Errors);
		AddToList(&Fail0, i);
	    }
	    else
	    if ( LocalNFail[i] == 1 )
	    {
		d = SingleFail(i);
		Increment(d, i, Total, Errors);
		AddToList(&Fail1, i);
	    }
	    else
	    {
		AddToList(&FailMany, i);
	    }
	}
    }
    else
    {
	/*  Change the fail list to remove condition Bestd  */

	/*  Promote cases from Fail1 to Fail0  */

	Prev = &Fail1;

	for ( i = Fail1 ; i >= 0 ; )
	{
	    iNext = Succ[i];
	    if ( CondFailedBy[Bestd][i] )
	    {
		DeleteFromList(Prev, i);
		AddToList(&Fail0, i);
	    }
	    else
	    {
		Prev = &Succ[i];
	    }
	    i = iNext;
	}

	/*  Check cases in FailMany  */

	Prev = &FailMany;

	for ( i = FailMany ; i >= 0 ; )
	{
	    iNext = Succ[i];
	    if ( CondFailedBy[Bestd][i] && --LocalNFail[i] == 1 )
	    {
		d = SingleFail(i);
		Increment(d, i, Total, Errors);

		DeleteFromList(Prev, i);
		AddToList(&Fail1, i);
	    }
	    else
	    {
		Prev = &Succ[i];
	    }
	    i = iNext;
	}
    }
}



/*************************************************************************/
/*								  	 */
/*	Add case to list whose first case is *List			 */
/*								  	 */
/*************************************************************************/


void AddToList(CaseNo *List, CaseNo N)
/*   ---------  */
{
    Succ[N] = *List;
    *List   = N;
}



/*************************************************************************/
/*								  	 */
/*	Delete case from list where previous case is *Before		 */
/*								  	 */
/*************************************************************************/


void DeleteFromList(CaseNo *Before, CaseNo N)
/*   --------------  */
{
    *Before = Succ[N];
}



/*************************************************************************/
/*								  	 */
/*	Find single condition failed by a case				 */
/*								  	 */
/*************************************************************************/


int SingleFail(CaseNo i)
/*  ----------  */
{
    int		d;

    ForEach(d, 1, NCond)
    {
	if ( ! Deleted[d] && CondFailedBy[d][i] ) return d;
    }

    return 0;
}



/*************************************************************************/
/*								  	 */
/*	Case i covers all conditions except d; update Total and Errors	 */
/*								  	 */
/*************************************************************************/


void Increment(int d, CaseNo i, double *Total, double *Errors)
/*   ---------  */
{
    Total[d] += Weight(Case[i]);
    Errors[d]+= Weight(Case[i]) * NCost[TargetClass][Class(Case[i])];
}






/*************************************************************************/
/*								  	 */
/*	Free all data allocated for forming rules			 */
/*								  	 */
/*************************************************************************/


void FreeFormRuleData()
/*   ----------------  */
{
    if ( ! CondFailedBy ) return;

    FreeVector((void **) CondFailedBy, 0, MaxDepth+1);	CondFailedBy = Nil;
    FreeVector((void **) Stack, 0, MaxDepth+1);		Stack = Nil;
    Free(Deleted);					Deleted = Nil;
    Free(Pessimistic);					Pessimistic = Nil;
    Free(CondCost);					CondCost = Nil;
    Free(Total);					Total = Nil;
    Free(Errors);					Errors = Nil;
    Free(NFail);					NFail = Nil;
    Free(LocalNFail);					LocalNFail = Nil;
    Free(Succ);						Succ = Nil;
}
