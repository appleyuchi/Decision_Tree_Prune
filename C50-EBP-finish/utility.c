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
/*	Print header for all C5.0 programs				 */
/*	----------------------------------				 */
/*									 */
/*************************************************************************/

#include "defns.i"
#include "extern.i"


#define  NAME T_C50


void PrintHeader(String Title)
/*   -----------  */
{
    char	TitleLine[80];
    time_t	clock;
    int		Underline;

    clock = time(0);
    sprintf(TitleLine, "%s%s [%s]", NAME, Title, TX_Release(RELEASE));
    fprintf(Of, "\n%s  \t%s", TitleLine, ctime(&clock));

    Underline = CharWidth(TitleLine);
    while ( Underline-- ) putc('-', Of);
    putc('\n', Of);
}



/*************************************************************************/
/*									 */
/*	This is a specialised form of the getopt utility.		 */
/*									 */
/*************************************************************************/


String	OptArg, Option;


char ProcessOption(int Argc, char *Argv[], char *Options)
/*   -------------  */
{
    int		i;
    static int	OptNo=1;

    if ( OptNo >= Argc ) return '\00';

    if ( *(Option = Argv[OptNo++]) != '-' ) return '?';

    for ( i = 0 ; Options[i] ; i++ )
    {
	if ( Options[i] == Option[1] )
	{
	    OptArg = (char *) ( Options[i+1] != '+' ? Nil :
				Option[2] ? Option+2 :
				OptNo < Argc ? Argv[OptNo++] : "0" );
	    return Option[1];
	}
    }

    return '?';
}



/*************************************************************************/
/*									 */
/*	Protected memory allocation routines				 */
/*									 */
/*************************************************************************/



void *Pmalloc(size_t Bytes)
/*    -------  */
{
    void *p=Nil;

    if ( ! Bytes || (p = (void *) malloc(Bytes)) )
    {
	return p;
    }

    Error(NOMEM, "", "");

}



void *Prealloc(void *Present, size_t Bytes)
/*    --------  */
{
    void *p=Nil;

    if ( ! Bytes ) return Nil;

    if ( ! Present ) return Pmalloc(Bytes);

    if ( (p = (void *) realloc(Present, Bytes)) )
    {
	return p;
    }

    Error(NOMEM, "", "");

}



void *Pcalloc(size_t Number, unsigned int Size)
/*    -------  */
{
    void *p=Nil;

    if ( ! Number || (p = (void *) calloc(Number, Size)) )
    {
	return p;
    }

    Error(NOMEM, "", "");

}



void FreeVector(void **V, int First, int Last)
/*   ----------  */
{
    if ( V )
    {
	while ( First <= Last )
	{
	    FreeUnlessNil(V[First]);
	    First++;
	}

	Free(V);
    }
}



/*************************************************************************/
/*									 */
/*	Special memory allocation routines for case memory		 */
/*									 */
/*************************************************************************/

typedef struct _datablockrec	*DataBlock;

typedef	struct _datablockrec
	{
	  DataRec	Head;		/* first address */
	  int		Allocated;	/* number of cases in this block */
	  DataBlock	Prev;		/* previous data block */
	}
	DataBlockRec;

DataBlock	DataMem=Nil;
int		DataBlockSize=0;



DataRec NewCase()
/*      -------  */
{
    DataBlock	Prev;

    if ( ! DataMem || DataMem->Allocated == DataBlockSize )
    {
	DataBlockSize = Min(8192, 262144 / (MaxAtt+2) + 1);

	Prev = DataMem;
	DataMem = AllocZero(1, DataBlockRec);
	DataMem->Head = Alloc(DataBlockSize * (MaxAtt+2), AttValue);
	DataMem->Prev = Prev;
    }

    return DataMem->Head + (DataMem->Allocated++) * (MaxAtt+2) + 1;
}



void FreeCases()
/*   ---------  */
{
    DataBlock	Prev;

    while ( DataMem )
    {
	Prev = DataMem->Prev;
	Free(DataMem->Head);
	Free(DataMem);
	DataMem = Prev;
    }
}



void FreeLastCase(DataRec Case)
/*   ------------  */
{
    DataMem->Allocated--;
}



/*************************************************************************/
/*									 */
/*	Generate uniform random numbers					 */
/*									 */
/*************************************************************************/


