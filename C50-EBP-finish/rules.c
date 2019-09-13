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
/*	Miscellaneous routines for rule handling		  	 */
/*	----------------------------------------		  	 */
/*								  	 */
/*************************************************************************/


#include "defns.i"
#include "extern.i"


/*************************************************************************/
/*								  	 */
/*	Add a new rule to the current ruleset, by updating Rule[],  	 */
/*	NRules and, if necessary, RuleSpace			  	 */
/*								  	 */
/*************************************************************************/


Boolean NewRule(Condition Cond[], int NCond, ClassNo TargetClass,
		Boolean *Deleted, CRule Existing,
		CaseCount Cover, CaseCount Correct, float Prior)
/*      -------  */
{
    int		d, dd, id, r, Size=0, Bytes;
    CaseNo	i;
    CRule	R;
    Condition	*Lhs;
    Boolean	Exclude=false;
    int		Vote;

    /*  Sort and copy the conditions if required  */

    if ( ! Existing )
    {
	ForEach(d, 1, NCond)
	{
	    if ( ! Deleted[d] ) Size++;
	}

	Lhs = Alloc(Size+1, Condition);

	/*  Sort conditions in print order  */

	ForEach(d, 1, Size)
	{
	    dd =  0;
	    ForEach(id, 1, NCond)
	    {
		if ( ! Deleted[id] && ( ! dd || Before(Cond[id], Cond[dd]) ) )
		{
		    dd = id;
		}
	    }

	    Lhs[d] = Alloc(1, CondRec);
	    memcpy(Lhs[d], Cond[dd], sizeof(CondRec));
	    if ( Lhs[d]->NodeType == BrSubset )
	    {
		Bytes = (MaxAttVal[Lhs[d]->Tested]>>3) + 1;
		Lhs[d]->Subset = Alloc(Bytes, Byte);
		memcpy(Lhs[d]->Subset, Cond[dd]->Subset, Bytes);
	    }

	    Deleted[dd] = true;
	}
    }
    else
    {
	Lhs  = Cond;
	Size = NCond;
    }

    Vote = 1000 * (Correct + 1.0) / (Cover + 2.0) + 0.5;

    /*  See if rule already exists  */

    for ( r = 1 ; ! Exclude && r <= NRules ; r++ )
    {
	if ( SameRule(r, Lhs, Size, TargetClass) )
	{
	    Verbosity(1, fprintf(Of, "\tduplicates rule %d\n", r))

	    /*  Keep the most optimistic error estimate  */

	    if ( Vote > Rule[r]->Vote )
	    {
		Rule[r]->Vote = Vote;
	    }

	    Exclude = true;
	}
    }

    if ( Exclude )
    {
	if ( ! Existing )
	{
	    ForEach(d, 1, Size)
	    {
		if ( Lhs[d]->NodeType == BrSubset ) Free(Lhs[d]->Subset);
	    }
	    FreeVector((void **) Lhs, 1, Size);
	}

	return false;
    }

    /*  Make sure there is enough room for the new rule  */

    NRules++;
    if ( NRules >= RuleSpace )
    {
	RuleSpace += 100;
	if ( RuleSpace > 100 )
	{
	    Realloc(Rule,  RuleSpace, CRule);
	    Realloc(Fires, RuleSpace, Byte *);
	    ForEach(r, RuleSpace-100, RuleSpace-1)
	    {
		Fires[r] = Nil;
	    }
	}
	else
	{
	    Rule  = Alloc(RuleSpace, CRule);
	    Fires = AllocZero(RuleSpace, Byte *);
	}
    }

    /*  Form the new rule  */

    Rule[NRules] = R = Alloc(1, RuleRec);

    R->TNo     = ( Existing ? Existing->TNo : Trial );
    R->RNo     = ( Existing ? Existing->RNo : NRules );
    R->Size    = Size;
    R->Lhs     = Lhs;
    R->Rhs     = TargetClass;
    R->Cover   = Cover;
    R->Correct = Correct;
    R->Prior   = Prior;
    R->Vote    = Vote;

    /*  Record entry in Fires and CovBy  */

    ListSort(List, 1, List[0]);
    Fires[NRules] = Compress(List);

    ForEach(i, 1, List[0])
    {
	CovBy[List[i]]++;
    }

    Verbosity(1, if ( ! Existing ) PrintRule(R))

    return true;
}



