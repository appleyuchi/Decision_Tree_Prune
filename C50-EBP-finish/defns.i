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
/*		Definitions used in C5.0				 */
/*              ------------------------				 */
/*									 */
/*************************************************************************/


#define	 RELEASE	"2.07 GPL Edition"

				/*  Uncomment following line to enable
				    sample estimates for large datasets.
				    This can lead to some variablility,
				    especially when used with SMP  */
//#define	SAMPLE_ESTIMATES

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
#include <float.h>

#include "text.i"



/*************************************************************************/
/*									 */
/*		Definitions dependent on cc options			 */
/*									 */
/*************************************************************************/


#ifdef	VerbOpt
#define Goodbye(x)		{Cleanup(); exit(x);}
#else
#define Goodbye(x)		exit(x)
#endif

#ifdef	VerbOpt
#include <assert.h>
#define	Verbosity(d,s)		if(VERBOSITY >= d) {s;}
#else
#define	 assert(x)
#define Verbosity(d,s)
#endif


/*  Alternative random number generator  */

#define AltRandom		drand48()
#define	AltSeed(x)		srand48(x)

#define Free(x)			{free(x); x=0;}


/*************************************************************************/
/*									 */
/*		Constants, macros etc.					 */
/*									 */
/*************************************************************************/

#define	 THEORYFRAC	0.23	/* discount rate for estimated coding cost */

#define	 Nil	   0		/* null pointer */
#define	 false	   0
#define	 true	   1
#define	 None	   -1
#define	 Epsilon   1E-4
#define	 MinLeaf   0.05		/* minimum weight for non-null leaf */
#define	 Width	   80		/* approx max width of output */

#define  EXCLUDE   1		/* special attribute status: do not use */
#define  SKIP	   2		/* do not use in classifiers */
#define  DISCRETE  4		/* ditto: collect values as data read */
#define  ORDERED   8		/* ditto: ordered discrete values */
#define  DATEVAL   16		/* ditto: YYYY/MM/DD or YYYY-MM-DD */
#define  STIMEVAL  32		/* ditto: HH:MM:SS */
#define	 TSTMPVAL  64		/* date time */

#define	 CMINFO	   1		/* generate confusion matrix */
#define	 USAGEINFO 2		/* print usage information */

				/* unknown and N/A values are represented by
				   unlikely floating-point numbers
				   (octal 01600000000 and 01) */
#define	 UNKNOWN   01600000000	/* 1.5777218104420236e-30 */
#define	 NA	   01		/* 1.4012984643248171e-45 */

#define	 BrDiscr   1
#define	 BrThresh  2
#define	 BrSubset  3

#define  Plural(n)		((n) != 1 ? "s" : "")

#define  AllocZero(N,T)		(T *) Pcalloc(N, sizeof(T))
#define  Alloc(N,T)		AllocZero(N,T) /* for safety */
#define  Realloc(V,N,T)		V = (T *) Prealloc(V, (N)*sizeof(T))

#define	 Max(a,b)               ((a)>(b) ? (a) : (b))
#define	 Min(a,b)               ((a)<(b) ? (a) : (b))

#define	 Log2			0.69314718055994530942
#define	 Log(x)			((x) <= 0 ? 0.0 : log((double)x) / Log2)

#define	 Bit(b)			(1 << (b))
#define	 In(b,s)		((s[(b) >> 3]) & Bit((b) & 07))
#define	 ClearBits(n,s)		memset(s,0,n)
#define	 CopyBits(n,f,t)	memcpy(t,f,n)
#define	 SetBit(b,s)		(s[(b) >> 3] |= Bit((b) & 07))
#define	 ResetBit(b,s)		(s[(b) >> 3] ^= Bit((b) & 07))

#define	 ForEach(v,f,l)		for(v=f ; v<=l ; ++v)

#define	 CountCases(f,l)	(UnitWeights ? (l-(f)+1.0) : SumWeights(f,l))