#define	Modify(F,S)	if ( (F -= S) < 0 ) F += 1.0

int	KRFp=0, KRSp=0;

double KRandom()
/*     -------  */
{
    static double	URD[55];
    double		V1, V2;
    int			i, j;

    /*  Initialisation  */

    if ( KRFp == KRSp )
    {
	KRFp = 0;
	KRSp = 31;

	V1 = 1.0;
	V2 = 0.314159285;

	ForEach(i, 1, 55)
	{
	    URD[ j = (i * 21) % 55 ] = V1;
	    V1 = V2 - V1;
	    if ( V1 < 0 ) V1 += 1.0;
	    V2 = URD[j];
	}

	ForEach(j, 0, 5)
	{
	    ForEach(i, 0, 54)
	    {
		Modify(URD[i], URD[(i+30) % 55]);
	    }
	}
    }

    KRFp = (KRFp + 1) % 55;
    KRSp = (KRSp + 1) % 55;
    Modify(URD[KRFp], URD[KRSp]);

    return URD[KRFp];
}



void ResetKR(int KRInit)
/*   -------  */
{
    KRFp = KRSp = 0;

    KRInit += 1000;
    while ( KRInit-- )
    {
	KRandom();
    }
}



/*************************************************************************/
/*									 */
/*	Error messages							 */
/*									 */
/*************************************************************************/


void Error(int ErrNo, String S1, String S2)
/*   -----  */
{
    Boolean	Quit=false, WarningOnly=false;
    char	Buffer[10000], *Msg=Buffer;


    if ( Of ) fprintf(Of, "\n");

    if ( ErrNo == NOFILE || ErrNo == NOMEM || ErrNo == MODELFILE )
    {
	sprintf(Msg, "*** ");
    }
    else
    {
	sprintf(Msg, TX_Line(LineNo, Fn));
    }
    Msg += strlen(Buffer);

    switch ( ErrNo )
    {
	case NOFILE:
	    sprintf(Msg, E_NOFILE(Fn, S2));
	    Quit = true;
	    break;

	case BADCLASSTHRESH:
	    sprintf(Msg, E_BADCLASSTHRESH, S1);
	    break;

	case LEQCLASSTHRESH:
	    sprintf(Msg, E_LEQCLASSTHRESH, S1);
	    break;

	case BADATTNAME:
	    sprintf(Msg, E_BADATTNAME, S1);
	    break;

	case EOFINATT:
	    sprintf(Msg, E_EOFINATT, S1);
	    break;

	case SINGLEATTVAL:
	    sprintf(Msg, E_SINGLEATTVAL(S1, S2));
	    break;

	case DUPATTNAME:
	    sprintf(Msg, E_DUPATTNAME, S1);
	    break;

	case CWTATTERR:
	    sprintf(Msg, E_CWTATTERR);
	    break;

	case BADATTVAL:
	    sprintf(Msg, E_BADATTVAL(S2, S1));
	    break;

	case BADNUMBER:
	    sprintf(Msg, E_BADNUMBER(S1));
	    break;

	case BADCLASS:
	    sprintf(Msg, E_BADCLASS, S2);
	    break;

	case BADCOSTCLASS:
	    sprintf(Msg, E_BADCOSTCLASS, S1);
	    Quit = true;
	    break;

	case BADCOST:
	    sprintf(Msg, E_BADCOST, S1);
	    Quit = true;
	    break;

	case NOMEM:
	    sprintf(Msg, E_NOMEM);
	    Quit = true;
	    break;

	case TOOMANYVALS:
	    sprintf(Msg, E_TOOMANYVALS(S1, (int) (long) S2));
	    break;

	case BADDISCRETE:
	    sprintf(Msg, E_BADDISCRETE, S1);
	    break;

	case NOTARGET:
	    sprintf(Msg, E_NOTARGET, S1);
	    Quit = true;
	    break;

	case BADCTARGET:
	    sprintf(Msg, E_BADCTARGET, S1);
	    Quit = true;
	    break;

	case BADDTARGET:
	    sprintf(Msg, E_BADDTARGET, S1);
	    Quit = true;
	    break;

	case LONGNAME:
	    sprintf(Msg, E_LONGNAME);
	    Quit = true;
	    break;

	case HITEOF:
	    sprintf(Msg, E_HITEOF);
	    break;

	case MISSNAME:
	    sprintf(Msg, E_MISSNAME, S2);
	    break;

	case BADTSTMP:
	    sprintf(Msg, E_BADTSTMP(S2, S1));
	    break;

	case BADDATE:
	    sprintf(Msg, E_BADDATE(S2, S1));
	    break;

	case BADTIME:
	    sprintf(Msg, E_BADTIME(S2, S1));
	    break;

	case UNKNOWNATT:
	    sprintf(Msg, E_UNKNOWNATT, S1);
	    break;

	case BADDEF1:
	    sprintf(Msg, E_BADDEF1(AttName[MaxAtt], S1, S2));
	    break;

	case BADDEF2:
	    sprintf(Msg, E_BADDEF2(AttName[MaxAtt], S1, S2));
	    break;

	case SAMEATT:
	    sprintf(Msg, E_SAMEATT(AttName[MaxAtt], S1));
	    WarningOnly = true;
	    break;

	case BADDEF3:
	    sprintf(Msg, E_BADDEF3, AttName[MaxAtt]);
	    break;

	case BADDEF4:
	    sprintf(Msg, E_BADDEF4, AttName[MaxAtt]);
	    WarningOnly = true;
	    break;

	case MODELFILE:
	    sprintf(Msg, EX_MODELFILE(Fn));
	    sprintf(Msg, "    (%s `%s')\n", S1, S2);
	    Quit = true;
	    break;
    }

    fprintf(Of, Buffer);
	
    if ( ! WarningOnly ) ErrMsgs++;

    if ( ErrMsgs == 10 )
    {
	fprintf(Of,  T_ErrorLimit);
	MaxCase--;
	Quit = true;
    }

    if ( Quit && Of )
    {
	Goodbye(1);
    }
}