/*************************************************************************/
/*								  	 */
/*	Compress list of ascending integers.				 */
/*								  	 */
/*	The first integer occupies 4 bytes.  Each subsequent integer is	 */
/*	represented as the increment on the previous and is encoded as	 */
/*	one or more bytes b0 + b1 + .... where				 */
/*	  if byte b < 128, value is b					 */
/*	  if byte b = 128 + x, value is x * 128				 */
/*								  	 */
/*	For example, an increment 4321 (= 33 * 128 + 97) is encoded as	 */
/*	two bytes [128 + 33] [97]					 */
/*								  	 */
/*************************************************************************/


Byte *Compress(int *L)
/*    --------  */
{
    int		i, Last=0, Entry, Blocks;
    Byte	*p, *Compressed;

    /*  Copy first integer (uncompressed)  */

    memcpy(CBuffer, L, 4);
    p = CBuffer + 4;

    ForEach(i, 1, L[0])
    {
	Entry = L[i] - Last;
	Last  = L[i];

	/*  Place any necessary skip bytes  */

	while ( Entry > 127 )
	{
	    Blocks = (Entry >> 7);
	    if ( Blocks > 127 ) Blocks = 127;
	    Entry -= Blocks * 128;
	    *p++   = Blocks + 128;
	}

	*p++ = Entry;
    }

    Compressed = Alloc(p - CBuffer, Byte);
    memcpy(Compressed, CBuffer, p - CBuffer);

    return Compressed;
}



void Uncompress(Byte *CL, int *UCL)
/*   ----------  */
{
    int		i, Entry=0;
    Byte	*p;

    memcpy(UCL, CL, 4);
    p = CL + 4;

    ForEach(i, 1, UCL[0])
    {
	while ( (*p) & 128 )
	{
	    Entry += ((*p++) & 127) * 128;
	}

	Entry = UCL[i] = Entry + *p++;
    }
}



/*************************************************************************/
/*								  	 */
/*	Sort list in preparation for Compress()				 */
/*								  	 */
/*************************************************************************/


void ListSort(int *L, int Fp, int Lp)
/*   --------  */
{
    int		i, High, Middle, Thresh, Temp;

    if ( Fp < Lp )
    {
	Thresh = L[(Fp+Lp) / 2];

	/*  Divide cases into three groups:
		Fp .. Middle-1: values < Thresh
		Middle .. High: values = Thresh
		High+1 .. Lp:   values > Thresh  */

	for ( Middle = Fp ; L[Middle] < Thresh ; Middle++ )
	    ;

	for ( High = Lp ; L[High] > Thresh ; High-- )
	    ;

	for ( i = Middle ; i <= High ; )
	{
	    if ( L[i] < Thresh )
	    {
		Temp = L[Middle];
		L[Middle] = L[i];
		L[i] = Temp;
		Middle++;
		i++;
	    }
	    else
	    if ( L[i] > Thresh )
	    {
		Temp = L[High];
		L[High] = L[i];
		L[i] = Temp;
		High--;
	    }
	    else
	    {
		i++;
	    }
	}

	/*  Sort the first and third groups  */

	ListSort(L, Fp, Middle-1);
	ListSort(L, High+1, Lp);
    }
}



/*************************************************************************/
/*								  	 */
/*	Decide whether the given rule duplicates rule r		  	 */
/*								  	 */
/*************************************************************************/


Boolean SameRule(RuleNo r, Condition Cond[], int NConds, ClassNo TargetClass)
/*      --------  */
{
    int	d, i, Bytes;

    if ( Rule[r]->Size != NConds || Rule[r]->Rhs != TargetClass )
    {
	return false;
    }

    ForEach(d, 1, NConds)
    {
	if ( Rule[r]->Lhs[d]->NodeType != Cond[d]->NodeType ||
	     Rule[r]->Lhs[d]->Tested   != Cond[d]->Tested )
	{
	    return false;
	}

	switch ( Cond[d]->NodeType )
	{
	    case BrDiscr:
		if ( Rule[r]->Lhs[d]->TestValue != Cond[d]->TestValue )
		{
		    return false;
		}
		break;

	    case BrThresh:
		if ( Rule[r]->Lhs[d]->TestValue != Cond[d]->TestValue ||
		     Rule[r]->Lhs[d]->Cut != Cond[d]->Cut )
		{
		    return false;
		}
		break;

	    case BrSubset:
		Bytes = (MaxAttVal[Cond[d]->Tested]>>3) + 1;
		ForEach(i, 0, Bytes-1)
		{
		    if ( Rule[r]->Lhs[d]->Subset[i] != Cond[d]->Subset[i] )
		    {
			return false;
		    }
		}
	}
    }

    return true;
}



/*************************************************************************/
/*								  	 */
/*	Free space occupied by a rule and a ruleset			 */
/*								  	 */
/*************************************************************************/


