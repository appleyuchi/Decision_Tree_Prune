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
/*		General data for C5.0					 */
/*		---------------------					 */
/*									 */
/*************************************************************************/

#include "defns.i"

/*************************************************************************/
/*									 */
/*		Parameters etc						 */
/*									 */
/*************************************************************************/

int		VERBOSITY=0,	/* verbosity level (0 = none) */
		TRIALS=1,	/* number of trees to be grown */
		FOLDS=10,	/* crossvalidation folds */
		UTILITY=0;	/* rule utility bands */

Boolean		SUBSET=0,	/* subset tests allowed */
		BOOST=0,	/* boosting invoked */
		PROBTHRESH=0,	/* to use soft thresholds */
		RULES=0,	/* rule-based classifiers */
		XVAL=0,		/* perform crossvalidation */
		NOCOSTS=0,	/* ignoring costs */
		WINNOW=0,	/* attribute winnowing */
		GLOBAL=1;	/* use global pruning for trees */

CaseCount	MINITEMS=2,	/* minimum cases each side of a cut */
		LEAFRATIO=0;	/* leaves per case for boosting */

float		CF=0.25,	/* confidence limit for tree pruning */
		SAMPLE=0.0;	/* sample training proportion */

Boolean		LOCK=false;	/* sample locked */


/*************************************************************************/
/*									 */
/*		Attributes and data					 */
/*									 */
/*************************************************************************/

Attribute	ClassAtt=0,	/* attribute to use as class */
		LabelAtt=0,	/* attribute to use as case ID */
		CWtAtt=0;	/* attribute to use for case weight */

double		AvCWt;		/* average case weight */

String		*ClassName=0,	/* class names */
		*AttName=0,	/* att names */
		**AttValName=0;	/* att value names */

char		*IgnoredVals=0;	/* values of labels and atts marked ignore */
int		IValsSize=0,	/* size of above */
		IValsOffset=0;	/* index of first free char */

int		MaxAtt,		/* max att number */
		MaxClass,	/* max class number */
		MaxDiscrVal=3,	/* max discrete values for any att */
		MaxLabel=0,	/* max characters in case label */
		LineNo=0,	/* input line number */
		ErrMsgs=0,	/* errors found */
		AttExIn=0,	/* attribute exclusions/inclusions */
		TSBase=0;	/* base day for time stamps */

DiscrValue	*MaxAttVal=0;	/* number of values for each att */

char		*SpecialStatus=0;/* special att treatment */

Definition	*AttDef=0;	/* definitions of implicit atts */
Attribute	**AttDefUses=0;	/* list of attributes used by definition */

Boolean		*SomeMiss=Nil,	/* att has missing values */
		*SomeNA=Nil,	/* att has N/A values */
		Winnowed=0;	/* atts have been winnowed */

ContValue	*ClassThresh=0;	/* thresholded class attribute */

CaseNo		MaxCase=-1;	/* max data case number */

DataRec		*Case=0;	/* data cases */

DataRec		*SaveCase=0;

String		FileStem="undefined";

/*************************************************************************/
/*									 */
/*		Trees							 */
/*									 */
/*************************************************************************/

Tree		*Raw=0,		/* unpruned trees */
		*Pruned=0,	/* pruned trees */
		WTree=0;	/* winnow tree */

float		Confidence,	/* set by classify() */
		SampleFrac=1,	/* fraction used when sampling */
		*Vote=0,	/* total votes for classes */
		*BVoteBlock=0,	/* boost voting block */
		**MCost=0,	/* misclass cost [pred][real] */
		**NCost=0,	/* normalised MCost used for rules */
		*WeightMul=0;	/* prior adjustment factor */

CRule		*MostSpec=0;	/* most specific rule for each class */

Boolean		UnitWeights=1,	/* all weights are 1.0 */
		CostWeights=0;	/* reweight cases for costs */

int		Trial,		/* trial number for boosting */
		MaxTree=0;	/* max tree grown */

ClassNo		*TrialPred=0;	/* predictions for each boost trial */

double		*ClassFreq=0,	/* ClassFreq[c] = # cases of class c */
		**DFreq=0;	/* DFreq[a][c*x] = Freq[][] for attribute a */

float		*Gain=0,	/* Gain[a] = info gain by split on att a */
		*Info=0,	/* Info[a] = max info from split on att a */
		*EstMaxGR=0,	/* EstMaxGR[a] = est max GR from folit on a */
		*ClassSum=0;	/* class weights during classification */

ContValue	*Bar=0;		/* Bar[a]  = best threshold for contin att a */

double		GlobalBaseInfo,	/* base information before split */
		**Bell=0;	/* table of Bell numbers for subsets */

Byte		*Tested=0;	/* Tested[a] = att a already tested */

Set		**Subset=0;	/* Subset[a][s] = subset s for att a */
int		*Subsets=0;	/* Subsets[a] = no. subsets for att a */

EnvRec		GEnv;		/* environment block */

/*************************************************************************/
/*									 */
/*		Rules							 */
/*									 */
/*************************************************************************/

CRule		*Rule=0;	/* current rules */

RuleNo		NRules,		/* number of rules */
		RuleSpace;	/* space currently allocated for rules */

CRuleSet	*RuleSet=0;	/* rulesets */

ClassNo		Default;	/* default class associated with ruleset or
				   boosted classifier */

Byte		**Fires=Nil,	/* Fires[r][*] = cases covered by rule r */
		*CBuffer=Nil;	/* buffer for compressing lists */

int		*CovBy=Nil,	/* entry numbers for Fires inverse */
		*List=Nil;	/* temporary list of cases or rules */

float		AttTestBits,	/* average bits to encode tested attribute */
		*BranchBits=0;	/* ditto attribute value */
int		*AttValues=0,	/* number of attribute values in the data */
		*PossibleCuts=0;/* number of thresholds for an attribute */

double		*LogCaseNo=0,	/* LogCaseNo[i] = log2(i) */
		*LogFact=0;	/* LogFact[i] = log2(i!) */

int		*UtilErr=0,	/* error by utility band */
		*UtilBand=0;	/* last rule in each band */
double		*UtilCost=0;	/* cost ditto */


/*************************************************************************/
/*									 */
/*		Misc							 */
/*									 */
/*************************************************************************/

int		KRInit=0,	/* KRandom initializer for SAMPLE */
		Now=0;		/* current stage */

FILE		*TRf=0;		/* file pointer for tree and rule i/o */
char		Fn[500];	/* file name */

FILE  		*Of=0;		/* output file */