/*************************************************************************/
/*                                                                       */
/*      Generate the label for a case                                    */
/*                                                                       */
/*************************************************************************/

char	LabelBuffer[1000];


String CaseLabel(CaseNo N)
/*     ---------  */
{
    String      p;

    if ( LabelAtt && (p = IgnoredVals + SVal(Case[N], LabelAtt)) )
	;
    else
    {
	sprintf(LabelBuffer, "#%d", N+1);
	p = LabelBuffer;
    }

    return p;
}



/*************************************************************************/
/*									 */
/*	Open file with given extension for read/write			 */
/*									 */
/*************************************************************************/


FILE *GetFile(String Extension, String RW)
/*    --------  */
{
    strcpy(Fn, FileStem);
    strcat(Fn, Extension);
    return fopen(Fn, RW);
}



/*************************************************************************/
/*									 */
/*	Determine total elapsed time so far.				 */
/*									 */
/*************************************************************************/


#include <sys/time.h>

double  ExecTime()
/*      --------  */
{
    struct timeval	TV;
    struct timezone	TZ={0,0};

    gettimeofday(&TV, &TZ);
    return TV.tv_sec + TV.tv_usec / 1000000.0;
}



/*************************************************************************/
/*									 */
/*	Determine precision of floating value				 */
/*									 */
/*************************************************************************/


int Denominator(ContValue Val)
/*  -----------  */
{
    double	RoundErr, Accuracy;
    int		Mult;

    Accuracy = fabs(Val) * 1E-6;	/* approximate */
    Val = modf(Val, &RoundErr);

    for ( Mult = 100000 ; Mult >= 1 ; Mult /= 10 )
    {
	RoundErr = fabs(rint(Val * Mult) / Mult - Val);
	if ( RoundErr > 2 * Accuracy )
	{
	    return Mult * 10;
	}
    }

    return 1;
}



/*************************************************************************/
/*									 */
/*	Routines to process date (Algorithm due to Gauss?)		 */
/*									 */
/*************************************************************************/


int GetInt(String S, int N)
/*  ------  */
{
    int	Result=0;

    while ( N-- )
    {
	if ( ! isdigit(*S) ) return 0;

	Result = Result * 10 + (*S++ - '0');
    }

    return Result;
}


