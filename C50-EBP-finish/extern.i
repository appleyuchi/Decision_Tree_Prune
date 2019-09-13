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



extern	int		VERBOSITY,
			TRIALS,
			FOLDS,
			UTILITY,
			NCPU;

extern	Boolean		SUBSET,
			BOOST,
			PROBTHRESH,
			RULES,
			XVAL,
			NOCOSTS,
			WINNOW,
			GLOBAL;

extern	CaseCount	MINITEMS,
			LEAFRATIO;

extern	float		CF,
			SAMPLE;

extern	Boolean		LOCK;

extern	Attribute	ClassAtt,
			LabelAtt,
			CWtAtt;

extern double		AvCWt;

extern	String		*ClassName,
			*AttName,
			**AttValName;

extern	char 		*IgnoredVals;
extern	int		IValsSize,
			IValsOffset;

extern	int		MaxAtt,
			MaxClass,
			MaxDiscrVal,
			MaxLabel,
			LineNo,
			ErrMsgs,
			AttExIn,
			TSBase;

extern	DiscrValue	*MaxAttVal;

extern	char		*SpecialStatus;

extern	Definition	*AttDef;
extern	Attribute	**AttDefUses;

extern	Boolean		*SomeMiss,
			*SomeNA,
			Winnowed;

extern	ContValue	*ClassThresh;

extern	CaseNo		MaxCase;

extern	DataRec		*Case;

extern	DataRec		*SaveCase;

extern	String		FileStem;

extern	Tree		*Raw,
			*Pruned,
			WTree;

extern	float		Confidence,
			SampleFrac,
			*Vote,
			*BVoteBlock,
			**MCost,
			**NCost,
			*WeightMul;

extern	CRule		*MostSpec;

extern	Boolean		UnitWeights,
			CostWeights;

extern	int		Trial,
			MaxTree;

extern	ClassNo		*TrialPred;

extern double		*ClassFreq,
			**DFreq;

extern	float		*Gain,
			*Info,
			*EstMaxGR,
			*ClassSum;

extern	ContValue	*Bar;

extern	double		GlobalBaseInfo,
			**Bell;

extern	Byte		*Tested;

extern	Set		**Subset;
extern	int		*Subsets;

extern	EnvRec		GEnv;

extern	CRule		*Rule;

extern	RuleNo		NRules,
			RuleSpace;

extern	CRuleSet	 *RuleSet;

extern	ClassNo		Default;

extern	Byte		**Fires,
			*CBuffer;

extern	int		*CovBy,
			*List;

extern	float		AttTestBits,
			*BranchBits;
extern	int		*AttValues,
			*PossibleCuts;

extern	double		*LogCaseNo,
			*LogFact;

extern	int		*UtilErr,
			*UtilBand;
extern	double		*UtilCost;

extern	int		KRInit,
			Now;

extern	FILE		*TRf;
extern	char		Fn[500];

extern	FILE  		*Of;

