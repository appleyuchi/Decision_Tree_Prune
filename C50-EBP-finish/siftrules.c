/*************************************************************************/
/*									 */
/*  Copyright 2010 Rulequest Research Pty Ltd.				 */
/*  Author: Ross Quinlan (quinlan@rulequest.com) [Rev Jan 2016]		 */
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
/*	Find a good subset of a set of rules				 */
/*	------------------------------------				 */
/*									 */
/*************************************************************************/


#include "defns.i"
#include "extern.i"


float	*DeltaErrs=Nil,	/* DeltaErrs[r]	 = change attributable to rule r or
					   realisable if rule r included */
	*Bits=Nil,	/* Bits[r]	 = bits to encode rule r */
	BitsErr,	/* BitsErr	 = bits to label prediction as error */
	BitsOK;		/* BitsOK	 = bits to label prediction as ok */

int	**TotVote=Nil;	/* TotVote[i][c] = case i's votes for class c */

ClassNo	*TopClass=Nil,	/* TopClass[i]	 = class with highest vote */
	*AltClass=Nil;	/* AltClass[i]	 = class with second highest vote */

Boolean	*RuleIn=Nil,	/* RuleIn[r]	 = rule r included */
	*Covered=Nil;	/* Covered[i]	 = case i covered by rule(s) */

Byte	*CovByBlock=Nil,/* holds entries for inverse of Fires */
	**CovByPtr=Nil;	/* next entry for CovBy[i] */

RuleNo	*LastCovBy=Nil; /* Last rule covering case i  */


/*************************************************************************/
/*									 */
/*	Main rule selection routine.					 */
/*	1.  Form initial theory						 */
/*      2.  Hillclimb in MDL space					 */
/*									 */
/*************************************************************************/


void SiftRules(float EstErrRate)
/*   ---------  */
{
    RuleNo	r;
    int		d, *bp;
    CRule	R;
    float	CodeLength;
    CaseNo	i;

    NotifyStage(SIFTRULES);
    Progress(-(float) NRules);

    /*  Determine inverse of Fires in CovBy, CovByPtr, CovByBlock  */

    InvertFires();

    /*  Clean up any subsets in conditions by removing values that do
	not appear in the covered cases  */

    if ( SUBSET )
    {
	PruneSubsets();
    }

    Covered = Alloc(MaxCase+1, Boolean);
    RuleIn  = AllocZero(NRules+1, Boolean);

    /*  Set initial theory  */

    SetInitialTheory();

    Bits = Alloc(NRules+1, float);

    /*  Calculate the number of bits associated with attribute tests;
	this is not repeated in boosting, composite rulesets etc  */

    if ( ! BranchBits || NRules > MaxCase )
    {
	GenerateLogs(Max(MaxCase+1, Max(MaxAtt, Max(MaxClass,
			 Max(MaxDiscrVal, NRules)))));
    }

    if ( ! BranchBits )
    {
	FindTestCodes();
    }

    /*  Determine rule codelengths  */

    if ( NRules >= MaxCase+1 )
    {
	Realloc(List, NRules+1, CaseNo);
    }

    ForEach(r, 1, NRules)
    {
	R = Rule[r];

	CodeLength = 0;
	ForEach(d, 1, R->Size)
	{
	    CodeLength += CondBits(R->Lhs[d]);
	}
	Bits[r] = CodeLength + LogCaseNo[R->Size] - LogFact[R->Size];
    }

    /*  Use estimated error rate to determine the bits required to
	label a theory's prediction for a case as an error or correct  */

    if ( EstErrRate > 0.5 ) EstErrRate = 0.45;

    BitsErr = - Log(EstErrRate);
    BitsOK  = - Log(1.0 - EstErrRate);


    /*  Allocate tables used in hillclimbing  */

    DeltaErrs = Alloc(NRules+1, float);
    TopClass = Alloc(MaxCase+1, ClassNo);

    AltClass = Alloc(MaxCase+1, ClassNo);
    TotVote  = Alloc(MaxCase+1, int *);

    bp = AllocZero((MaxCase+1) * (MaxClass+1), int);
    ForEach(i, 0, MaxCase)
    {
	TotVote[i] = bp;
	bp += MaxClass + 1;
    }

    /*  Now find best subset of rules  */

    HillClimb();

    /*  Determine default class and reorder rules  */

    SetDefaultClass();
    OrderRules();

    /*  Deallocate storage  */

    FreeSiftRuleData();
}