int DateToDay(String DS)	/*  Day 1 is 0000/03/01  */
/*  ---------  */
{
    int Year, Month, Day;

    if ( strlen(DS) != 10 ) return 0;

    Year  = GetInt(DS, 4);
    Month = GetInt(DS+5, 2);
    Day   = GetInt(DS+8, 2);

    if ( ! ( DS[4] == '/' && DS[7] == '/' || DS[4] == '-' && DS[7] == '-' ) ||
	 Year < 0 || Month < 1 || Day < 1 ||
	 Month > 12 ||
	 Day > 31 ||
	 Day > 30 &&
	    ( Month == 4 || Month == 6 || Month == 9 || Month == 11 ) ||
	 Month == 2 &&
	    ( Day > 29 ||
	      Day > 28 && ( Year % 4 != 0 ||
			    Year % 100 == 0 && Year % 400 != 0 ) ) )
    {
	return 0;
    }

    if ( (Month -= 2) <= 0 )
    {
	Month += 12;
	Year -= 1;
    }

    return Year * 365 + Year / 4 - Year / 100 + Year / 400
	   + 367 * Month / 12
	   + Day - 30;
}



void DayToDate(int Day, String Date)
/*   ---------  */
{
    int Year, Month, OrigDay=Day;

    if ( Day <= 0 )
    {
	strcpy(Date, "?");
	return;
    }

    Year = (Day - 1) / 365.2425L;  /*  Year = completed years  */
    Day -= Year * 365 + Year / 4 - Year / 100 + Year / 400;

    if ( Day < 1 )
    {
	Year--;
	Day = OrigDay - (Year * 365 + Year / 4 - Year / 100 + Year / 400);
    }
    else
    if ( Day > 366 ||
	 Day == 366 &&
	 ( (Year+1) % 4 != 0 || (Year+1) % 100 == 0 && (Year+1) % 400 != 0 ) )
    {
	Year++;
	Day = OrigDay - (Year * 365 + Year / 4 - Year / 100 + Year / 400);
    }

    Month = (Day + 30) * 12 / 367;
    Day -= 367 * Month / 12 - 30;
    if ( Day < 1 )
    {
	Month = 11;
	Day = 31;
    }

    Month += 2;
    if ( Month > 12 )
    {
	Month -= 12;
	Year++;
    }

    sprintf(Date, "%d/%d%d/%d%d", Year, Month/10, Month % 10, Day/10, Day % 10);
}



/*************************************************************************/
/*									 */
/*	Routines to process clock time and timestamps			 */
/*									 */
/*************************************************************************/


int TimeToSecs(String TS)
/*  ----------  */
{
    int Hour, Mins, Secs;

    if ( strlen(TS) != 8 ) return -1;

    Hour = GetInt(TS, 2);
    Mins = GetInt(TS+3, 2);
    Secs = GetInt(TS+6, 2);

    if ( TS[2] != ':' || TS[5] != ':' ||
	 Hour >= 24 || Mins >= 60 || Secs >= 60 )
    {
	return -1;
    }

    return Hour * 3600 + Mins * 60 + Secs;
}



void SecsToTime(int Secs, String Time)
/*   ----------  */
{
    int Hour, Mins;

    Hour = Secs / 3600;
    Mins = (Secs % 3600) / 60;
    Secs = Secs % 60;

    sprintf(Time, "%d%d:%d%d:%d%d",
		  Hour / 10, Hour % 10,
		  Mins / 10, Mins % 10,
		  Secs / 10, Secs % 10);
}



void SetTSBase(int y)
/*   ---------  */
{
    y -= 15;
    TSBase = y * 365 + y / 4 - y / 100 + y / 400 + (367 * 4) / 12 + 1 - 30;
}



int TStampToMins(String TS)
/*  ------------  */
{
    int		Day, Sec, i;

    /*  Check for reasonable length and space between date and time  */

    if ( strlen(TS) < 19 || ! Space(TS[10]) ) return (1 << 30);

    /*  Read date part  */

    TS[10] = '\00';
    Day = DateToDay(TS);
    TS[10] = ' ';

    /*  Skip one or more spaces  */

    for ( i = 11 ; TS[i] && Space(TS[i]) ; i++ )
	;

    /*  Read time part  */

    Sec = TimeToSecs(TS+i);

    /*  Return a long time in the future if there is an error  */

    return ( Day < 1 || Sec < 0 ? (1 << 30) :
	     (Day - TSBase) * 1440 + (Sec + 30) / 60 );
}



