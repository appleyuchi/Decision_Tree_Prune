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
/*	Routines for saving and reading model files			 */
/*	-------------------------------------------			 */
/*									 */
/*************************************************************************/


#include "defns.i"
#include "extern.i"

int	Entry;

char*	Prop[]={"null",
		"att",
		"class",
		"cut",
		"conds",
		"elts",
		"entries",
		"forks",
		"freq",
		"id",
		"type",
		"low",
		"mid",
		"high",
		"result",
		"rules",
		"val",
		"lift",
		"cover",
		"ok",
		"default",
		"costs",
		"sample",
		"init"
	       };

char	PropName[20],
	*PropVal=Nil,
	*Unquoted;
int	PropValSize=0;

#define	PROPS 23

#define ERRORP		0
#define ATTP		1
#define CLASSP		2
#define CUTP		3
#define	CONDSP		4
#define ELTSP		5
#define ENTRIESP	6
#define FORKSP		7
#define FREQP		8
#define IDP		9
#define TYPEP		10
#define LOWP		11
#define MIDP		12
#define HIGHP		13
#define RESULTP		14
#define RULESP		15
#define VALP		16
#define LIFTP		17
#define COVERP		18
#define OKP		19
#define DEFAULTP	20
#define COSTSP		21
#define SAMPLEP		22
#define INITP		23


/*************************************************************************/
/*									 */
/*	Check whether file is open.  If it is not, open it and		 */
/*	read/write sampling information and discrete names		 */
/*									 */
/*************************************************************************/


void CheckFile(String Extension, Boolean Write)
/*   ---------  */
{
    static char	*LastExt="";

    if ( ! TRf || strcmp(LastExt, Extension) )
    {
	LastExt = Extension;

	if ( TRf )
	{
	    fprintf(TRf, "\n");
	    fclose(TRf);
	}

	if ( Write )
	{
	    WriteFilePrefix(Extension);
	}
	else
	{
	    ReadFilePrefix(Extension);
	}
    }
}



/*************************************************************************/
/*									 */
/*	Write information on system, sampling				 */
/*									 */
/*************************************************************************/


void WriteFilePrefix(String Extension)
/*   ---------------  */
{
    time_t	clock;
    struct tm	*now;

    if ( ! (TRf = GetFile(Extension, "w")) )
    {
	Error(NOFILE, Fn, E_ForWrite);
    }

    clock = time(0);
    now = localtime(&clock);
    now->tm_mon++;
    fprintf(TRf, "id=\"See5/C5.0 %s %d-%d%d-%d%d\"\n",
	    RELEASE,
	    now->tm_year + 1900,
	    now->tm_mon / 10, now->tm_mon % 10,
	    now->tm_mday / 10, now->tm_mday % 10);

    if ( MCost )
    {
	fprintf(TRf, "costs=\"1\"\n");
    }

    if ( SAMPLE > 0 )
    {
	fprintf(TRf, "sample=\"%g\" init=\"%d\"\n", SAMPLE, KRInit);
    }

    SaveDiscreteNames();

    fprintf(TRf, "entries=\"%d\"\n", TRIALS);
}



/*************************************************************************/
/*									 */
/*	Read header information						 */
/*									 */
/*************************************************************************/


void ReadFilePrefix(String Extension)
/*   --------------  */
{
    if ( ! (TRf = GetFile(Extension, "r")) ) Error(NOFILE, Fn, "");

    StreamIn((char *) &TRIALS, sizeof(int));
    if ( memcmp((char *) &TRIALS, "id=", 3) != 0 )
    {
	printf("\nCannot read old format classifiers\n");
	exit(1);
    }
    else
    {
	rewind(TRf);
	ReadHeader();
    }
}



/*************************************************************************/
/*									 */
/*	Save attribute values read with "discrete N"			 */
/*									 */
/*************************************************************************/


void SaveDiscreteNames()
/*   -----------------  */
{
    Attribute	Att;
    DiscrValue	v;

    ForEach(Att, 1, MaxAtt)
    {
	if ( ! StatBit(Att, DISCRETE) || MaxAttVal[Att] < 2 ) continue;

	AsciiOut("att=", AttName[Att]);
	AsciiOut(" elts=", AttValName[Att][2]); 	/* skip N/A */

	ForEach(v, 3, MaxAttVal[Att])
	{
	    AsciiOut(",", AttValName[Att][v]);
	}
	fprintf(TRf, "\n");
    }
}