/*************************************************************************/
/*								  	 */
/*	Find inverse of Fires[][] in CovBy, CovByPtr, and CovByBlock.	 */
/*								  	 */
/*	CovBy[i] = number of rules covering case i (set by NewRule)	 */
/*								  	 */
/*	Set up CovByPtr as pointers into CovByBlock so that		 */
/*	CovByPtr[i] is the start of the compressed entry for case i	 */
/*								  	 */
/*************************************************************************/


void InvertFires()
/*   -----------  */
{
    RuleNo	r, Entry;
    int		j, Blocks, Extra;
    CaseNo	i;
    Byte	*p, *From, *To, *Next;

    CovByPtr = Alloc(MaxCase+2, Byte *);
    Extra = NRules / 128;		/* max number of filler entries */
    CovByPtr[0] = 0;
    ForEach(i, 1, MaxCase+1)
    {
	CovByPtr[i] = CovByPtr[i-1] + CovBy[i-1] + Extra;
    }

    CovByBlock = Alloc((size_t) CovByPtr[MaxCase+1], Byte);
    ForEach(i, 0, MaxCase)
    {
	CovByPtr[i] += (size_t) CovByBlock;
    }

    LastCovBy = AllocZero(MaxCase+1, RuleNo);

    /*  Add entries for each rule  */

    ForEach(r, 1, NRules)
    {
	Uncompress(Fires[r], List);
	ForEach(j, 1, List[0])
	{
	    i = List[j];

	    /*  Add compressed entry for this rule  */

	    p = CovByPtr[i];
	    Entry = r - LastCovBy[i];
	    LastCovBy[i] = r;

	    while ( Entry > 127 )
	    {
		Blocks = (Entry >> 7);
		if ( Blocks > 127 ) Blocks = 127;
		Entry -= Blocks * 128;
		*p++   = Blocks + 128;
	    }

	    *p++ = Entry;
	    CovByPtr[i] = p;
	}
    }

    Free(LastCovBy);					LastCovBy = Nil;

    /*  Reset CovByPtr entries and compact  */

    To   = CovByPtr[0];
    From = CovByPtr[0] = CovByBlock;

    ForEach(i, 1, MaxCase)
    {
	From += CovBy[i-1] + Extra;
	Next  = CovByPtr[i];
	CovByPtr[i] = To;

	for ( p = From ; p < Next ; )
	{
	    *To++ = *p++;
	}
    }

    /*  Reduce CovByBlock to size actually used  */

    From = CovByBlock;			/* current address */

    Realloc(CovByBlock, To - CovByBlock, Byte);

    if ( CovByBlock != From )
    {
	/*  CovByBlock has been moved  */

	ForEach(i, 0, MaxCase)
	{
	    CovByPtr[i] += CovByBlock - From;
	}
    }
}



/*************************************************************************/
/*								  	 */
/*	Determine code lengths for attributes and branches		 */
/*								  	 */
/*************************************************************************/


void FindTestCodes()
/*   -------------  */
{
    Attribute	Att;
    DiscrValue	v, V;
    CaseNo	i, *ValFreq;
    int		PossibleAtts=0;
    float	Sum;

    BranchBits = AllocZero(MaxAtt+1, float);
    AttValues  = AllocZero(MaxAtt+1, int);

    ForEach(Att, 1, MaxAtt)
    {
	if ( Skip(Att) || Att == ClassAtt ) continue;

	PossibleAtts++;

	if ( Ordered(Att) )
	{
	    BranchBits[Att] = 1 + 0.5 * LogCaseNo[MaxAttVal[Att] - 1];
	}
	else
	if ( (V = MaxAttVal[Att]) )
	{
	    /*  Discrete attribute  */

	    ValFreq = AllocZero(V+1, CaseNo);

	    ForEach(i, 0, MaxCase)
	    {
		assert(XDVal(Case[i],Att) >= 0 && XDVal(Case[i],Att) <= V);
		ValFreq[ XDVal(Case[i],Att) ]++;
	    }

	    Sum = 0;
	    ForEach(v, 1, V)
	    {
		if ( ValFreq[v] )
		{
		    Sum += (ValFreq[v] / (MaxCase+1.0)) *
			   (LogCaseNo[MaxCase+1] - LogCaseNo[ValFreq[v]]);
		    AttValues[Att]++;
		}
	    }
	    Free(ValFreq);

	    BranchBits[Att] = Sum;
	}
	else
	{
	    /*  Continuous attribute  */

	    BranchBits[Att] = PossibleCuts[Att] > 1 ?
			      1 + 0.5 * LogCaseNo[PossibleCuts[Att]] : 0 ;
	}
    }

    AttTestBits = LogCaseNo[PossibleAtts];
}



