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
/*	Main routine, C5.0						 */
/*	------------------						 */
/*									 */
/*************************************************************************/


#include "defns.i"
#include "extern.i"
#include <signal.h>

#include <sys/unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

#define SetFOpt(V)	V = strtod(OptArg, &EndPtr);\
			if ( ! EndPtr || *EndPtr != '\00' ) break;\
			ArgOK = true
#define SetIOpt(V)	V = strtol(OptArg, &EndPtr, 10);\
			if ( ! EndPtr || *EndPtr != '\00' ) break;\
			ArgOK = true


int main(int Argc, char *Argv[])
/*  ----  */
{
    int			o;
    extern String	OptArg, Option;
    char		*EndPtr;
    Boolean		FirstTime=true, ArgOK;
    double		StartTime;
    FILE		*F;
    CaseNo		SaveMaxCase;
    Attribute		Att;

    struct rlimit RL;

    /*  Make sure there is a largish runtime stack  */

    getrlimit(RLIMIT_STACK, &RL);

    RL.rlim_cur = Max(RL.rlim_cur, 20 * 1024 * 1024);

    if ( RL.rlim_max > 0 )	/* -1 if unlimited */
    {
	RL.rlim_cur = Min(RL.rlim_max, RL.rlim_cur);
    }

    setrlimit(RLIMIT_STACK, &RL);


    /*  Check for output to be saved to a file  */

    if ( Argc > 2 && ! strcmp(Argv[Argc-2], "-o") )
    {
	Of = fopen(Argv[Argc-1], "w");
	Argc -= 2;
    }

    if ( ! Of )
    {
	Of = stdout;
    }

    KRInit = time(0) & 07777;

    PrintHeader("");

    /*  Process options  */

    while ( (o = ProcessOption(Argc, Argv, "f+bpv+t+sm+c+S+I+ru+egX+wh")) )
    {
	if ( FirstTime )
	{
	    fprintf(Of, T_OptHeader);
	    FirstTime = false;
	}

	ArgOK = false;

	switch (o)
	{
	case 'f':   FileStem = OptArg;
		    fprintf(Of, T_OptApplication, FileStem);
		    ArgOK = true;
		    break;
	case 'b':   BOOST = true;
		    fprintf(Of, T_OptBoost);
		    if ( TRIALS == 1 ) TRIALS = 10;
		    ArgOK = true;
		    break;
	case 'p':   PROBTHRESH = true;
		    fprintf(Of, T_OptProbThresh);
		    ArgOK = true;
		    break;
#ifdef VerbOpt
	case 'v':   SetIOpt(VERBOSITY);
		    fprintf(Of, "\tVerbosity level %d\n", VERBOSITY);
		    ArgOK = true;
		    break;
#endif
	case 't':   SetIOpt(TRIALS);
		    fprintf(Of, T_OptTrials, TRIALS);
		    Check(TRIALS, 3, 1000);
		    BOOST = true;
		    break;
	case 's':   SUBSET = true;
		    fprintf(Of, T_OptSubsets);
		    ArgOK = true;
		    break;
	case 'm':   SetFOpt(MINITEMS);
		    fprintf(Of, T_OptMinCases, MINITEMS);
		    Check(MINITEMS, 1, 1000000);
		    break;
	case 'c':   SetFOpt(CF);
		    fprintf(Of, T_OptCF, CF);
		    Check(CF, 0, 100);
		    CF /= 100;
		    break;
	case 'r':   RULES = true;
		    fprintf(Of, T_OptRules);
		    ArgOK = true;
		    break;
	case 'S':   SetFOpt(SAMPLE);
		    fprintf(Of, T_OptSampling, SAMPLE);
		    Check(SAMPLE, 0.1, 99.9);
		    SAMPLE /= 100;
		    break;
	case 'I':   SetIOpt(KRInit);
		    fprintf(Of, T_OptSeed, KRInit);
		    KRInit = KRInit & 07777;
		    break;
	case 'u':   SetIOpt(UTILITY);
		    fprintf(Of, T_OptUtility, UTILITY);
		    Check(UTILITY, 2, 10000);
		    RULES = true;
		    break;
	case 'e':   NOCOSTS = true;
		    fprintf(Of, T_OptNoCosts);
		    ArgOK = true;
		    break;
	case 'w':   WINNOW = true;
		    fprintf(Of, T_OptWinnow);
		    ArgOK = true;
		    break;
	case 'g':   GLOBAL = false;
		    fprintf(Of, T_OptNoGlobal);
		    ArgOK = true;
		    break;
	case 'X':   SetIOpt(FOLDS);
		    fprintf(Of, T_OptXval, FOLDS);
		    Check(FOLDS, 2, 1000);
		    XVAL = true;
		    break;
	}

	if ( ! ArgOK )
	{
	    if ( o != 'h' )
	    {
		fprintf(Of, T_UnregnizedOpt,
			    Option,
			    ( ! OptArg || OptArg == Option+2 ? "" : OptArg ));
		fprintf(Of, T_SummaryOpts);
	    }
	    fprintf(Of, T_ListOpts);
	    Goodbye(1);
	}
    }

    if ( UTILITY && BOOST )
    {
	fprintf(Of, T_UBWarn);
    }

    StartTime = ExecTime();

    /*  Get information on training data  */

    if ( ! (F = GetFile(".names", "r")) ) Error(NOFILE, "", "");
    GetNames(F);

    if ( ClassAtt )
    {
	fprintf(Of, T_ClassVar, AttName[ClassAtt]);
    }

    NotifyStage(READDATA);
    Progress(-1.0);

    /*  Allocate space for SomeMiss[] and SomeNA[] */

    SomeMiss = AllocZero(MaxAtt+1, Boolean);
    SomeNA   = AllocZero(MaxAtt+1, Boolean);

    /*  Read data file  */

    if ( ! (F = GetFile(".data", "r")) ) Error(NOFILE, "", "");
    GetData(F, true, false);
    fprintf(Of, TX_ReadData(MaxCase+1, MaxAtt, FileStem));

    if ( XVAL && (F = GetFile(".test", "r")) )
    {
	SaveMaxCase = MaxCase;
	GetData(F, false, false);
	fprintf(Of, TX_ReadTest(MaxCase-SaveMaxCase, FileStem));
    }

    /*  Check whether case weight attribute appears  */

    if ( CWtAtt )
    {
	fprintf(Of, T_CWtAtt);
    }

    if ( ! NOCOSTS && (F = GetFile(".costs", "r")) )
    {
	GetMCosts(F);
	if ( MCost )
	{
	    fprintf(Of, T_ReadCosts, FileStem);
	}
    }

    /*  Note any attribute exclusions/inclusions  */

    if ( AttExIn )
    {
	fprintf(Of, "%s", ( AttExIn == -1 ? T_AttributesOut : T_AttributesIn ));

	ForEach(Att, 1, MaxAtt)
	{
	    if ( Att != ClassAtt &&
		 Att != CWtAtt &&
		 ( StatBit(Att, SKIP) > 0 ) == ( AttExIn == -1 ) )
	    {
		fprintf(Of, "    %s\n", AttName[Att]);
	    }
	}
    }

    /*  Build decision trees  */

    if ( ! BOOST )
    {
	TRIALS = 1;
    }

    InitialiseTreeData();
    if ( RULES )
    {
	RuleSet = AllocZero(TRIALS+1, CRuleSet);
    }

    if ( WINNOW )
    {
	NotifyStage(WINNOWATTS);
	Progress(-MaxAtt);
	WinnowAtts();
    }

    if ( XVAL )
    {
	CrossVal();
    }
    else
    {
	ConstructClassifiers();

	/*  Evaluation  */

	fprintf(Of, T_EvalTrain, MaxCase+1);

	NotifyStage(EVALTRAIN);
	Progress(-TRIALS * (MaxCase+1.0));

	Evaluate(CMINFO | USAGEINFO);

	if ( (F = GetFile(( SAMPLE ? ".data" : ".test" ), "r")) )
	{
	    NotifyStage(READTEST);
	    fprintf(Of, "\n");

	    FreeData();
	    GetData(F, false, false);

	    fprintf(Of, T_EvalTest, MaxCase+1);

	    NotifyStage(EVALTEST);
	    Progress(-TRIALS * (MaxCase+1.0));

	    Evaluate(CMINFO);
	}
    }

    fprintf(Of, T_Time, ExecTime() - StartTime);

#ifdef VerbOpt
    Cleanup();
#endif

    return 0;
}
