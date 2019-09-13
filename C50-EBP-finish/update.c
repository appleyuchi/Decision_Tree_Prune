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
/*		Routines that provide information on progress		 */
/*              ---------------------------------------------		 */
/*									 */
/*************************************************************************/


#include "defns.i"
#include "extern.i"


FILE	*Uf=0;			/* File to which update info written  */


/*************************************************************************/
/*									 */
/*	There are several stages (see messages in Progress() below)	 */
/*	Record stage and open update file if necessary			 */
/*									 */
/*************************************************************************/


void NotifyStage(int S)
/*   -----------  */
{
    Now = S;
    if ( S == 1 )
    {
	if ( ! (Uf = GetFile(".tmp", "w")) ) Error(NOFILE, "", E_ForWrite);
    }
}



/*************************************************************************/
/*									 */
/*	Print progress message.  This routine is called in two ways:	 */
/*	  *  negative Delta = measure of total effort required for stage */
/*	  *  positive Delta = increment since last call			 */
/*									 */
/*************************************************************************/


void Progress(float Delta)
/*   --------  */
{
    static float Total, Current=0;
    static int   Twentieth=0, LastStage=0;
    int		 p;
    static char *Message[]={ "",
			     "Reading training data      ",
			     "Winnowing attributes       ",
			     "Constructing decision tree ",
			     "Simplifying decision tree  ",
			     "Forming rules              ",
			     "Selecting final rules      ",
			     "Evaluating on training data",
			     "Reading test data          ",
			     "Evaluating on test data    ",
			     "Cleaning up                ",
			     "Allocating tables          ",
			     "Preparing results          " },
		Tell[]={ 0,0,0,1,1,1,1,0,0,0,0,0,0 },

		*Done=">>>>>>>>>>>>>>>>>>>>",
		*ToDo="....................";

    if ( LastStage == Now && ! Tell[Now] )
    {
	return;
    }

    LastStage = Now;

    if ( Delta <= -1 )
    {
	Total = -Delta;
	Current = 0;
	Twentieth = -1;
    }
    else
    {
	Current = Min(Total, Current + Delta);
    }

    if ( (p = rint((20.0 * Current) / Total)) != Twentieth )
    {
	Twentieth = p;
assert(p >= 0 && p <= 20);
	fprintf(Uf, "%s", Message[Now]);
	if ( Tell[Now] )
	{
	    fprintf(Uf, "  %s%s  (%d %s)",
			Done + (20 - Twentieth), ToDo + Twentieth,
			(int) (Current+0.5),
			( Now == SIFTRULES ?
			    "refinements" : "cases covered" ));
	}
	fprintf(Uf, "\n");
	fflush(Uf);
    }
}