/*************************************************************************/
/*									 */
/*	Determine the number of bits required to encode a condition	 */
/*									 */
/*************************************************************************/


float CondBits(Condition C)
/*    --------  */
{
    Attribute	Att;
    float	Code=0;
    int		Elts=0;
    DiscrValue	v;

    Att = C->Tested;
    switch ( C->NodeType )
    {
	case BrDiscr:		/* test of discrete attribute */
	case BrThresh:		/* test of continuous attribute */

	    return AttTestBits + BranchBits[Att];

	case BrSubset:		/* subset test on discrete attribute  */

	    /* Ignore subset test form for ordered attributes  */

	    if ( Ordered(Att) )
	    {
		return AttTestBits + BranchBits[Att];
	    }

	    ForEach(v, 1, MaxAttVal[Att])
	    {
		if ( In(v, C->Subset) )
		{
		    Elts++;
		}
	    }
	    Elts = Min(Elts, AttValues[Att] - 1);  /* if values not present */
	    Code = LogFact[AttValues[Att]] -
		   (LogFact[Elts] + LogFact[AttValues[Att] - Elts]);

	    return AttTestBits + Code;
    }
}



/*************************************************************************/
/*									 */
/*	Select initial theory.  This is important, since the greedy	 */
/*	optimization procedure is very sensitive to starting with	 */
/*	a reasonable theory.						 */
/*									 */
/*	The theory is constructed class by class.  For each class,	 */
/*	rules are added in confidence order until all of the cases of	 */
/*	that class are covered.  Rules that do not improve coverage	 */
/*	are skipped.							 */
/*									 */
/*************************************************************************/


void SetInitialTheory()
/*   ----------------  */
{
    ClassNo	c;
    RuleNo	r, Active=0;

    ForEach(c, 1, MaxClass)
    {
	CoverClass(c);
    }

    /*  Remove rules that don't help coverage  */

    ForEach(r, 1, NRules)
    {
	if ( (RuleIn[r] &= 1) ) Active++;
    }
}



void CoverClass(ClassNo Target)
/*   ----------  */
{
    CaseNo	i;
    double	Remaining, FalsePos=0, NewFalsePos, NewTruePos;
    RuleNo	r, Best;
    int		j;

    memset(Covered, false, MaxCase+1);

    Remaining = ClassFreq[Target];

    while ( Remaining > FalsePos )
    {
	/*  Find most accurate unused rule from a leaf  */

	Best = 0;
	ForEach(r, 1, NRules)
	{
	    if ( Rule[r]->Rhs == Target && ! RuleIn[r] &&
		 Rule[r]->Correct >= MINITEMS )
	    {
		if ( ! Best || Rule[r]->Vote > Rule[Best]->Vote ) Best = r;
	    }
	}

	if ( ! Best ) return;

	/*  Check increased coverage  */

	NewFalsePos = NewTruePos = 0;

	Uncompress(Fires[Best], List);
	for( j = List[0] ; j ; j-- )
	{
	    i = List[j];
	    if ( ! Covered[i] )
	    {
		if ( Class(Case[i]) == Target )
		{
		    NewTruePos += Weight(Case[i]);
		}
		else
		{
		    NewFalsePos += Weight(Case[i]);
		}
	    }
	}

	/*  If coverage is not increased, set RuleIn to 2 so that
	    the rule can be removed later  */

	if ( NewTruePos - NewFalsePos <= MINITEMS + Epsilon )
	{
	    RuleIn[Best] = 2;
	}
	else
	{
	    Remaining -= NewTruePos;
	    FalsePos  += NewFalsePos;

	    RuleIn[Best] = true;

	    Uncompress(Fires[Best], List);
	    for( j = List[0] ; j ; j-- )
	    {
		i = List[j];
		if ( ! Covered[i] )
		{
		    Covered[i] = true;
		}
	    }
	}
    }
}



