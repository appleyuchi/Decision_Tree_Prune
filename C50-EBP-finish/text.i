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
/*	Text strings for UTF-8 internationalization			 */
/*	-------------------------------------------			 */
/*									 */
/*************************************************************************/



	/*  General stuff  */

//#define UTF8		 	/* uncomment if using UTF-8 */

#ifdef UTF8
#define	 CharWidth(S)		UTF8CharWidth(S)
#else
#define	 CharWidth(S)		(int) strlen(S)
#endif


	/*  Strings with width/format restrictions  */
	/*  (W = width when printed, C = centered)  */

#define	 F_Fold			"Fold"				/* W<8 */
#define	 F_UFold		"----"				/* W same */
#define	 F_Trial		"Trial"				/* W<8 */
#define	 F_UTrial		"-----"				/* W same */
#define	 F_DecisionTree16	"  Decision Tree "		/* W=16C */
#define	 F_SizeErrors		"Size      Errors"		/* W=16 */
#define	 F_DecisionTree23	"     Decision Tree     "	/* W=23C */
#define	 F_SizeErrorsCost	"Size      Errors   Cost"	/* W=23 */
#define	 F_Rules16		"      Rules     "		/* W=16C */
#define	 F_NoErrors		"  No      Errors"		/* W=16 */
#define	 F_Rules23		"         Rules         "	/* W=23C */
#define	 F_NoErrorsCost		"  No      Errors   Cost"	/* W=23 */
#define	 F_Rules		"Rules"				/* W<8 */
#define	 F_URules		"-----"				/* W same */
#define	 F_Errors		"Errors"			/* W<8 */
#define	 F_UErrors		"------"			/* W same */
#define	 F_Cost			"Cost"				/* W<8 */
#define	 F_UCost		"----"				/* W same */
#define	 F_Boost		"boost"				/* W<8 */


	/*  Strings of arbitrary length  */

#define	 T_See5			"See5"
#define	 T_C50			"C5.0"
#define	 TX_Release(n)		"Release " n

#define	 T_OptHeader		"\n    Options:\n"
#define	 T_OptApplication	"\tApplication `%s'\n"
#define	 T_OptBoost		"\tBoosted classifiers\n"
#define	 T_OptProbThresh	"\tProbability thresholds\n"
#define	 T_OptTrials		"\t%d boosting trials\n"
#define	 T_OptSubsets		"\tTests on discrete attribute groups\n"
#define	 T_OptMinCases		"\tTests require 2 branches with >=%g cases\n"
#define	 T_OptCF		"\tPruning confidence level %g%%\n"
#define	 T_OptRules		"\tRule-based classifiers\n"
#define	 T_OptSampling		"\tUse %g%% of data for training\n"
#define	 T_OptSeed		"\tRandom seed %d\n"
#define	 T_OptUtility		"\tRule utility ordering (1/%d's)\n"
#define	 T_OptNoCosts		"\tFocus on errors (ignore costs file)\n"
#define	 T_OptWinnow		"\tWinnow attributes\n"
#define	 T_OptNoGlobal		"\tDo not use global tree pruning\n"
#define	 T_OptXval		"\tCross-validate using %d folds\n"
#define	 T_UnregnizedOpt	"\n    **  Unrecognised option %s %s\n"
#define	 T_SummaryOpts		"    **  Summary of options for c5.0:\n"
#define	 T_ListOpts		"\t-f <filestem>\tapplication filestem\n"\
				"\t-r\t\tuse rule-based classifiers\n"\
				"\t-u <bands>\torder rules by utility in"\
					" bands\n"\
				"\t-w\t\tinvoke attribute winnowing\n"\
				"\t-b\t\tinvoke boosting\n"\
				"\t-t <trials>\tnumber of boosting trials\n"\
				"\t-p\t\tuse soft thresholds\n"\
				"\t-e\t\tfocus on errors (ignore costs file)\n"\
				"\t-s\t\tfind subset tests for discrete atts\n"\
				"\t-g\t\tdo not use global tree pruning\n"\
				"\t-m <cases>\trestrict allowable splits\n"\
				"\t-c <percent>\tconfidence level (CF) for"\
					" pruning\n"\
				"\t-S <percent>\ttraining sample percentage\n"\
				"\t-X <folds>\tcross-validate\n"\
				"\t-I <integer>\trandom seed for sampling"\
					" and cross-validation\n"\
				"\t-h\t\tprint this message\n"
#define	 T_UBWarn		"    **  Warning (-u): rule ordering "\
					"has no effect on boosting\n"
#define	 T_ClassVar		"\nClass specified by attribute `%s'\n"
#define	 TX_ReadData(c,a,f)	"\nRead %d cases (%d attributes) from"\
					" %s.data\n", c, a, f
#define	 TX_ReadTest(c,f)	"Read %d cases from %s.test\n", c, f
#define	 T_ReadCosts		"Read misclassification costs from %s.costs\n"
#define	 T_CWtAtt		"Using relative case weighting\n"
#define	 T_AttributesIn		"\nAttributes included:\n"
#define	 T_AttributesOut	"\nAttributes excluded:\n"
#define	 T_AttributesWinnowed	"\n%d attribute%s winnowed\n"
#define	 T_EstImportance	"Estimated importance of remaining"\
					" attributes:\n\n"
#define	 T_NoWinnow		"\nNo attributes winnowed\n"
#define	 T_EvalTrain		"\n\nEvaluation on training data (%d cases):\n"
#define	 T_Usage		"\n\n\tAttribute usage:\n\n"
#define	 T_EvalTest		"\nEvaluation on test data (%d cases):\n"
#define	 T_Time			"\n\nTime: %.1f secs\n"