/*************************************************************************/
/*									 */
/*	Save entire decision tree T in file with extension Extension	 */
/*									 */
/*************************************************************************/


void SaveTree(Tree T, String Extension)
/*   --------  */
{
    CheckFile(Extension, true);

    OutTree(T);
}



void OutTree(Tree T)
/*   -------  */
{
    DiscrValue	v, vv;
    ClassNo	c;
    Boolean	First;

    fprintf(TRf, "type=\"%d\"", T->NodeType);
    AsciiOut(" class=", ClassName[T->Leaf]);
    if ( T->Cases > 0 )
    {
	fprintf(TRf, " freq=\"%g", T->ClassDist[1]);
	ForEach(c, 2, MaxClass)
	{
	    fprintf(TRf, ",%g", T->ClassDist[c]);
	}
	fprintf(TRf, "\"");
    }

    if ( T->NodeType )
    {
	AsciiOut(" att=", AttName[T->Tested]);
	fprintf(TRf, " forks=\"%d\"", T->Forks);

	switch ( T->NodeType )
	{
	    case BrDiscr:
		break;

	    case BrThresh:
		fprintf(TRf, " cut=\"%.*g\"", PREC+1, T->Cut);
		if ( T->Upper > T->Cut )
		{
		    fprintf(TRf, " low=\"%.*g\" mid=\"%.*g\" high=\"%.*g\"",
				 PREC, T->Lower, PREC, T->Mid, PREC, T->Upper);
		}
		break;

	    case BrSubset:
		ForEach(v, 1, T->Forks)
		{
		    First=true;
		    ForEach(vv, 1, MaxAttVal[T->Tested])
		    {
			if ( In(vv, T->Subset[v]) )
			{
			    if ( First )
			    {
				AsciiOut(" elts=", AttValName[T->Tested][vv]);
				First = false;
			    }
			    else
			    {
				AsciiOut(",", AttValName[T->Tested][vv]);
			    }
			}
		    }
		    /*  Make sure have printed at least one element  */

		    if ( First ) AsciiOut(" elts=", "N/A");
		}
		break;
	}
	fprintf(TRf, "\n");

	ForEach(v, 1, T->Forks)
	{
	    OutTree(T->Branch[v]);
	}
    }
    else
    {
	fprintf(TRf, "\n");
    }
}



/*************************************************************************/
/*								  	 */
/*	Save the current ruleset in rules file				 */
/*								  	 */
/*************************************************************************/


void SaveRules(CRuleSet RS, String Extension)
/*   ---------  */
{
    int		ri, d;
    CRule	R;
    Condition	C;
    DiscrValue	v;
    Boolean	First;

    CheckFile(Extension, true);

    fprintf(TRf, "rules=\"%d\"", RS->SNRules);
    AsciiOut(" default=", ClassName[RS->SDefault]);
    fprintf(TRf, "\n");

    ForEach(ri, 1, RS->SNRules)
    {
	R = RS->SRule[ri];
	fprintf(TRf, "conds=\"%d\" cover=\"%g\" ok=\"%g\" lift=\"%g\"",
		     R->Size, R->Cover, R->Correct,
		     (R->Correct + 1) / ((R->Cover + 2) * R->Prior));
	AsciiOut(" class=", ClassName[R->Rhs]);
	fprintf(TRf, "\n");

	ForEach(d, 1, R->Size)
	{
	    C = R->Lhs[d];

	    fprintf(TRf, "type=\"%d\"", C->NodeType);
	    AsciiOut(" att=", AttName[C->Tested]);

	    switch ( C->NodeType )
	    {
		case BrDiscr:
		    AsciiOut(" val=", AttValName[C->Tested][C->TestValue]);
		    break;

		case BrThresh:
		    if ( C->TestValue == 1 )	/* N/A */
		    {
			fprintf(TRf, " val=\"N/A\"");
		    }
		    else
		    {
			fprintf(TRf, " cut=\"%.*g\" result=\"%c\"",
				     PREC+1, C->Cut,
				     ( C->TestValue == 2 ? '<' : '>' ));
		    }
		    break;

		case BrSubset:
		    First=true;
		    ForEach(v, 1, MaxAttVal[C->Tested])
		    {
			if ( In(v, C->Subset) )
			{
			    if ( First )
			    {
				AsciiOut(" elts=", AttValName[C->Tested][v]);
				First = false;
			    }
			    else
			    {
				AsciiOut(",", AttValName[C->Tested][v]);
			    }
			}
		    }
		    break;
	    }

	    fprintf(TRf, "\n");
	}
    }
}