/*************************************************************************/
/*									 */
/*	Calculate total message length as				 */
/*	  THEORYFRAC * cost of transmitting theory			 */
/*	  + cost of identifying and correcting errors			 */
/*									 */
/*	The cost of identifying errors assumes that the final theory	 */
/*	will have about the same error rate as the pruned tree, so	 */
/*	is approx. the sum of the corresponding messages.		 */
/*									 */
/*************************************************************************/


double MessageLength(RuleNo NR, double RuleBits, float Errs)
/*  -------------  */
{
    return
	(THEORYFRAC * Max(0, RuleBits - LogFact[NR]) +
	 Errs * BitsErr + (MaxCase+1 - Errs) * BitsOK +
	 Errs * LogCaseNo[MaxClass-1]);
}



/*************************************************************************/
/*									 */
/*	Improve a subset of rules by adding and deleting rules.		 */
/*	MDL costs are rounded to nearest 0.01 bit			 */
/*									 */
/*************************************************************************/


void HillClimb()
/*   ---------  */
{
    RuleNo	r, RuleCount=0, OriginalCount, Toggle, LastToggle=0;
    int		OutCount;
    CaseNo	i;
    int		j;
    CaseCount	Errs;
    double	RuleBits=0;
    double	LastCost=1E99, CurrentCost, AltCost, NewCost;
    Boolean	DeleteOnly=false;

    ForEach(r, 1, NRules)
    {
	if ( RuleIn[r] )
	{
	    RuleBits += Bits[r];
	    RuleCount++;
	}
    }
    OriginalCount = RuleCount;

    InitialiseVotes();
    Verbosity(1, fprintf(Of, "\n"))

    /*  Initialise DeltaErrs[]  */

    Errs = CalculateDeltaErrs();

    /*  Add or drop rule with greatest reduction in coding cost  */

    while ( true )
    {
	CurrentCost = NewCost = MessageLength(RuleCount, RuleBits, Errs);

	Verbosity(1,
	    fprintf(Of, "\t%d rules, %.1f errs, cost=%.1f bits\n",
		   RuleCount, Errs, CurrentCost/100.0);

	    if ( ! DeleteOnly && CurrentCost > LastCost )
	    {
		fprintf(Of, "ERROR %g %g\n",
			    CurrentCost/1000.0, LastCost/100.0);
		break;
	    })

	Toggle = OutCount = 0;

	ForEach(r, 1, NRules)
	{
	    if ( r == LastToggle ) continue;

	    if ( RuleIn[r] )
	    {
		AltCost = MessageLength(RuleCount - 1,
					RuleBits - Bits[r],
					Errs + DeltaErrs[r]);
	    }
	    else
	    {
		if ( Errs < 1E-3 || DeleteOnly ) continue;

		AltCost = MessageLength(RuleCount + 1,
					RuleBits + Bits[r],
					Errs + DeltaErrs[r]);
	    }

	    Verbosity(2,
		if ( ! (OutCount++ % 5) ) fprintf(Of, "\n\t\t");
		fprintf(Of, "%d<%g=%.1f> ",
			    r, DeltaErrs[r], (AltCost - CurrentCost)/100.0))

	    if ( AltCost < NewCost ||
		 AltCost == NewCost && RuleIn[r] )
	    {
		Toggle  = r;
		NewCost = AltCost;
	    }
	}

	if ( ! DeleteOnly && NewCost > CurrentCost )
	{
	    DeleteOnly = true;
	    Verbosity(1, fprintf(Of, "(start delete mode)\n"))
	}

	Verbosity(2, fprintf(Of, "\n"))

	if ( ! Toggle || DeleteOnly && RuleCount <= OriginalCount ) break;

	Verbosity(1,
	    fprintf(Of, "\t%s rule %d/%d (errs=%.1f, cost=%.1f bits)\n",
		   ( RuleIn[Toggle] ? "Delete" : "Add" ),
		   Rule[Toggle]->TNo, Rule[Toggle]->RNo,
		   Errs + DeltaErrs[Toggle], NewCost/100.0))

	/*  Adjust vote information  */

	Uncompress(Fires[Toggle], List);
	for ( j = List[0] ; j ; j-- )
	{
	    i = List[j];

	    /*  Downdate DeltaErrs for all rules except Toggle that cover i  */

	    UpdateDeltaErrs(i, -Weight(Case[i]), Toggle);

	    if ( RuleIn[Toggle] )
	    {
		TotVote[i][Rule[Toggle]->Rhs] -= Rule[Toggle]->Vote;
	    }
	    else
	    {
		TotVote[i][Rule[Toggle]->Rhs] += Rule[Toggle]->Vote;
	    }

	    CountVotes(i);

	    /*  Update DeltaErrs for all rules except Toggle that cover i  */

	    UpdateDeltaErrs(i, Weight(Case[i]), Toggle);
	}

	/*  Update information about rules selected and current errors  */

	if ( RuleIn[Toggle] )
	{
	    RuleIn[Toggle] = false;
	    RuleBits -= Bits[Toggle];
	    RuleCount--;
	}
	else
	{
	    RuleIn[Toggle] = true;
	    RuleBits += Bits[Toggle];
	    RuleCount++;
	}

	Errs += DeltaErrs[Toggle];
	DeltaErrs[Toggle] = - DeltaErrs[Toggle];

	LastToggle = Toggle;
	LastCost   = CurrentCost;

	Progress(1.0);
    }
}