#define	 StatBit(a,b)		(SpecialStatus[a]&(b))
#define	 Exclude(a)		StatBit(a,EXCLUDE)
#define	 Skip(a)		StatBit(a,EXCLUDE|SKIP)
#define  Discrete(a)		(MaxAttVal[a] || StatBit(a,DISCRETE))
#define  Continuous(a)		(! MaxAttVal[a] && ! StatBit(a,DISCRETE))
#define	 Ordered(a)		StatBit(a,ORDERED)
#define	 DateVal(a)		StatBit(a,DATEVAL)
#define	 TimeVal(a)		StatBit(a,STIMEVAL)
#define	 TStampVal(a)		StatBit(a,TSTMPVAL)

#define  FreeUnlessNil(p)	if((p)!=Nil) Free(p)

#define  CheckClose(f)		if(f) {fclose(f); f=Nil;}

#define	 Int(x)			((int)(x+0.5))

#define  Space(s)	(s == ' ' || s == '\n' || s == '\r' || s == '\t')
#define  SkipComment	while ( ( c = InChar(f) ) != '\n' && c != EOF )

#define	 P1(x)		(rint((x)*10) / 10)

#define	 No(f,l)	((l)-(f)+1)

#define	 EmptyNA(T)	(T->Branch[1]->Cases < 0.01)

#define  Before(n1,n2)  (n1->Tested < n2->Tested ||\
			n1->Tested == n2->Tested && n1->Cut < n2->Cut)

#define	 Swap(a,b)	{DataRec xab;\
			 assert(a >= 0 && a <= MaxCase &&\
			        b >= 0 && b <= MaxCase);\
			 xab = Case[a]; Case[a] = Case[b]; Case[b] = xab;}


#define	 NOFILE		 0
#define	 BADCLASSTHRESH	 1
#define	 LEQCLASSTHRESH	 2
#define	 BADATTNAME	 3
#define	 EOFINATT	 4
#define	 SINGLEATTVAL	 5
#define	 BADATTVAL	 6
#define	 BADNUMBER	 7
#define	 BADCLASS	 8
#define	 BADCOSTCLASS	 9
#define	 BADCOST	10
#define	 NOMEM		11
#define	 TOOMANYVALS	12
#define	 BADDISCRETE	13
#define	 NOTARGET	14
#define	 BADCTARGET	15
#define	 BADDTARGET	16
#define	 LONGNAME	17
#define	 HITEOF		18
#define	 MISSNAME	19
#define	 BADDATE	20
#define	 BADTIME	21
#define	 BADTSTMP	22
#define	 DUPATTNAME	23
#define	 UNKNOWNATT	24
#define	 BADDEF1	25
#define	 BADDEF2	26
#define	 BADDEF3	27
#define	 BADDEF4	28
#define	 SAMEATT	29
#define	 MODELFILE	30
#define	 CWTATTERR	31

#define	 READDATA	 1
#define	 WINNOWATTS	 2
#define	 FORMTREE	 3
#define	 SIMPLIFYTREE	 4
#define	 FORMRULES	 5
#define	 SIFTRULES	 6
#define	 EVALTRAIN	 7
#define	 READTEST	 8
#define	 EVALTEST	 9
#define	 CLEANUP	10
#define	 ALLOCTABLES	11
#define	 RESULTS	12
#define	 READXDATA	13


/*************************************************************************/
/*									 */
/*		Type definitions					 */
/*									 */
/*************************************************************************/


typedef  unsigned char	Boolean, BranchType, *Set, Byte;
typedef	 char		*String;

typedef  int	CaseNo;		/* data case number */
typedef  float	CaseCount;	/* count of (partial) cases */

typedef  int	ClassNo,	/* class number, 1..MaxClass */
		DiscrValue,	/* discrete attribute value */
		Attribute;	/* attribute number, 1..MaxAtt */

#ifdef USEDOUBLE
typedef	 double	ContValue;	/* continuous attribute value */
#define	 PREC	14		/* precision */
#else
typedef	 float	ContValue;	/* continuous attribute value */
#define	 PREC	 7		/* precision */
#endif


typedef  union	 _def_val
	 {
	    String	_s_val;		/* att val for comparison */
	    ContValue	_n_val;		/* number for arith */
	 }
	 DefVal;

typedef  struct  _def_elt
	 {
	    short	_op_code;	/* type of element */
	    DefVal	_operand;	/* string or numeric value */
	 }
	 DefElt, *Definition;