void FreeRule(CRule R)
/*   --------  */
{
    int	d;

    ForEach(d, 1, R->Size)
    {
	if ( R->Lhs[d]->NodeType == BrSubset )
	{
	    FreeUnlessNil(R->Lhs[d]->Subset);
	}
	FreeUnlessNil(R->Lhs[d]);
    }
    FreeUnlessNil(R->Lhs);
    FreeUnlessNil(R);
}



void FreeRules(CRuleSet RS)
/*   ---------  */
{
    int	ri;

    ForEach(ri, 1, RS->SNRules)
    {
	FreeRule(RS->SRule[ri]);
    }
    Free(RS->SRule);
    FreeRuleTree(RS->RT);
    Free(RS);
}



/*************************************************************************/
/*								  	 */
/*	Print a ruleset							 */
/*								  	 */
/*************************************************************************/


void PrintRules(CRuleSet RS, String Msg)
/*   ----------  */
{
    int	r;

    fprintf(Of, "\n%s\n", Msg);

    ForEach(r, 1, RS->SNRules)
    {
	PrintRule(RS->SRule[r]);
    }
}



/*************************************************************************/
/*								  	 */
/*	Print rule R						  	 */
/*								  	 */
/*************************************************************************/


void PrintRule(CRule R)
/*   ---------  */
{
    int		d;

    fprintf(Of, T_RuleHeader);
    if ( TRIALS > 1 ) fprintf(Of, "%d/", R->TNo);
    fprintf(Of, "%d: (%.8g", R->RNo, P1(R->Cover));
    if ( R->Correct < R->Cover - 0.1 )
    {
	fprintf(Of, "/%.8g", P1(R->Cover - R->Correct));
    }
    fprintf(Of, T_RuleLift, ((R->Correct + 1) / (R->Cover + 2)) / R->Prior);

    ForEach(d, 1, R->Size)
    {
	PrintCondition(R->Lhs[d]);
    }

    fprintf(Of, "\t->  " T_class " %s  [%.3f]\n",
		ClassName[R->Rhs], R->Vote/1000.0);
}



/*************************************************************************/
/*								  	 */
/*	Print a condition C of a rule				  	 */
/*								  	 */
/*************************************************************************/


void PrintCondition(Condition C)
/*  --------------  */
{
    DiscrValue	v, pv, Last, Values;
    Boolean	First=true;
    Attribute	Att;
    int		Col, Base, Entry;
    char	CVS[20];

    v   = C->TestValue;
    Att = C->Tested;

    fprintf(Of, "\t%s", AttName[Att]);

    if ( v < 0 )
    {
	fprintf(Of, T_IsUnknown);
	return;
    }

    switch ( C->NodeType )
    {
	case BrDiscr:
	    fprintf(Of, " = %s\n", AttValName[Att][v]);
	    break;

	case BrThresh:
	    if ( v == 1 )
	    {
		fprintf(Of, " = N/A\n");
	    }
	    else
	    {
		CValToStr(C->Cut, Att, CVS);
		fprintf(Of, " %s %s\n", ( v == 2 ? "<=" : ">" ), CVS);
	    }
	    break;

	case BrSubset:
	    /*  Count values at this branch  */

	    Values = Elements(Att, C->Subset, &Last);
	    if ( Values == 1 )
	    {
		fprintf(Of, " = %s\n", AttValName[Att][Last]);
		break;
	    }

	    if ( Ordered(Att) )
	    {
		/*  Find first value  */

		for ( pv = 1 ; ! In(pv, C->Subset) ; pv++ )
		    ;

		fprintf(Of, " %s [%s-%s]\n", T_InRange,
			AttValName[Att][pv], AttValName[Att][Last]);
		break;
	    }

	    /*  Must keep track of position to break long lines  */

	    fprintf(Of, " %s {", T_ElementOf);
	    Col = Base = CharWidth(AttName[Att]) + CharWidth(T_ElementOf) + 11;

	    ForEach(pv, 1, MaxAttVal[Att])
	    {
		if ( In(pv, C->Subset) )
		{
		    Entry = CharWidth(AttValName[Att][pv]);

		    if ( First )
		    {
			First = false;
		    }
		    else
		    if ( Col + Entry + 2 >= Width )
		    {
			Col = Base;
			fprintf(Of, ",\n%*s", Col, "");
		    }
		    else
		    {
			fprintf(Of, ", ");
			Col += 2;
		    }

		    fprintf(Of, "%s", AttValName[Att][pv]);
		    Col += Entry;
		}
	    }
	    fprintf(Of, "}\n");
    }
}