/*************************************************************************/
/*									 */
/*	Determine votes for each case from initial rules		 */
/*	Note: no vote for default class					 */
/*									 */
/*************************************************************************/


void InitialiseVotes()
/*   ---------------  */
{
    CaseNo	i;
    int		j, Vote;
    ClassNo	Rhs;
    RuleNo	r;

    /*  Adjust vote for each case covered by rule  */

    ForEach(r, 1, NRules)
    {
	if ( ! RuleIn[r] ) continue;

	Rhs  = Rule[r]->Rhs;
	Vote = Rule[r]->Vote;

	Uncompress(Fires[r], List);
	for ( j = List[0] ; j ; j-- )
	{
	    TotVote[List[j]][Rhs] += Vote;
	}
    }

    /*  Find the best and alternate class for each case  */

    ForEach(i, 0, MaxCase)
    {
	CountVotes(i);
    }
}



/*************************************************************************/
/*									 */
/*	Find the best and second-best class for each case using the	 */
/*	current values of TotVote					 */
/*									 */
/*************************************************************************/


void CountVotes(CaseNo i)
/*   ----------  */
{
    ClassNo	c, First=0, Second=0;
    int		V;

    ForEach(c, 1, MaxClass)
    {
	if ( (V = TotVote[i][c]) )
	{
	    if ( ! First || V > TotVote[i][First] )
	    {
		Second = First;
		First  = c;
	    }
	    else
	    if ( ! Second || V > TotVote[i][Second] )
	    {
		Second = c;
	    }
	}
    }

    TopClass[i] = First;
    AltClass[i] = Second;
}



/*************************************************************************/
/*									 */
/*	Adjust DeltaErrors for all rules except Toggle that cover case i */
/*									 */
/*************************************************************************/


#define Prefer(d,c1,c2) ((d) > 0 || (d) == 0 && c1 < c2)

void UpdateDeltaErrs(CaseNo i, double Delta, RuleNo Toggle)
/*   ---------------  */
{
    ClassNo	RealClass, Top, Alt, Rhs;
    RuleNo	r;
    Byte	*p;
    int		k;

    RealClass = Class(Case[i]);
    Top	= TopClass[i];
    Alt = AltClass[i];

    r = 0;
    p = CovByPtr[i];
    ForEach(k, 1, CovBy[i])
    {
	/*  Update r to next rule covering case i  */

	while ( (*p) & 128 )
	{
	    r += ((*p++) & 127) * 128;
	}
	r += *p++;

	if ( r != Toggle )
	{
	    /*  Examine effect of adding or deleting rule  */
	
	    Rhs = Rule[r]->Rhs;

	    if ( RuleIn[r] )
	    {
		if ( Rhs == Top &&
		     Prefer(TotVote[i][Alt] - (TotVote[i][Top] - Rule[r]->Vote),
			    Alt, Top) )
		{
		    DeltaErrs[r] +=
			(NCost[Alt][RealClass] - NCost[Top][RealClass]) * Delta;
		}
	    }
	    else
	    {
		if ( Rhs != Top &&
		     Prefer(TotVote[i][Rhs] + Rule[r]->Vote - TotVote[i][Top],
			    Rhs, Top) )
		{
		    DeltaErrs[r] +=
			(NCost[Rhs][RealClass] - NCost[Top][RealClass]) * Delta;
		}
	    }
	}
    }
}