typedef  struct  _elt_rec
	 {
	    int		Fi,		/* index of first char of element */
			Li;		/* last ditto */
	    char	Type;		/* 'B', 'S', or 'N' */
	 }
	 EltRec;

#define	 DefOp(DE)	DE._op_code
#define	 DefSVal(DE)	DE._operand._s_val
#define	 DefNVal(DE)	DE._operand._n_val

#define	 OP_ATT			 0	/* opcodes */
#define	 OP_NUM			 1
#define	 OP_STR			 2
#define	 OP_MISS		 3
#define	 OP_AND			10
#define	 OP_OR			11
#define	 OP_EQ			20
#define	 OP_NE			21
#define	 OP_GT			22
#define	 OP_GE			23
#define	 OP_LT			24
#define	 OP_LE			25
#define	 OP_SEQ			26
#define	 OP_SNE			27
#define	 OP_PLUS		30
#define	 OP_MINUS		31
#define	 OP_UMINUS		32
#define	 OP_MULT		33
#define	 OP_DIV			34
#define	 OP_MOD			35
#define	 OP_POW			36
#define	 OP_SIN			40
#define	 OP_COS			41
#define	 OP_TAN			42
#define	 OP_LOG			43
#define	 OP_EXP			44
#define	 OP_INT			45
#define	 OP_END			99


typedef  union  _attribute_value
	 {
	    DiscrValue	_discr_val;
	    ContValue	_cont_val;
	 }
	 AttValue, *DataRec;

typedef	 struct _sort_rec
	 {
	    ContValue	V;
	    ClassNo	C;
	    float	W;
	 }
	 SortRec;

#define  CVal(Case,Att)		Case[Att]._cont_val
#define  DVal(Case,Att)		Case[Att]._discr_val
#define  XDVal(Case,Att)	(Case[Att]._discr_val & 077777777)
#define  SVal(Case,Att)		Case[Att]._discr_val
#define  Class(Case)		(*Case)._discr_val
#define  Weight(Case)		(*(Case-1))._cont_val

#define	 Unknown(Case,Att)	(DVal(Case,Att)==UNKNOWN)
#define	 UnknownVal(AV)		(AV._discr_val==UNKNOWN)
#define	 NotApplic(Case,Att)	(Att != ClassAtt && DVal(Case,Att)==NA)
#define	 NotApplicVal(AV)	(AV._discr_val==NA)

#define	 RelCWt(Case)		( Unknown(Case,CWtAtt)||\
				  NotApplic(Case,CWtAtt)||\
			  	  CVal(Case,CWtAtt)<=0 ? 1 :\
				  CVal(Case,CWtAtt)/AvCWt )

typedef  struct _treerec	*Tree;
typedef  struct _treerec
	 {
	    BranchType	NodeType;
	    ClassNo	Leaf;		/* best class at this node */
	    CaseCount	Cases,		/* no of cases at this node */
			*ClassDist,	/* class distribution of cases */
	    		Errors;		/* est or resub errors at this node */
	    Attribute	Tested; 	/* attribute referenced in test */
	    int		Forks,		/* number of branches at this node */
			Leaves;		/* number of non-empty leaves in tree */
	    ContValue	Cut,		/* threshold for continuous attribute */
		  	Lower,		/* lower limit of soft threshold */
		  	Upper,		/* upper limit ditto */
			Mid;		/* midpoint for soft threshold */
	    Set         *Subset;	/* subsets of discrete values  */
	    Tree	*Branch,	/* Branch[x] = subtree for outcome x */
			Parent;		/* node above this one */
	 }
	 TreeRec;