/*************************************************************************/
/*									 */
/*	Write ASCII string with prefix, escaping any quotes		 */
/*									 */
/*************************************************************************/


void AsciiOut(String Pre, String S)
/*   --------  */
{
    fprintf(TRf, "%s\"", Pre);
    while ( *S )
    {
	if ( *S == '"' || *S == '\\' ) fputc('\\', TRf);
	fputc(*S++, TRf);
    }
    fputc('"', TRf);
}



/*************************************************************************/
/*								  	 */
/*	Read the header information (id, saved names, models)		 */
/*								  	 */
/*************************************************************************/


void ReadHeader()
/*   ---------  */
{
    Attribute	Att;
    DiscrValue	v;
    char	*p, Dummy;
    int		Year, Month, Day;
    FILE	*F;

    while ( true )
    {
	switch ( ReadProp(&Dummy) )
	{
	    case ERRORP:
		return;

	    case IDP:
		/*  Recover year run and set base date for timestamps  */

		if ( sscanf(PropVal + strlen(PropVal) - 11,
			    "%d-%d-%d\"", &Year, &Month, &Day) == 3 )
		{
		    SetTSBase(Year);
		}
		break;

	    case COSTSP:
		/*  Recover costs file used to generate model  */

		if ( (F = GetFile(".costs", "r")) )
		{
		    GetMCosts(F);
		}
		break;
	    case SAMPLEP:
		sscanf(PropVal, "\"%f\"", &SAMPLE);
		break;

	    case INITP:
		sscanf(PropVal, "\"%d\"", &KRInit);
		break;

	    case ATTP:
		Unquoted = RemoveQuotes(PropVal);
		Att = Which(Unquoted, AttName, 1, MaxAtt);
		if ( ! Att || Exclude(Att) )
		{
		    Error(MODELFILE, E_MFATT, Unquoted);
		}
		break;

	    case ELTSP:
		MaxAttVal[Att] = 1;
		AttValName[Att][1] = strdup("N/A");

		for ( p = PropVal ; *p ; )
		{
		    p = RemoveQuotes(p);
		    v = ++MaxAttVal[Att];
		    AttValName[Att][v] = strdup(p);

		    for ( p += strlen(p) ; *p != '"' ; p++ )
			;
		    p++;
		    if ( *p == ',' ) p++;
		}
		AttValName[Att][MaxAttVal[Att]+1] = "<other>";
		MaxDiscrVal = Max(MaxDiscrVal, MaxAttVal[Att]+1);
		break;

	    case ENTRIESP:
		sscanf(PropVal, "\"%d\"", &TRIALS);
		Entry = 0;
		return;
	}
    }
}



/*************************************************************************/
/*									 */
/*	Retrieve decision tree with extension Extension			 */
/*									 */
/*************************************************************************/


Tree GetTree(String Extension)
/*   -------  */
{
    CheckFile(Extension, false);

    return InTree();
}