#define	 T_IgnoreBadClass	"*** ignoring cases with bad or unknown class\n"

#define	 T_Subtree		"\nSubTree [S%d]\n"
#define	 T_ElementOf		"in"
#define	 T_InRange		"in"
#define	 T_RuleHeader		"\nRule "
#define	 T_RuleLift		", lift %.1f)\n"
#define	 T_IsUnknown		" is unknown\n"

#define	 TX_Reduced1(t)		( (t) > 1 ?\
				   "\n*** boosting reduced to %d trials since"\
					" last classifier is very accurate\n" :\
				   "\n*** boosting reduced to %d trial since"\
					" last classifier is very accurate\n" )
#define	 TX_Reduced2(t)		( (t) > 1 ?\
				   "\n*** boosting reduced to %d trials since"\
					" last classifier is very inaccurate\n" :\
				   "\n*** boosting reduced to %d trial since"\
					" last classifier is very inaccurate\n" )
#define	 T_Abandoned		"\n*** boosting abandoned (too few classifiers)\n"
#define	 T_BoostingUnhelpful	"\n*** warning: boosting may be unhelpful\n"
#define	 T_Composite		"Composite ruleset:"
#define	 T_Tree			"Decision tree:"
#define	 T_Rules		"Rules:"

#define	 T_Default_class	"Default class"
#define	 T_boost		"boost"
#define	 T_composite_ruleset	"composite ruleset"
#define	 T_Rule_utility_summary	"Rule utility summary"
#define	 T_class		"class"
#define	 T_classified_as	"classified as"
#define	 T_Summary		"Summary"
#define	 T_FoldsReduced		"\n*** folds reduced to number of cases\n"
#define	 T_EvalHoldOut		"\nEvaluation on hold-out data (%d cases):\n"
#define	 T_Summary		"Summary"
#define	 T_Fold			"Fold"
#define	 T_Mean			"Mean"
#define	 T_SE			"SE"

#define	 TX_Line(l,f)		"*** line %d of `%s': ", l, f
#define	 E_NOFILE(f,e)		"cannot open file %s%s\n", f, e
#define	 E_ForWrite		" for writing"
#define	 E_BADCLASSTHRESH	"bad class threshold `%s'\n"
#define	 E_LEQCLASSTHRESH	"class threshold `%s' <= previous threshold\n"
#define	 E_BADATTNAME		"`:' or `:=' expected after attribute name"\
					" `%s'\n"
#define	 E_EOFINATT		"unexpected eof while reading attribute `%s'\n"
#define	 E_SINGLEATTVAL(a,v)	"attribute `%s' has only one value `%s'\n",\
					a, v
#define	 E_DUPATTNAME		"multiple attributes with name `%s'\n"
#define	 E_CWTATTERR		"case weight attribute must be continuous\n"
#define	 E_BADATTVAL(v,a)	"bad value of `%s' for attribute `%s'\n", v, a
#define	 E_BADNUMBER(a)		"value of `%s' changed to `?'\n", a
#define	 E_BADCLASS		"bad class value `%s'\n"
#define	 E_BADCOSTCLASS		"bad class `%s'\n"
#define	 E_BADCOST		"bad cost value `%s'\n"
#define	 E_NOMEM		"unable to allocate sufficient memory\n"
#define	 E_TOOMANYVALS(a,n)	"too many values for attribute `%s'"\
					" (max %d)\n", a, n
#define	 E_BADDISCRETE		"bad number of discrete values for attribute"\
					" `%s'\n"
#define	 E_NOTARGET		"target attribute `%s' not found or"\
					" type `ignore'\n"
#define	 E_BADCTARGET		"target attribute `%s' must be"\
					" type `continuous'\n"
#define	 E_BADDTARGET		"target attribute `%s' must be specified by"\
					" a list of discrete values\n"
#define	 E_LONGNAME		"overlength name: check data file formats\n"
#define	 E_HITEOF		"unexpected end of file\n"
#define	 E_MISSNAME		"missing name or value before `%s'\n"
#define	 E_BADTSTMP(d,a)	"bad timestamp `%s' for attribute `%s'\n", d, a
#define	 E_BADDATE(d,a)		"bad date `%s' for attribute `%s'\n", d, a
#define	 E_BADTIME(d,a)		"bad time `%s' for attribute `%s'\n", d, a
#define	 E_UNKNOWNATT		"unknown attribute name `%s'\n"
#define	 E_BADDEF1(a,s,x)	"in definition of attribute `%s':\n"\
					"\tat `%.12s': expect %s\n", a, s, x
#define	 E_BADDEF2(a,s,x)	"in definition of attribute `%s':\n"\
					"\t`%s': %s\n", a, s, x
#define	 E_SAMEATT(a,b)		"[warning] attribute `%s' is identical to"\
					" attribute `%s'\n", a, b
#define	 E_BADDEF3		"cannot define target attribute `%s'\n"
#define	 E_BADDEF4		"[warning] target attribute appears in"\
					" definition of attribute `%s'\n"
#define	 EX_MODELFILE(f)	"file %s incompatible with .names file\n", f
#define	 E_MFATT		"undefined or excluded attribute"
#define	 E_MFATTVAL		"undefined attribute value"
#define	 E_MFCLASS		"undefined class"
#define	 E_MFEOF		"unexpected eof"
#define	 T_ErrorLimit		"\nError limit exceeded\n"
#define	 TX_IllegalValue(v,l,h)	"\t** illegal value %g -- "\
				"should be between %g and %g\n", v, l, h