/*************************************************************************/
/*									 */
/*	Calculate initial value of DeltaErrs and total errors		 */
/*									 */
/*************************************************************************/


CaseCount CalculateDeltaErrs()
/*        ------------------  */
{
    RuleNo	r;
    CaseNo	i;
    double	Errs=0;

    ForEach(i, 0, MaxCase)
    {
	Errs += Weight(Case[i]) * NCost[TopClass[i]][Class(Case[i])];
    }

    ForEach(r, 1, NRules)
    {
	DeltaErrs[r] = 0;
    }

    ForEach(i, 0, MaxCase)
    {
	UpdateDeltaErrs(i, Weight(Case[i]), 0);
    }

    return Errs;
}



/*************************************************************************/
/*									 */
/*	Remove unrepresented values from subsets			 */
/*									 */
/*************************************************************************/


void PruneSubsets()
/*   ------------  */
{
    Set		*PossibleValues;
    Attribute	Att, *Atts, Last;
    int		*Bytes, d, NAtts, j, b;
    CaseNo	i;
    CRule	R;
    RuleNo	r;

    /*  Allocate subsets for possible values  */

    Atts  = Alloc(MaxAtt+1, Attribute);
    Bytes = Alloc(MaxAtt+1, int);

    PossibleValues = AllocZero(MaxAtt+1, Set);
    ForEach(Att, 1, MaxAtt)
    {
	if ( MaxAttVal[Att] > 3 )
	{
	    Bytes[Att] = (MaxAttVal[Att]>>3)+1;
	    PossibleValues[Att] = AllocZero(Bytes[Att], Byte);
	}
    }

    /*  Check each rule in turn  */

    ForEach(r, 1, NRules)
    {
	R = Rule[r];
	NAtts = 0;

	/*  Find all subset conditions  */

	ForEach(d, 1, R->Size)
	{
	    if ( R->Lhs[d]->NodeType != BrSubset ) continue;

	    Atts[++NAtts] = Att = R->Lhs[d]->Tested;
	    ClearBits(Bytes[Att], PossibleValues[Att]);
	}

	if ( ! NAtts ) continue;	/* no subset conditions */

	/*  Scan cases covered by this rule  */

	Uncompress(Fires[r], List);
	for ( j = List[0] ; j ; j-- )
	{
	    i = List[j];

	    /*  Record values of listed attributes  */

	    ForEach(d, 1, NAtts)
	    {
		Att = Atts[d];
		SetBit(DVal(Case[i], Att), PossibleValues[Att]);
	    }
	}

	/*  Delete unrepresented values  */

	ForEach(d, 1, R->Size)
	{
	    if ( R->Lhs[d]->NodeType != BrSubset ) continue;

	    Att = R->Lhs[d]->Tested;
	    ForEach(b, 0, Bytes[Att]-1)
	    {
		R->Lhs[d]->Subset[b] &= PossibleValues[Att][b];
	    }

	    if ( Elements(Att, R->Lhs[d]->Subset, &Last) == 1 )
	    {
		R->Lhs[d]->NodeType  = BrDiscr;
		R->Lhs[d]->TestValue = Last;
		Free(R->Lhs[d]->Subset);
	    }
	}
    }

    FreeVector((void **) PossibleValues, 1, MaxAtt);
    Free(Bytes);
    Free(Atts);
}



/*************************************************************************/
/*									 */
/*	Choose the default class as the one with the maximum		 */
/*	weight of uncovered cases					 */
/*									 */
/*************************************************************************/