Tree InTree()
/*   ------  */
{
    Tree	T;
    DiscrValue	v, Subset=0;
    char	Delim, *p;
    ClassNo	c;
    int		X;
    double	XD;

    T = (Tree) AllocZero(1, TreeRec);

    do
    {
	switch ( ReadProp(&Delim) )
	{
	    case ERRORP:
		return Nil;

	    case TYPEP:
		sscanf(PropVal, "\"%d\"", &X); T->NodeType = X;
		break;

	    case CLASSP:
		Unquoted = RemoveQuotes(PropVal);
		T->Leaf = Which(Unquoted, ClassName, 1, MaxClass);
		if ( ! T->Leaf ) Error(MODELFILE, E_MFCLASS, Unquoted);
		break;

	    case ATTP:
		Unquoted = RemoveQuotes(PropVal);
		T->Tested = Which(Unquoted, AttName, 1, MaxAtt);
		if ( ! T->Tested || Exclude(T->Tested) )
		{
		    Error(MODELFILE, E_MFATT, Unquoted);
		}
		break;

	    case CUTP:
		sscanf(PropVal, "\"%lf\"", &XD);	T->Cut = XD;
		T->Lower = T->Mid = T->Upper = T->Cut;
		break;

	    case LOWP:
		sscanf(PropVal, "\"%lf\"", &XD);	T->Lower = XD;
		break;

	    case MIDP:
		sscanf(PropVal, "\"%lf\"", &XD);	T->Mid = XD;
		break;

	    case HIGHP:
		sscanf(PropVal, "\"%lf\"", &XD);	T->Upper = XD;
		break;

	    case FORKSP:
		sscanf(PropVal, "\"%d\"", &T->Forks);
		break;

	    case FREQP:
		T->ClassDist = Alloc(MaxClass+1, CaseCount);
		p = PropVal+1;

		ForEach(c, 1, MaxClass)
		{
		    T->ClassDist[c] = strtod(p, &p);
		    T->Cases += T->ClassDist[c];
		    p++;
		}
		break;

	    case ELTSP:
		if ( ! Subset++ )
		{
		    T->Subset = AllocZero(T->Forks+1, Set);
		}

		T->Subset[Subset] = MakeSubset(T->Tested);
		break;
	}
    }
    while ( Delim == ' ' );

    if ( T->ClassDist )
    {
	T->Errors = T->Cases - T->ClassDist[T->Leaf];
    }
    else
    {
	T->ClassDist = Alloc(1, CaseCount);
    }

    if ( T->NodeType )
    {
	T->Branch = AllocZero(T->Forks+1, Tree);
	ForEach(v, 1, T->Forks)
	{
	    T->Branch[v] = InTree();
	}
    }

    return T;
}



/*************************************************************************/
/*									 */
/*	Retrieve ruleset with extension Extension			 */
/*	(Separate functions for ruleset, single rule, single condition)	 */
/*									 */
/*************************************************************************/


CRuleSet GetRules(String Extension)
/*	 --------  */
{
    CheckFile(Extension, false);

    return InRules();
}



CRuleSet InRules()
/*	 -------  */
{
    CRuleSet	RS;
    RuleNo	r;
    char	Delim;

    RS = Alloc(1, RuleSetRec);

    do
    {
	switch ( ReadProp(&Delim) )
	{
	    case ERRORP:
		return Nil;

	    case RULESP:
		sscanf(PropVal, "\"%d\"", &RS->SNRules);
		CheckActiveSpace(RS->SNRules);
		break;

	    case DEFAULTP:
		Unquoted = RemoveQuotes(PropVal);
		RS->SDefault = Which(Unquoted, ClassName, 1, MaxClass);
		if ( ! RS->SDefault ) Error(MODELFILE, E_MFCLASS, Unquoted);
		break;
	}
    }
    while ( Delim == ' ' );

    /*  Read each rule  */

    RS->SRule = Alloc(RS->SNRules+1, CRule);
    ForEach(r, 1, RS->SNRules)
    {
	if ( (RS->SRule[r] = InRule()) )
	{
	    RS->SRule[r]->RNo = r;
	    RS->SRule[r]->TNo = Entry;
	}
    }
    ConstructRuleTree(RS);
    Entry++;
    return RS;
}



CRule InRule()
/*    ------  */
{
    CRule	R;
    int		d;
    char	Delim;
    float	Lift;

    R = Alloc(1, RuleRec);

    do
    {
	switch ( ReadProp(&Delim) )
	{
	    case ERRORP:
		return Nil;

	    case CONDSP:
		sscanf(PropVal, "\"%d\"", &R->Size);
		break;

	    case COVERP:
		sscanf(PropVal, "\"%f\"", &R->Cover);
		break;

	    case OKP:
		sscanf(PropVal, "\"%f\"", &R->Correct);
		break;

	    case LIFTP:
		sscanf(PropVal, "\"%f\"", &Lift);
		R->Prior = (R->Correct + 1) / ((R->Cover + 2) * Lift);
		break;

	    case CLASSP:
		Unquoted = RemoveQuotes(PropVal);
		R->Rhs = Which(Unquoted, ClassName, 1, MaxClass);
		if ( ! R->Rhs ) Error(MODELFILE, E_MFCLASS, Unquoted);
		break;
	}
    }
    while ( Delim == ' ' );

    R->Lhs = Alloc(R->Size+1, Condition);
    ForEach(d, 1, R->Size)
    {
	R->Lhs[d] = InCondition();
    }

    R->Vote = 1000 * (R->Correct + 1.0) / (R->Cover + 2.0) + 0.5;

    return R;
}



