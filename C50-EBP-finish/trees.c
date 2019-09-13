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
/*	Routines for displaying, building, saving and restoring trees	 */
/*	-------------------------------------------------------------	 */
/*									 */
/*************************************************************************/


#include "defns.i"
#include "extern.i"


#define	TabSize		4
#define Utility		ClassDist[0]
#define Digits(n)	((n) < 10 ? 3 : (int)(3 + log(n-1) / log(10.0)))


	/*  If lines look like getting too long while a tree is being
	    printed, subtrees are broken off and printed separately after
	    the main tree is finished	 */

int	SubTree,		/* highest subtree to be printed */
	SubSpace=0;		/* maximum subtree encountered */
Tree	*SubDef=Nil;		/* pointers to subtrees */
Boolean	LastBranch[Width];	/* whether printing last branch of subtree */



/*************************************************************************/
/*									 */
/*	Calculate the depth of nodes in a tree in Utility field		 */
/*									 */
/*************************************************************************/


void FindDepth(Tree T)
/*   ---------  */
{
    float	MaxDepth=0;
    DiscrValue	v;

    if ( T->NodeType )
    {
	ForEach(v, 1, T->Forks)
	{
	    FindDepth(T->Branch[v]);
	    if ( T->Branch[v]->Utility > MaxDepth )
	    {
		MaxDepth = T->Branch[v]->Utility;
	    }
	}
    }

    T->Utility = MaxDepth + 1;
}



/*************************************************************************/
/*									 */
/*	Display entire decision tree T					 */
/*									 */
/*************************************************************************/


void PrintTree(Tree T, String Title)
/*   ---------  */
{
    int s;

    FindDepth(T);

    SubTree=0;
    fprintf(Of, "\n%s\n", Title);
    Show(T, 0);
    fprintf(Of, "\n");

    ForEach(s, 1, SubTree)
    {
	fprintf(Of, T_Subtree, s);
	Show(SubDef[s], 0);
	fprintf(Of, "\n");
    }
}



/*************************************************************************/
/*									 */
/*	Display the tree T with offset Sh				 */
/*									 */
/*************************************************************************/


void Show(Tree T, int Sh)
/*   ---- */
{
    DiscrValue	v, MaxV, BrNo, Simplest, First;
    CaseCount	Errors=0.0;

    if ( T->NodeType )
    {
	/*  See whether separate subtree needed  */

	if ( Sh && Sh * TabSize + MaxLine(T) > Width )
	{
	    if ( ++SubTree >= SubSpace )
	    {
		SubSpace += 100;
		if ( SubDef )
		{
		    Realloc(SubDef, SubSpace, Tree);
		}
		else
		{
		    SubDef = Alloc(SubSpace, Tree);
		}
	    }

	    SubDef[SubTree] = T;
	    fprintf(Of, " [S%d]", SubTree);
	}
	else
	{
	    MaxV = T->Forks;

	    /*  Skip N/A branch if no cases  */

	    First = ( EmptyNA(T) ? 2 : 1 );
	    BrNo = First - 1;

	    /*  Print simplest branches first  */

	    while ( BrNo < MaxV )
	    {
		Simplest = First;
		ForEach(v, 2, MaxV)
		{
		    if ( T->Branch[v]->Utility < T->Branch[Simplest]->Utility ||

			 T->Branch[v]->Utility == 1 && ! T->Branch[v]->Cases )
		    {
			Simplest = v;
		    }
		}

		LastBranch[Sh+1] = ( ++BrNo == MaxV );
		ShowBranch(Sh, T, Simplest, (int)( BrNo == First ));
		T->Branch[Simplest]->Utility = 1E10;
	    }
	}
    }
    else
    {
	fprintf(Of, " %s (%.8g", ClassName[T->Leaf], P1(T->Cases));
	if ( T->Cases >= MinLeaf )
	{
	    if ( (Errors = T->Cases - T->ClassDist[T->Leaf]) >= 0.05 )
	    {
		fprintf(Of, "/%.8g", P1(Errors));
	    }
	}
	putc(')', Of);
    }
}



/*************************************************************************/
/*									 */
/*	Print a node T with offset Sh, branch value v, and continue	 */
/*									 */
/*************************************************************************/


