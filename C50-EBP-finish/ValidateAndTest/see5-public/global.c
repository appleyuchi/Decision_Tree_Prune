/*************************************************************************/
/*									 */
/*	 Source code for use with See5/C5.0 Release 2.11a		 */
/*	 ------------------------------------------------		 */
/*		        Copyright RuleQuest Research 2019		 */
/*									 */
/*	This code is provided "as is" without warranty of any kind,	 */
/*	either express or implied.  All use is at your own risk.	 */
/*									 */
/*************************************************************************/


/*************************************************************************/
/*									 */
/*		Parameters etc						 */
/*									 */
/*************************************************************************/

int		TRIALS=1,	/* number of trees to be grown */
		Trial;		/* trial number for boosting */

Boolean		RULES=0,	/* rule-based classifiers */
		RULESUSED=0;	/* list applicable rules */


/*************************************************************************/
/*									 */
/*		Attributes and data					 */
/*									 */
/*************************************************************************/

Attribute	ClassAtt=0,	/* attribute to use as class */
		LabelAtt,	/* attribute to use as case ID */
		CWtAtt;		/* attribute to use for case weight */

String		*ClassName=0,	/* class names */
		*AttName=0,	/* att names */
		**AttValName=0;	/* att value names */

char		*IgnoredVals=0;	/* values of labels and atts marked ignore */
int		IValsSize=0,	/* size of above */
		IValsOffset=0;	/* index of first free char */

int		MaxAtt,		/* max att number */
		MaxClass=0,	/* max class number */
		AttExIn=0,	/* attribute exclusions/inclusions */
		LineNo=0,	/* input line number */
		ErrMsgs=0,	/* errors found */
		Delimiter,	/* character at end of name */
		TSBase=0;	/* base day for time stamps */

DiscrValue	*MaxAttVal=0;	/* number of values for each att */

ContValue	*ClassThresh=0;	/* thresholded class attribute */

char		*SpecialStatus=0;/* special att treatment */

Definition	*AttDef=0;	/* definitions of implicit atts */

Boolean		*SomeMiss=Nil,	/* att has missing values */
		*SomeNA=Nil;	/* att has N/A values */

String		FileStem="crx";

/*************************************************************************/
/*									 */
/*		Classification environment				 */
/*									 */
/*************************************************************************/

CEnvRec		*GCEnv;		/* used by classification routines */

/*************************************************************************/
/*									 */
/*		Trees							 */
/*									 */
/*************************************************************************/

Tree		*Pruned=0;	/* decision trees */

ClassNo		*TrialPred=0;	/* predictions for each boost trial */

float		**MCost=0;	/* misclass cost [pred][real] */

/*************************************************************************/
/*									 */
/*		Rules							 */
/*									 */
/*************************************************************************/


CRuleSet	*RuleSet=0;	/* rulesets */

ClassNo		Default;	/* default class associated with ruleset or
				   boosted classifier */


/*************************************************************************/
/*									 */
/*		Misc							 */
/*									 */
/*************************************************************************/

FILE		*TRf=0;		/* file pointer for tree and rule i/o */
char		Fn[500];	/* file name */