void SetDefaultClass()
/*   ---------------  */
{
    RuleNo	r;
    ClassNo	c;
    double	*UncoveredWeight, TotUncovered=1E-3;
    CaseNo	i, j;

    memset(Covered, false, MaxCase+1);
    UncoveredWeight = AllocZero(MaxClass+1, double);

    /*  Check which cases are covered by at least one rule  */

    ForEach(r, 1, NRules)
    {
	if ( ! RuleIn[r] ) continue;

	Uncompress(Fires[r], List);
	for ( j = List[0] ; j ; j-- )
	{
	    Covered[List[j]] = true;
	}
    }

    /*  Find weights by class of uncovered cases  */

    ForEach(i, 0, MaxCase)
    {
	if ( ! Covered[i] )
	{
	    UncoveredWeight[ Class(Case[i]) ] += Weight(Case[i]);
	    TotUncovered += Weight(Case[i]);
	}
    }

    /*  Choose new default class using rel freq and rel uncovered  */

    Verbosity(1, fprintf(Of, "\n    Weights of uncovered cases:\n"));

    ForEach(c, 1, MaxClass)
    {
	Verbosity(1, fprintf(Of, "\t%s (%.2f): %.1f\n",
			    ClassName[c], ClassFreq[c] / (MaxCase + 1.0),
			    UncoveredWeight[c]));

	ClassSum[c] = (UncoveredWeight[c] + 1) / (TotUncovered + 2.0) +
		      ClassFreq[c] / (MaxCase + 1.0);
    }

    Default = SelectClass(1, (Boolean) (MCost && ! CostWeights));

    Free(UncoveredWeight);
}



/*************************************************************************/
/*									 */
/*	Swap two rules							 */
/*									 */
/*************************************************************************/


void SwapRule(RuleNo A, RuleNo B)
/*   --------  */
{
    CRule	Hold;
    Boolean	HoldIn;

    Hold    = Rule[A];
    Rule[A] = Rule[B];
    Rule[B] = Hold;

    HoldIn    = RuleIn[A];
    RuleIn[A] = RuleIn[B];
    RuleIn[B] = HoldIn;
}



/*************************************************************************/
/*									 */
/*	Order rules by utility, least important first			 */
/*	(Called after HilClimb(), so RuleIn etc already known.)		 */
/*									 */
/*************************************************************************/


int OrderByUtility()
/*  --------------  */
{
    RuleNo	r, *Drop, NDrop=0, NewNRules=0, Toggle;
    CaseNo	i;
    int		j, OutCount;
    double	Errs=0;

    Verbosity(1, fprintf(Of, "\n    Determining rule utility\n"))

    Drop = Alloc(NRules, RuleNo);

    /*  Find the rule that has the least beneficial effect on accuracy  */

    while ( true )
    {
	Toggle = OutCount = 0;

	ForEach(r, 1, NRules)
	{
	    if ( ! RuleIn[r] ) continue;

	    Verbosity(2,
		if ( ! (OutCount++ %10 ) ) fprintf(Of, "\n\t\t");
		fprintf(Of, "%d<%g> ", r, DeltaErrs[r]))

	    if ( ! Toggle ||
		 DeltaErrs[r] < DeltaErrs[Toggle] - 1E-3 ||
		 ( DeltaErrs[r] < DeltaErrs[Toggle] + 1E-3 &&
		   Bits[r] > Bits[Toggle] ) )
	    {
		Toggle = r;
	    }
	}
	Verbosity(2, fprintf(Of, "\n"))

	if ( ! Toggle ) break;

	Verbosity(1,
	    fprintf(Of, "\tDelete rule %d/%d (errs up %.1f)\n",
		   Rule[Toggle]->TNo, Rule[Toggle]->RNo,
		   Errs + DeltaErrs[Toggle]))

	/*  Adjust vote information  */

	Uncompress(Fires[Toggle], List);
	for ( j = List[0] ; j ; j-- )
	{
	    i = List[j];

	    /*  Downdate DeltaErrs for all rules except Toggle that cover i  */

	    UpdateDeltaErrs(i, -Weight(Case[i]), Toggle);

	    TotVote[i][Rule[Toggle]->Rhs] -= Rule[Toggle]->Vote;

	    CountVotes(i);

	    /*  Update DeltaErrs for all rules except Toggle that cover i  */

	    UpdateDeltaErrs(i, Weight(Case[i]), Toggle);
	}

	Drop[NDrop++]  = Toggle;
	RuleIn[Toggle] = false;

	Errs += DeltaErrs[Toggle];
    }

    /*  Now reverse the order  */

    while ( --NDrop >= 0 )
    {
	NewNRules++;
	RuleIn[Drop[NDrop]] = true;
	SwapRule(Drop[NDrop], NewNRules);

	/*  Have to alter rule number in Drop  */
	ForEach(r, 0, NDrop-1)
	{
	    if ( Drop[r] == NewNRules ) Drop[r] = Drop[NDrop];
	}
    }
    Free(Drop);

    return NewNRules;
}