void ShowBranch(int Sh, Tree T, DiscrValue v, DiscrValue BrNo)
/*   ----------  */
{
    DiscrValue	Pv, Last;
    Attribute	Att;
    Boolean	FirstValue;
    int		TextWidth, Skip, Values, i, Extra;
    char	CVS1[20], CVS2[20];

    Att = T->Tested;

    switch ( T->NodeType )
    {
	case BrDiscr:

	    Indent(Sh, BrNo);

	    fprintf(Of, "%s = %s:", AttName[Att], AttValName[Att][v]);

	    break;

	case BrThresh:

	    Indent(Sh, BrNo);

	    fprintf(Of, "%s", AttName[Att]);

	    if ( v == 1 )
	    {
		fprintf(Of, " = N/A:");
	    }
	    else
	    if ( T->Lower != T->Upper )
	    {
		if ( v == 2 )
		{
		    CValToStr(T->Lower, Att, CVS1);
		    CValToStr(T->Mid  , Att, CVS2);
		    fprintf(Of, " <= %s (%s):", CVS1, CVS2);
		}
		else
		{
		    CValToStr(T->Upper, Att, CVS1);
		    CValToStr(T->Mid  , Att, CVS2);
		    fprintf(Of, " >= %s (%s):", CVS1, CVS2);
		}
	    }
	    else
	    {
		CValToStr(T->Cut, Att, CVS1);
		fprintf(Of, " %s %s:", ( v == 2 ? "<=" : ">" ), CVS1);
	    }

	    break;

	case BrSubset:

	    /*  Count values at this branch  */

	    Values = Elements(Att, T->Subset[v], &Last);
	    if ( ! Values ) return;

	    Indent(Sh, BrNo);

	    if ( Values == 1 )
	    {
		fprintf(Of, "%s = %s:", AttName[Att], AttValName[Att][Last]);
		break;
	    }

	    if ( Ordered(Att) )
	    {
		/*  Find first value  */

		for ( Pv = 1 ; ! In(Pv, T->Subset[v]) ; Pv++ )
		    ;

		fprintf(Of, "%s %s [%s-%s]:", AttName[Att], T_InRange,
			AttValName[Att][Pv], AttValName[Att][Last]);
		break;
	    }

	    fprintf(Of, "%s %s {", AttName[Att], T_ElementOf);
	    FirstValue = true;
	    Skip = CharWidth(AttName[Att]) + CharWidth(T_ElementOf) + 3;
	    TextWidth = Skip + Sh * TabSize;

	    ForEach(Pv, 1, Last)
	    {
		if ( In(Pv, T->Subset[v]) )
		{
		    /*  Find number of characters after this element  */

		    if ( Pv != Last || T->Branch[v]->NodeType )
		    {
			Extra = 1;				/* for ":" */
		    }
		    else
		    {
			Extra = 2				/* for ": " */
				+ CharWidth(ClassName[T->Branch[v]->Leaf])
				+ 3				/* for " ()" */
				+ Digits(T->Cases)
				+ ( T->Errors < 0.05 ?  0 :
						1		/* for "/" */
						+ Digits(T->Errors) );
		    }

		    if ( ! FirstValue &&
			 TextWidth + CharWidth(AttValName[Att][Pv]) +
			     Extra + 1 > Width )
		    {
		  	Indent(Sh, 0);
			fprintf(Of, "%s",
				( LastBranch[Sh+1] && ! T->Branch[v]->NodeType ?
				  "    " : ":   " ));
			ForEach(i, 5, Skip) putc(' ', Of);

			TextWidth = Skip + Sh * TabSize;
			FirstValue = true;
		    }

		    fprintf(Of, "%s%c",
				AttValName[Att][Pv], Pv == Last ? '}' : ',');
		    TextWidth += CharWidth(AttValName[Att][Pv]) + 1;
		    FirstValue = false;
		}
	    }
	    putc(':', Of);
    }

    Show(T->Branch[v], Sh+1);
}



/*************************************************************************/
/*									 */
/*	Count the elements in a subset and record the last		 */
/*									 */
/*************************************************************************/


DiscrValue Elements(Attribute Att, Set S, DiscrValue *Last)
/*         --------  */
{
    DiscrValue Pv, Values=0;

    ForEach(Pv, 1, MaxAttVal[Att])
    {
	if ( In(Pv, S) )
	{
	    *Last = Pv;
	    Values++;
	}
    }

    return Values;
}