/*************************************************************************/
/*									 */
/*	Convert a continuous value to a string.		DS must be	 */
/*	large enough to hold any value (e.g. a date, time, ...)		 */
/*									 */
/*************************************************************************/


void CValToStr(ContValue CV, Attribute Att, String DS)
/*   ---------  */
{
    int		Mins;

    if ( TStampVal(Att) )
    {
	DayToDate(floor(CV / 1440) + TSBase, DS);
	DS[10] = ' ';
	Mins = rint(CV) - floor(CV / 1440) * 1440;
	SecsToTime(Mins * 60, DS+11);
    }
    else
    if ( DateVal(Att) )
    {
	DayToDate(CV, DS);
    }
    else
    if ( TimeVal(Att) )
    {
	SecsToTime(CV, DS);
    }
    else
    {
	sprintf(DS, "%.*g", PREC, CV);
    }
}



/*************************************************************************/
/*									 */
/*	Check parameter value						 */
/*									 */
/*************************************************************************/


void Check(float Val, float Low, float High)
/*   -----  */
{
    if ( Val < Low || Val > High )
    {
	fprintf(Of, TX_IllegalValue(Val, Low, High));
	exit(1);
    }
}





/*************************************************************************/
/*									 */
/*	Deallocate all dynamic storage					 */
/*									 */
/*************************************************************************/


void Cleanup()
/*   -------  */
{
    int		t, r;

    extern DataRec	*Blocked;
    extern Tree		*SubDef;
    extern int		SubSpace, ActiveSpace, PropValSize;
    extern RuleNo	*Active;
    extern float	*AttImp;
    extern char		*PropVal;
    extern Boolean	*Split, *Used;
    extern FILE		*Uf;

    NotifyStage(CLEANUP);

    CheckClose(Uf);					Uf = Nil;
    CheckClose(TRf);					TRf = Nil;

    /*  Boost voting (construct.c)  */

    FreeUnlessNil(BVoteBlock);				BVoteBlock = Nil;

    /*  Stuff from attribute winnowing  */

    FreeUnlessNil(SaveCase);				SaveCase = Nil;
    FreeUnlessNil(AttImp);				AttImp = Nil;
    FreeUnlessNil(Split);				Split = Nil;
    FreeUnlessNil(Used);				Used = Nil;

    FreeUnlessNil(PropVal);				PropVal = Nil;
							PropValSize = 0;

    if ( RULES )
    {
	FreeFormRuleData();
	FreeSiftRuleData();
    }

    /*  May have interrupted a winnowing tree  */

    if ( WINNOW && WTree )
    {
	FreeTree(WTree);				WTree = Nil;
    }

    FreeUnlessNil(Blocked);				Blocked = Nil;

    FreeData();

    if ( MCost )
    {
	FreeVector((void **) MCost, 1, MaxClass);	MCost = Nil;
	FreeUnlessNil(WeightMul);			WeightMul = Nil;
    }

    ForEach(t, 0, MaxTree)
    {
	FreeClassifier(t);
    }

    if ( RULES )
    {
	/*  May be incomplete ruleset in Rule[]  */

	if ( Rule )
	{
	    ForEach(r, 1, NRules)
	    {
		FreeRule(Rule[r]);
	    }
	    Free(Rule);					Rule = Nil;
	}						

	FreeUnlessNil(RuleSet);				RuleSet = Nil;
	FreeUnlessNil(LogCaseNo);			LogCaseNo = Nil;
	FreeUnlessNil(LogFact);				LogFact = Nil;
    }

    FreeTreeData();

    FreeUnlessNil(Active);				Active = Nil;
							ActiveSpace = 0;

    FreeUnlessNil(UtilErr);				UtilErr = Nil;
    FreeUnlessNil(UtilBand);				UtilBand = Nil;
    FreeUnlessNil(UtilCost);				UtilCost = Nil;

    FreeUnlessNil(SomeMiss);				SomeMiss = Nil;
    FreeUnlessNil(SomeNA);				SomeNA = Nil;

    FreeNames();

    FreeUnlessNil(SubDef);				SubDef = Nil;
							SubSpace = 0;
    MaxCase = -1;

    NotifyStage(0);
}