/*************************************************************************/
/*									 */
/*	Order rules by class and then by rule CF			 */
/*									 */
/*************************************************************************/


int OrderByClass()
/*  ------------  */
{
    RuleNo	r, nr, NewNRules=0;
    ClassNo	c;

    ForEach(c, 1, MaxClass)
    {
	while ( true )
	{
	    nr = 0;
	    ForEach(r, NewNRules+1, NRules)
	    {
		if ( RuleIn[r] && Rule[r]->Rhs == c &&
		     ( ! nr || Rule[r]->Vote > Rule[nr]->Vote ) )
		{
		    nr = r;
		}
	    }

	    if ( ! nr ) break;

	    NewNRules++;
	    if ( nr != NewNRules )
	    {
		SwapRule(NewNRules, nr);
	    }
	}
    }

    return NewNRules;
}



/*************************************************************************/
/*									 */
/*	Discard deleted rules and sequence and renumber those remaining. */
/*	Sort by class and then by rule CF or by utility			 */
/*									 */
/*************************************************************************/


void OrderRules()
/*   ----------  */
{
    RuleNo	r, NewNRules;

    NewNRules = ( UTILITY ? OrderByUtility() : OrderByClass() );

    ForEach(r, 1, NewNRules)
    {
	Rule[r]->RNo = r;
    }

    /*  Free discarded rules  */

    ForEach(r, NewNRules+1, NRules)
    {
	FreeRule(Rule[r]);
    }

    NRules = NewNRules;
}



/*************************************************************************/
/*									 */
/*	Tabluate logs and log factorials (to improve speed)		 */
/*									 */
/*************************************************************************/


void GenerateLogs(int MaxN)
/*   ------------  */
{
    CaseNo	i;

    if ( LogCaseNo )
    {
	Realloc(LogCaseNo, MaxN+2, double);
	Realloc(LogFact, MaxN+2, double);
    }
    else
    {
	LogCaseNo = Alloc(MaxN+2, double);
	LogFact   = Alloc(MaxN+2, double);
    }

    LogCaseNo[0] = -1E38;
    LogCaseNo[1] = 0;

    LogFact[0] = LogFact[1] = 0;

    ForEach(i, 2, MaxN+1)
    {
	LogCaseNo[i] = Log((double) i);
	LogFact[i]   = LogFact[i-1] + LogCaseNo[i];
    }
}



void FreeSiftRuleData()
/*   ----------------  */
{
    FreeUnlessNil(List);				List = Nil;
    FreeVector((void **) Fires, 1, RuleSpace-1);	Fires = Nil;
    FreeUnlessNil(CBuffer);				CBuffer = Nil;
    FreeUnlessNil(Covered);				Covered = Nil;
    FreeUnlessNil(RuleIn);				RuleIn = Nil;
    FreeUnlessNil(CovBy);				CovBy = Nil;
    FreeUnlessNil(CovByPtr);				CovByPtr = Nil;
    FreeUnlessNil(BranchBits);				BranchBits = Nil;
    FreeUnlessNil(AttValues);				AttValues = Nil;

    FreeUnlessNil(DeltaErrs);				DeltaErrs = Nil;
    FreeUnlessNil(CovByBlock);				CovByBlock = Nil;
    FreeUnlessNil(Bits);				Bits = Nil;
    FreeUnlessNil(TopClass);				TopClass = Nil;
    FreeUnlessNil(AltClass);				AltClass = Nil;
    if ( TotVote )
    {
	FreeUnlessNil(TotVote[0]);
	FreeUnlessNil(TotVote);				TotVote = Nil;
    }
}