Condition InCondition()
/*        -----------  */
{
    Condition	C;
    char	Delim;
    int		X;
    double	XD;

    C = Alloc(1, CondRec);

    do
    {
	switch ( ReadProp(&Delim) )
	{
	    case ERRORP:
		return Nil;

	    case TYPEP:
		sscanf(PropVal, "\"%d\"", &X); C->NodeType = X;
		break;

	    case ATTP:
		Unquoted = RemoveQuotes(PropVal);
		C->Tested = Which(Unquoted, AttName, 1, MaxAtt);
		if ( ! C->Tested || Exclude(C->Tested) )
		{
		    Error(MODELFILE, E_MFATT, Unquoted);
		}
		break;

	    case CUTP:
		sscanf(PropVal, "\"%lf\"", &XD);	C->Cut = XD;
		break;

	    case RESULTP:
		C->TestValue = ( PropVal[1] == '<' ? 2 : 3 );
		break;

	    case VALP:
		if ( Continuous(C->Tested) )
		{
		    C->TestValue = 1;
		}
		else
		{
		    Unquoted = RemoveQuotes(PropVal);
		    C->TestValue = Which(Unquoted,
					 AttValName[C->Tested],
					 1, MaxAttVal[C->Tested]);
		    if ( ! C->TestValue ) Error(MODELFILE, E_MFATTVAL, Unquoted);
		}
		break;

	    case ELTSP:
		C->Subset = MakeSubset(C->Tested);
		C->TestValue = 1;
		break;
	}
    }
    while ( Delim == ' ' );

    return C;
}



/*************************************************************************/
/*									 */
/*	ASCII reading utilities						 */
/*									 */
/*************************************************************************/


int ReadProp(char *Delim)
/*  --------  */
{
    int		c, i;
    char	*p;
    Boolean	Quote=false;

    for ( p = PropName ; (c = fgetc(TRf)) != '=' ;  )
    {
	if ( p - PropName >= 19 || c == EOF )
	{
	    Error(MODELFILE, E_MFEOF, "");
	    PropName[0] = PropVal[0] = *Delim = '\00';
	    return 0;
	}
	*p++ = c;
    }
    *p = '\00';

    for ( p = PropVal ; ((c = fgetc(TRf)) != ' ' && c != '\n') || Quote ; )
    {
	if ( c == EOF )
	{
	    Error(MODELFILE, E_MFEOF, "");
	    PropName[0] = PropVal[0] = '\00';
	    return 0;
	}

	if ( (i = p - PropVal) >= PropValSize )
	{
	    Realloc(PropVal, (PropValSize += 10000) + 3, char);
	    p = PropVal + i;
	}

	*p++ = c;
	if ( c == '\\' )
	{
	    *p++ = fgetc(TRf);
	}
	else
	if ( c == '"' )
	{
	    Quote = ! Quote;
	}
    }
    *p = '\00';
    *Delim = c;

    return Which(PropName, Prop, 1, PROPS);
}


String RemoveQuotes(String S)
/*     ------------  */
{
    char	*p, *Start;

    p = Start = S;
    
    for ( S++ ; *S != '"' ; S++ )
    {
	if ( *S == '\\' ) S++;
	*p++ = *S;
	*S = '-';
    }
    *p = '\00';

    return Start;
}



Set MakeSubset(Attribute Att)
/*  ----------  */
{
    int		Bytes, b;
    char	*p;
    Set		S;

    Bytes = (MaxAttVal[Att]>>3) + 1;
    S = AllocZero(Bytes, Byte);

    for ( p = PropVal ; *p ; )
    {
	p = RemoveQuotes(p);
	b = Which(p, AttValName[Att], 1, MaxAttVal[Att]);
	if ( ! b ) Error(MODELFILE, E_MFATTVAL, p);
	SetBit(b, S);

	for ( p += strlen(p) ; *p != '"' ; p++ )
	    ;
	p++;
	if ( *p == ',' ) p++;
    }

    return S;
}



/*************************************************************************/
/*								  	 */
/*	Character stream read for binary routines			 */
/*								  	 */
/*************************************************************************/


void StreamIn(String S, int n)
/*   --------  */
{
    while ( n-- ) *S++ = getc(TRf);
}