typedef	 struct _environment
	 {
	    CaseNo	Xp, Ep;			/* start and end of scan  */
	    double	Cases,			/* total cases */
			KnownCases,		/* ditto less missing values */
			ApplicCases,		/* cases with numeric values */
			HighCases, LowCases,	/* cases above/below cut */
			NAInfo,			/* info for N/A values */
			FixedSplitInfo,		/* split info for ?, N/A */
			BaseInfo,		/* info before split */
			UnknownRate,		/* proportion of ? values */
			MinSplit,		/* min cases before/after cut */
			**Freq,			/* local Freq[4][class] */
			*ClassFreq,		/* local class frequencies */
			*ValFreq;		/* cases with val i */
	    ClassNo	HighClass, LowClass;	/* class after/before cut */
	    ContValue	HighVal, LowVal;	/* values after/before cut */
	    SortRec	*SRec;			/* for Cachesort() */
	    Set		**Subset,		/* Subset[att][number] */
			*WSubset;		/* working subsets */
	    int		*Subsets,		/* no of subsets for att */
			Blocks,			/* intermediate no of subsets */
			Bytes,			/* size of each subset */
			ReasonableSubsets;
	    double	*SubsetInfo,		/* subset info */
			*SubsetEntr,		/* subset entropy */
			**MergeInfo,		/* info of merged subsets i,j */
			**MergeEntr;		/* entropy ditto */
	 }
	 EnvRec;


typedef  int	RuleNo;			/* rule number */

typedef  struct _condrec
	 {
	    BranchType	NodeType;	/* test type (see tree nodes) */
	    Attribute	Tested;		/* attribute tested */
	    ContValue	Cut;		/* threshold (if relevant) */
	    Set		Subset;		/* subset (if relevant) */
	    int		TestValue,	/* specified outcome of test */
			TestI;		/* rule tree index of this test */
	 }
	 CondRec, *Condition;


typedef  struct _rulerec
	 {
	    RuleNo	RNo;		/* rule number */
	    int		TNo,		/* trial number */
	    		Size;		/* number of conditions */
	    Condition	*Lhs;		/* conditions themselves */
	    ClassNo	Rhs;		/* class given by rule */
	    CaseCount	Cover,		/* number of cases covered by rule */
			Correct;	/* number on which correct */
	    float	Prior;		/* prior probability of RHS */
	    int		Vote;		/* unit = 0.001 */
	 }
	 RuleRec, *CRule;


typedef  struct _ruletreerec *RuleTree;
typedef  struct _ruletreerec
	 {
	    RuleNo	*Fire;		/* rules matched at this node */
	    Condition	CondTest;	/* new test */
	    int		Forks;		/* number of branches */
	    RuleTree	*Branch;	/* subtrees */
	 }
	 RuleTreeRec;


typedef struct _rulesetrec
	 {
	    RuleNo	SNRules;	/* number of rules */
	    CRule	*SRule;		/* rules */
	    ClassNo	SDefault;	/* default class for this ruleset */
	    RuleTree	RT;		/* rule tree (see ruletree.c) */
	 }
	 RuleSetRec, *CRuleSet;



/*************************************************************************/
/*									 */
/*		Function prototypes					 */
/*									 */
/*************************************************************************/

	/* c50.c */

int	    main(int, char *[]);
void	    FreeClassifier(int Trial);

	/* construct.c */

void	    ConstructClassifiers(void);
void	    InitialiseWeights(void);
void	    SetAvCWt(void);
void	    Evaluate(int Flags);
void	    EvaluateSingle(int Flags);
void	    EvaluateBoost(int Flags);
void	    RecordAttUsage(DataRec Case, int *Usage);

	/* getnames.c */

Boolean	    ReadName(FILE *f, String s, int n, char ColonOpt);
void	    GetNames(FILE *Nf);
void	    ExplicitAtt(FILE *Nf);
int	    Which(String Val, String *List, int First, int Last);
void	    ListAttsUsed(void);
void	    FreeNames(void);
int	    InChar(FILE *f);

	/* implicitatt.c */

void	    ImplicitAtt(FILE *Nf);
void	    ReadDefinition(FILE *f);
void	    Append(char c);
Boolean	    Expression(void);
Boolean	    Conjunct(void);
Boolean	    SExpression(void);
Boolean	    AExpression(void);
Boolean	    Term(void);
Boolean	    Factor(void);
Boolean	    Primary(void);
Boolean	    Atom(void);
Boolean	    Find(String S);
int	    FindOne(String *Alt);
Attribute   FindAttName(void);
void	    DefSyntaxError(String Msg);
void	    DefSemanticsError(int Fi, String Msg, int OpCode);
void	    Dump(char OpCode, ContValue F, String S, int Fi);
void	    DumpOp(char OpCode, int Fi);
Boolean	    UpdateTStack(char OpCode, ContValue F, String S, int Fi);
AttValue    EvaluateDef(Definition D, DataRec Case);

	/* getdata.c */