#ifdef UTF8
///////////////////////////////////////////////////////////////////////////
//									 //
//	Routines for Unicode/UTF-8 processing				 //
//	-------------------------------------				 //
//									 //
///////////////////////////////////////////////////////////////////////////

#include <wchar.h>



/*************************************************************************/
/*									 */
/*	Determine the total character width of a UTF-8 string		 */
/*									 */
/*************************************************************************/


int UTF8CharWidth(unsigned char *U)
/*  -------------  */
{
    int		CWidth=0, Mask, This;
    wchar_t	Unicode;

    while ( *U )
    {
	Unicode = *U;

	if ( *U < 0x7F )
	{
	    /*  ASCII character  */

	    CWidth++;
	    U++;
	}
	else
	{
	    /*  Discard header bits  */

	    Mask = 0x80;
	    while ( Unicode & Mask )
	    {
		Unicode ^= Mask;
		Mask = Mask >> 1;
	    }

	    while ( ((*(++U)) & 0xc0) == 0x80 )
	    {
		Unicode = (Unicode << 6) | (*U & 0x3f);
	    }

	    if ( (This = wcwidth(Unicode)) > 0 ) CWidth += This;
	}
    }

    return CWidth;
}



////////////////////////////////////////////////////////////////////////////////
//	Public domain code to determine the width of a Unicode character      //
////////////////////////////////////////////////////////////////////////////////


/*
 * This is an implementation of wcwidth() and wcswidth() as defined in
 * "The Single UNIX Specification, Version 2, The Open Group, 1997"
 * <http://www.UNIX-systems.org/online.html>
 *
 * Markus Kuhn -- 2000-02-08 -- public domain
 */

//#include <wchar.h>

/* These functions define the column width of an ISO 10646 character
 * as follows:
 *
 *    - The null character (U+0000) has a column width of 0.
 *
 *    - Other C0/C1 control characters and DEL will lead to a return
 *      value of -1.
 *
 *    - Non-spacing and enclosing combining characters (general
 *      category code Mn or Me in the Unicode database) have a
 *      column width of 0.
 *
 *    - Spacing characters in the East Asian Wide (W) or East Asian
 *      FullWidth (F) category as defined in Unicode Technical
 *      Report #11 have a column width of 2.
 *
 *    - All remaining characters (including all printable
 *      ISO 8859-1 and WGL4 characters, Unicode control characters,
 *      etc.) have a column width of 1.
 *
 * This implementation assumes that wchar_t characters are encoded
 * in ISO 10646.
 */