/*************************************************************************/
/*									 */
/*	Find the approximate maximum single line size for non-leaf	 */
/*	subtree T							 */
/*									 */
/*************************************************************************/


int MaxLine(Tree T)
/*  -------  */
{
    Attribute	Att;
    DiscrValue	v, vv;
    int		Ll, One, MaxLl=0;

    Att = T->Tested;

    /*  First find the max length of the line excluding tested att  */

    ForEach(v, 1, T->Forks)
    {
	switch ( T->NodeType )
	{
	    case BrThresh:
		if ( TStampVal(Att) )
		{
		    Ll = ( T->Lower != T->Upper ? 41 : 19 );
		}
		else
		if ( DateVal(Att) )
		{
		    Ll = ( T->Lower != T->Upper ? 23 : 10 );
		}
		else
		if ( TimeVal(Att) )
		{
		    Ll = ( T->Lower != T->Upper ? 19 : 8 );
		}
		else
		{
		    Ll = ( T->Lower != T->Upper ? 11 : 4 );
		}
		break;

	    case BrDiscr:
		if ( Ordered(Att) )
		{
		    vv = T->Cut;
	
		    switch ( v )
		    {
			case 1:
			    Ll = 3;
			    break;

			case 2:
			    Ll = CharWidth(AttValName[Att][2]);
			    if ( vv != 2 )
			    {
				Ll += CharWidth(AttValName[Att][vv])+1;
			    }
			    break;

			case 3:
			    Ll = CharWidth(AttValName[Att][MaxAttVal[Att]]);
			    if ( vv != MaxAttVal[Att] - 1 )
			    {
				Ll += CharWidth(AttValName[Att][vv+1])+1;
			    }
		    }
		}
		else
		{
		    Ll = CharWidth(AttValName[Att][v]) + 1;
		}
		break;

	    case BrSubset: /* difficult! */
		Ll = 0;
		ForEach(vv, 1, MaxAttVal[Att])
		{
		    if ( In(vv,T->Subset[v]) )
		    {
			One = CharWidth(AttValName[Att][vv]) + 6;
			if ( One > Ll ) Ll = One;
		    }
		}
	}

	/*  Check whether ends in leaf  */

	if ( ! T->Branch[v]->NodeType &&
	     ( v > 1 || T->Branch[v]->Cases > 0.01 ) )
	{
	    Ll += CharWidth(ClassName[T->Branch[v]->Leaf]) + 6;
	}

	if ( Ll > MaxLl ) MaxLl = Ll;
    }

    return CharWidth(AttName[Att]) + 4 + MaxLl;
}



/*************************************************************************/
/*								   	 */
/*	Indent Sh columns					  	 */
/*								  	 */
/*************************************************************************/


void Indent(int Sh, int BrNo)
/*   ------  */
{
    int	i;

    fprintf(Of, "\n");
    for ( i = 1 ; i <= Sh ; i++ )
    {
	fprintf(Of, "%s", ( i == Sh && BrNo == 1 ? ":..." :
			    LastBranch[i] ? "    " : ":   " ));
    }
}



/*************************************************************************/
/*									 */
/*	Free up space taken up by tree T				 */
/*									 */
/*************************************************************************/


void FreeTree(Tree T)
/*   --------  */
{
    DiscrValue v;

    if ( ! T ) return;

    if ( T->NodeType )
    {
	ForEach(v, 1, T->Forks)
	{
	    FreeTree(T->Branch[v]);
	}

	Free(T->Branch);

	if ( T->NodeType == BrSubset )
	{
	    FreeVector((void **) T->Subset, 1, T->Forks);
	}

    }

    Free(T->ClassDist);
    Free(T);
}



/*************************************************************************/
/*									 */
/*	Construct a leaf in a given node				 */
/*									 */
/*************************************************************************/


Tree Leaf(double *Freq, ClassNo NodeClass, CaseCount Cases, CaseCount Errors)
/*   ----  */
{
    Tree	Node;
    ClassNo	c;

    Node = AllocZero(1, TreeRec);

    Node->ClassDist = AllocZero(MaxClass+1, CaseCount);
    if ( Freq )
    {
	ForEach(c, 1, MaxClass)
	{
	    Node->ClassDist[c] = Freq[c];
	}
    }

    Node->NodeType	= 0;
    Node->Leaf		= NodeClass;
    Node->Cases		= Cases;
    Node->Errors	= Errors;

    return Node;
}