void	    GetData(FILE *Df, Boolean Train, Boolean AllowUnknownClass);
DataRec	    GetDataRec(FILE *Df, Boolean Train);
CaseNo	    CountData(FILE *Df);
int	    StoreIVal(String s);
void	    FreeData(void);
void	    CheckValue(DataRec Case, Attribute Att);

	/* mcost.c */

void	    GetMCosts(FILE *f);

	/* attwinnow.c */

void	    WinnowAtts(void);
float	    TrialTreeCost(Boolean FirstTime);
float	    ErrCost(Tree T, CaseNo Fp, CaseNo Lp);
void	    ScanTree(Tree T, Boolean *Used);

	/* formtree.c */

void	    InitialiseTreeData(void);
void	    FreeTreeData(void);
void	    SetMinGainThresh(void);
void	    FormTree(CaseNo, CaseNo, int, Tree *);
void	    SampleEstimate(CaseNo Fp, CaseNo Lp, CaseCount Cases);
void	    Sample(CaseNo Fp, CaseNo Lp, CaseNo N);
Attribute   ChooseSplit(CaseNo Fp, CaseNo Lp, CaseCount Cases, Boolean Sampled);
void	    ProcessQueue(CaseNo WFp, CaseNo WLp, CaseCount WCases);
Attribute   FindBestAtt(CaseCount Cases);
void	    EvalDiscrSplit(Attribute Att, CaseCount Cases);
CaseNo	    Group(DiscrValue, CaseNo, CaseNo, Tree);
CaseCount   SumWeights(CaseNo, CaseNo);
CaseCount   SumNocostWeights(CaseNo, CaseNo);
void	    FindClassFreq(double [], CaseNo, CaseNo);
void	    FindAllFreq(CaseNo, CaseNo);
void	    Divide(Tree Node, CaseNo Fp, CaseNo Lp, int Level);

	/* discr.c */

void	    EvalDiscreteAtt(Attribute Att, CaseCount Cases);
void	    EvalOrderedAtt(Attribute Att, CaseCount Cases);
void	    SetDiscrFreq(Attribute Att);
double	    DiscrKnownBaseInfo(CaseCount KnownCases, DiscrValue MaxVal);
void	    DiscreteTest(Tree Node, Attribute Att);

	/* contin.c */

void	    EvalContinuousAtt(Attribute Att, CaseNo Fp, CaseNo Lp);
void	    EstimateMaxGR(Attribute Att, CaseNo Fp, CaseNo Lp);
void	    PrepareForContin(Attribute Att, CaseNo Fp, CaseNo Lp);
CaseNo	    PrepareForScan(CaseNo Lp);
void	    ContinTest(Tree Node, Attribute Att);
void	    AdjustAllThresholds(Tree T);
void	    AdjustThresholds(Tree T, Attribute Att, CaseNo *Ep);
ContValue   GreatestValueBelow(ContValue Th, CaseNo *Ep);

	/* info.c */

double	    ComputeGain(double BaseInfo, float UnknFrac, DiscrValue MaxVal,
			CaseCount TotalCases);
double	    TotalInfo(double V[], DiscrValue MinVal, DiscrValue MaxVal);
void	    PrintDistribution(Attribute Att, DiscrValue MinVal,
			DiscrValue MaxVal, double **Freq, double *ValFreq,
			Boolean ShowNames);

	/* subset.c */

void	    InitialiseBellNumbers(void);
void	    EvalSubset(Attribute Att, CaseCount Cases);
void	    Merge(DiscrValue x, DiscrValue y, CaseCount Cases);
void	    EvaluatePair(DiscrValue x, DiscrValue y, CaseCount Cases);
void	    PrintSubset(Attribute Att, Set Ss);
void	    SubsetTest(Tree Node, Attribute Att);
Boolean	    SameDistribution(DiscrValue V1, DiscrValue V2);
void	    AddBlock(DiscrValue V1, DiscrValue V2);
void	    AddBlock(DiscrValue V1, DiscrValue V2);
void	    MoveBlock(DiscrValue V1, DiscrValue V2);

	/* prune.c */