int wcwidth(wchar_t ucs)
{
  /* sorted list of non-overlapping intervals of non-spacing characters */
  static const struct interval {
    unsigned short first;
    unsigned short last;
  } combining[] = {
    { 0x0300, 0x034E }, { 0x0360, 0x0362 }, { 0x0483, 0x0486 },
    { 0x0488, 0x0489 }, { 0x0591, 0x05A1 }, { 0x05A3, 0x05B9 },
    { 0x05BB, 0x05BD }, { 0x05BF, 0x05BF }, { 0x05C1, 0x05C2 },
    { 0x05C4, 0x05C4 }, { 0x064B, 0x0655 }, { 0x0670, 0x0670 },
    { 0x06D6, 0x06E4 }, { 0x06E7, 0x06E8 }, { 0x06EA, 0x06ED },
    { 0x0711, 0x0711 }, { 0x0730, 0x074A }, { 0x07A6, 0x07B0 },
    { 0x0901, 0x0902 }, { 0x093C, 0x093C }, { 0x0941, 0x0948 },
    { 0x094D, 0x094D }, { 0x0951, 0x0954 }, { 0x0962, 0x0963 },
    { 0x0981, 0x0981 }, { 0x09BC, 0x09BC }, { 0x09C1, 0x09C4 },
    { 0x09CD, 0x09CD }, { 0x09E2, 0x09E3 }, { 0x0A02, 0x0A02 },
    { 0x0A3C, 0x0A3C }, { 0x0A41, 0x0A42 }, { 0x0A47, 0x0A48 },
    { 0x0A4B, 0x0A4D }, { 0x0A70, 0x0A71 }, { 0x0A81, 0x0A82 },
    { 0x0ABC, 0x0ABC }, { 0x0AC1, 0x0AC5 }, { 0x0AC7, 0x0AC8 },
    { 0x0ACD, 0x0ACD }, { 0x0B01, 0x0B01 }, { 0x0B3C, 0x0B3C },
    { 0x0B3F, 0x0B3F }, { 0x0B41, 0x0B43 }, { 0x0B4D, 0x0B4D },
    { 0x0B56, 0x0B56 }, { 0x0B82, 0x0B82 }, { 0x0BC0, 0x0BC0 },
    { 0x0BCD, 0x0BCD }, { 0x0C3E, 0x0C40 }, { 0x0C46, 0x0C48 },
    { 0x0C4A, 0x0C4D }, { 0x0C55, 0x0C56 }, { 0x0CBF, 0x0CBF },
    { 0x0CC6, 0x0CC6 }, { 0x0CCC, 0x0CCD }, { 0x0D41, 0x0D43 },
    { 0x0D4D, 0x0D4D }, { 0x0DCA, 0x0DCA }, { 0x0DD2, 0x0DD4 },
    { 0x0DD6, 0x0DD6 }, { 0x0E31, 0x0E31 }, { 0x0E34, 0x0E3A },
    { 0x0E47, 0x0E4E }, { 0x0EB1, 0x0EB1 }, { 0x0EB4, 0x0EB9 },
    { 0x0EBB, 0x0EBC }, { 0x0EC8, 0x0ECD }, { 0x0F18, 0x0F19 },
    { 0x0F35, 0x0F35 }, { 0x0F37, 0x0F37 }, { 0x0F39, 0x0F39 },
    { 0x0F71, 0x0F7E }, { 0x0F80, 0x0F84 }, { 0x0F86, 0x0F87 },
    { 0x0F90, 0x0F97 }, { 0x0F99, 0x0FBC }, { 0x0FC6, 0x0FC6 },
    { 0x102D, 0x1030 }, { 0x1032, 0x1032 }, { 0x1036, 0x1037 },
    { 0x1039, 0x1039 }, { 0x1058, 0x1059 }, { 0x17B7, 0x17BD },
    { 0x17C6, 0x17C6 }, { 0x17C9, 0x17D3 }, { 0x18A9, 0x18A9 },
    { 0x20D0, 0x20E3 }, { 0x302A, 0x302F }, { 0x3099, 0x309A },
    { 0xFB1E, 0xFB1E }, { 0xFE20, 0xFE23 }
  };
  int min = 0;
  int max = sizeof(combining) / sizeof(struct interval) - 1;
  int mid;

  /* test for 8-bit control characters */
  if (ucs == 0)
    return 0;
  if (ucs < 32 || (ucs >= 0x7f && ucs < 0xa0))
    return -1;

  /* first quick check for Latin-1 etc. characters */
  if (ucs < combining[0].first)
    return 1;

  /* binary search in table of non-spacing characters */
  while (max >= min) {
    mid = (min + max) / 2;
    if (combining[mid].last < ucs)
      min = mid + 1;
    else if (combining[mid].first > ucs)
      max = mid - 1;
    else if (combining[mid].first <= ucs && combining[mid].last >= ucs)
      return 0;
  }

  /* if we arrive here, ucs is not a combining or C0/C1 control character */

  /* fast test for majority of non-wide scripts */
  if (ucs < 0x1100)
    return 1;

  return 1 +
    ((ucs >= 0x1100 && ucs <= 0x115f) || /* Hangul Jamo */
     (ucs >= 0x2e80 && ucs <= 0xa4cf && (ucs & ~0x0011) != 0x300a &&
      ucs != 0x303f) ||                  /* CJK ... Yi */
     (ucs >= 0xac00 && ucs <= 0xd7a3) || /* Hangul Syllables */
     (ucs >= 0xf900 && ucs <= 0xfaff) || /* CJK Compatibility Ideographs */
     (ucs >= 0xfe30 && ucs <= 0xfe6f) || /* CJK Compatibility Forms */
     (ucs >= 0xff00 && ucs <= 0xff5f) || /* Fullwidth Forms */
     (ucs >= 0xffe0 && ucs <= 0xffe6));
}


int wcswidth(const wchar_t *pwcs, size_t n)
{
  int w, width = 0;

  for (;*pwcs && n-- > 0; pwcs++)
    if ((w = wcwidth(*pwcs)) < 0)
      return -1;
    else
      width += w;

  return width;
}
#endif