/*************************************************************************/
/*									 */
/*	Insert branches in a node 		         		 */
/*									 */
/*************************************************************************/


void Sprout(Tree T, DiscrValue Branches)
/*   ------  */
{
    T->Forks = Branches;
    T->Branch = AllocZero(Branches+1, Tree);
}



/*************************************************************************/
/*									 */
/*	Remove branches etc from a node					 */
/*									 */
/*************************************************************************/


void UnSprout(Tree T)
/*   --------  */
{
    DiscrValue	v;

    ForEach(v, 1, T->Forks)
    {
	FreeTree(T->Branch[v]);
    }
    Free(T->Branch);					T->Branch = Nil;

    if ( T->NodeType == BrSubset )
    {
	FreeVector((void **) T->Subset, 1, T->Forks);	T->Subset = Nil;
    }

    T->Forks = T->NodeType = 0;
}



/*************************************************************************/
/*									 */
/*	Count the non-null leaves in a tree				 */
/*									 */
/*************************************************************************/


int TreeSize(Tree T)
/*  --------  */
{
    int		Sum=0;
    DiscrValue	v;

    if ( T->NodeType )
    {
	ForEach(v, ( EmptyNA(T) ? 2 : 1 ), T->Forks)
	{
	    Sum += TreeSize(T->Branch[v]);
	}

	return Sum;
    }

    return ( T->Cases >= MinLeaf ? 1 : 0 );
}



/*************************************************************************/
/*									 */
/*	Count the non-null leaves in a tree that may contain		 */
/*	compressed branches via CompressBranches()			 */
/*									 */
/*************************************************************************/


int ExpandedLeafCount(Tree T)
/*  -----------------  */
{
    int		Sum=0;
    DiscrValue	v, Dummy;

    if ( ! T->NodeType )
    {
	return 1;
    }

    ForEach(v, 1, T->Forks)
    {
	if ( T->Branch[v]->Cases < MinLeaf ) continue;

	if ( T->NodeType == BrSubset && ! T->Branch[v]->NodeType )
	{
	    Sum += Elements(T->Tested, T->Subset[v], &Dummy);
	}
	else
	{
	    Sum += ExpandedLeafCount(T->Branch[v]);
	}
    }

    return Sum;
}



/*************************************************************************/
/*                                                                	 */
/*	Find the maximum depth of a tree				 */
/*                                                                	 */
/*************************************************************************/


int TreeDepth(Tree T)
/*  ---------  */
{
    DiscrValue	v;
    int		Subtree, MaxSubtree=0;

    if ( T->NodeType )
    {
	ForEach(v, 1, T->Forks)
	{
	    Subtree = TreeDepth(T->Branch[v]);
	    if ( Subtree > MaxSubtree ) MaxSubtree = Subtree;
	}
    }

    return MaxSubtree + 1;
}



/*************************************************************************/
/*									 */
/*	Return a copy of tree T						 */
/*									 */
/*************************************************************************/


Tree CopyTree(Tree T)
/*   --------  */
{
    DiscrValue	v;
    Tree	New;
    int		Bytes;

    New = Alloc(1, TreeRec);
    memcpy(New, T, sizeof(TreeRec));

    New->ClassDist = Alloc(MaxClass+1, CaseCount);
    memcpy(New->ClassDist, T->ClassDist, (MaxClass + 1) * sizeof(CaseCount));

    if ( T->NodeType == BrSubset )
    {
	Bytes = (MaxAttVal[T->Tested]>>3) + 1;

	New->Subset = Alloc(T->Forks+1, Set);
	ForEach(v, 1, T->Forks)
	{
	    New->Subset[v] = Alloc(Bytes, unsigned char);
	    memcpy(New->Subset[v], T->Subset[v], Bytes);
	}
    }

    if ( T->NodeType )
    {
	New->Branch = AllocZero(T->Forks+1, Tree);
	ForEach(v, 1, T->Forks)
	{
	    New->Branch[v] = CopyTree(T->Branch[v]);
	}
    }

    return New;
}