void	    Prune(Tree T);
void	    EstimateErrs(Tree T, CaseNo Fp, CaseNo Lp, int Sh, int Flags);
void	    GlobalPrune(Tree T);
void	    FindMinCC(Tree T);
void	    InsertParents(Tree T, Tree P);
void	    CheckSubsets(Tree T, Boolean);
void	    InitialiseExtraErrs(void);
float	    ExtraErrs(CaseCount N, CaseCount E, ClassNo C);
float	    RawExtraErrs(CaseCount N, CaseCount E);
void	    RestoreDistribs(Tree T);
void	    CompressBranches(Tree T);
void	    SetGlobalUnitWeights(int LocalFlag);

	/* p-thresh.c */

void	    SoftenThresh(Tree T);
void	    ResubErrs(Tree T, CaseNo Fp, CaseNo Lp);
void	    FindBounds(Tree T, CaseNo Fp, CaseNo Lp);

	/* classify.c */

ClassNo	    TreeClassify(DataRec Case, Tree DecisionTree);
void	    FollowAllBranches(DataRec Case, Tree T, float Fraction);
ClassNo	    RuleClassify(DataRec Case, CRuleSet RS);
int	    FindOutcome(DataRec Case, Condition OneCond);
Boolean	    Matches(CRule R, DataRec Case);
void	    CheckActiveSpace(int N);
void	    MarkActive(RuleTree RT, DataRec Case);
void	    SortActive(void);
void	    CheckUtilityBand(int *u, RuleNo r, ClassNo Actual, ClassNo Default);
ClassNo	    BoostClassify(DataRec Case, int MaxTrial);
ClassNo	    SelectClass(ClassNo Default, Boolean UseCosts);
ClassNo	    Classify(DataRec Case);
float	    Interpolate(Tree T, ContValue Val);

	/* special case for dual-purpose routines  */

void	    FindLeaf(DataRec Case, Tree T, Tree PT, float Wt);
Boolean	    Satisfies(DataRec Case, Condition OneCond);

	/* sort.c */

void	    Quicksort(CaseNo Fp, CaseNo Lp, Attribute Att);
void	    Cachesort(CaseNo Fp, CaseNo Lp, SortRec *SRec);

	/* trees.c */

void	    FindDepth(Tree T);
void	    PrintTree(Tree T, String Title);
void	    Show(Tree T, int Sh);
void	    ShowBranch(int Sh, Tree T, DiscrValue v, DiscrValue BrNo);
DiscrValue  Elements(Attribute Att, Set S, DiscrValue *Last);
int	    MaxLine(Tree SubTree);
void	    Indent(int Sh, int BrNo);
void	    FreeTree(Tree T);
Tree	    Leaf(double *Freq, ClassNo NodeClass, CaseCount Cases,
		 CaseCount Errors);
void	    Sprout(Tree T, DiscrValue Branches);
void	    UnSprout(Tree T);
int	    TreeSize(Tree T);
int	    ExpandedLeafCount(Tree T);
int	    TreeDepth(Tree T);
Tree	    CopyTree(Tree T);

	/* utility.c */

void	    PrintHeader(String Title);
char	    ProcessOption(int Argc, char **Argv, char *Str);
void	    *Pmalloc(size_t Bytes);
void	    *Prealloc(void *Present, size_t Bytes);
void	    *Pcalloc(size_t Number, unsigned int Size);
void	    FreeVector(void **V, int First, int Last);
DataRec	    NewCase(void);
void	    FreeCases(void);
void	    FreeLastCase(DataRec Case);
double	    KRandom(void);
void	    ResetKR(int KRInit);
void	    Error(int ErrNo, String S1, String S2);
String	    CaseLabel(CaseNo N);
FILE *	    GetFile(String Extension, String RW);
double	    ExecTime(void);
int	    Denominator(ContValue Val);
int	    GetInt(String S, int N);
int	    DateToDay(String DS);
void	    DayToDate(int DI, String Date);
int	    TimeToSecs(String TS);
void	    SecsToTime(int Secs, String Time);
void	    SetTSBase(int y);
int	    TStampToMins(String TS);
void	    Check(float Val, float Low, float High);
void	    CValToStr(ContValue CV, Attribute Att, String DS);
double	    rint(double v);
void	    Cleanup(void);
#ifdef UTF8
int	    UTF8CharWidth(unsigned char *U);
int	    wcwidth(wchar_t ucs);
int	    wcswidth(const wchar_t *pwcs, size_t n);
#endif

	/* confmat.c */

void	    PrintConfusionMatrix(CaseNo *ConfusionMat);
void	    PrintErrorBreakdown(CaseNo *ConfusionMat);
void	    PrintUsageInfo(CaseNo *Usage);

	/* formrules.c */

CRuleSet    FormRules(Tree T);
void	    Scan(Tree T);
void	    SetupNCost(void);
void	    PushCondition(void);
void	    PopCondition(void);
void	    PruneRule(Condition Cond[], ClassNo TargetClass);
void	    ProcessLists(void);
void	    AddToList(CaseNo *List, CaseNo N);
void	    DeleteFromList(CaseNo *Before, CaseNo N);
int	    SingleFail(CaseNo i);
void	    Increment(int d, CaseNo i, double *Total, double *Errors);
void	    FreeFormRuleData(void);

	/* rules.c */

Boolean	    NewRule(Condition Cond[], int NConds, ClassNo TargetClass,
		    Boolean *Deleted, CRule Existing,
		    CaseCount Cover, CaseCount Correct, float Prior);
void	    ListSort(int *L, int Fp, int Lp);
Byte	    *Compress(int *L);
void	    Uncompress(Byte *CL, int *UCL);
Boolean	    SameRule(RuleNo r, Condition Cond[], int NConds,
		     ClassNo TargetClass);
void	    FreeRule(CRule R);
void	    FreeRules(CRuleSet RS);
void	    PrintRules(CRuleSet, String);
void	    PrintRule(CRule R);
void	    PrintCondition(Condition C);

	/* siftrules.c */

void	    SiftRules(float EstErrRate);
void	    InvertFires(void);
void	    FindTestCodes(void);
float	    CondBits(Condition C);
void	    SetInitialTheory(void);
void	    CoverClass(ClassNo Target);
double	    MessageLength(RuleNo NR, double RuleBits, float Errs);
void	    HillClimb(void);
void	    InitialiseVotes(void);
void	    CountVotes(CaseNo i);
void	    UpdateDeltaErrs(CaseNo i, double Delta, RuleNo Toggle);
CaseCount   CalculateDeltaErrs(void);
void	    PruneSubsets(void);
void	    SetDefaultClass(void);
void	    SwapRule(RuleNo A, RuleNo B);
int	    OrderByUtility(void);
int	    OrderByClass(void);
void	    OrderRules(void);
void	    GenerateLogs(int MaxN);
void	    FreeSiftRuleData(void);

	/* ruletree.c */

void	    ConstructRuleTree(CRuleSet RS);
void	    SetTestIndex(Condition C);
RuleTree    GrowRT(RuleNo *RR, int RRN, CRule *Rule);
int	    DesiredOutcome(CRule R, int TI);
int	    SelectTest(RuleNo *RR, int RRN, CRule *Rule);
void	    FreeRuleTree(RuleTree RT);

	/* modelfiles.c */

void	    CheckFile(String Extension, Boolean Write);
void	    WriteFilePrefix(String Extension);
void	    ReadFilePrefix(String Extension);
void	    SaveDiscreteNames(void);
void	    SaveTree(Tree T, String Extension);
void	    OutTree(Tree T);
void	    SaveRules(CRuleSet RS, String Extension);
void	    AsciiOut(String Pre, String S);
void	    ReadHeader(void);
Tree	    GetTree(String Extension);
Tree	    InTree(void);
CRuleSet    GetRules(String Extension);
CRuleSet    InRules(void);
CRule	    InRule(void);
Condition   InCondition(void);
int	    ReadProp(char *Delim);
String	    RemoveQuotes(String S);
Set	    MakeSubset(Attribute Att);
void	    StreamIn(String S, int n);

	/* update.c (Unix) or winmain.c (WIN32) */

void	    NotifyStage(int);
void	    Progress(float);

	/* xval.c */

void	    CrossVal(void);
void	    Prepare(void);
void	    Shuffle(int *Vec);
void	    Summary(void);
float	    SE(float sum, float sumsq, int no);



