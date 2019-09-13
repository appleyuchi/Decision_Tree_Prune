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
/*	Sorting utilities						 */
/*	-----------------						 */
/*									 */
/*************************************************************************/


#include "defns.i"
#include "extern.i"

#define SwapSRec(a,b)	{Xab=SRec[a]; SRec[a]=SRec[b]; SRec[b]=Xab;}


/*************************************************************************/
/*									 */
/*	Sort elements Fp to Lp of SRec					 */
/*									 */
/*************************************************************************/


void Cachesort(CaseNo Fp, CaseNo Lp, SortRec *SRec)
/*   ---------  */
{
    CaseNo	i, Middle, High;
    ContValue	Thresh, Val;
    SortRec	Xab;


    while ( Fp < Lp )
    {
	Thresh = SRec[(Fp+Lp) / 2].V;

	/*  Divide elements into three groups:
		Fp .. Middle-1: values < Thresh
		Middle .. High: values = Thresh
		High+1 .. Lp:   values > Thresh  */

	for ( Middle = Fp ; SRec[Middle].V < Thresh ; Middle++ )
	    ;

	for ( High = Lp ; SRec[High].V > Thresh ; High-- )
	    ;

	for ( i = Middle ; i <= High ; )
	{
	    if ( (Val = SRec[i].V) < Thresh )
	    {
		SwapSRec(Middle, i);
		Middle++;
		i++;
	    }
	    else
	    if ( Val > Thresh )
	    {
		SwapSRec(High, i);
		High--;
	    }
	    else
	    {
		i++;
	    }
	}

	/*  Sort the first group  */

	Cachesort(Fp, Middle-1, SRec);

	/*  Continue with the last group  */

	Fp = High+1;
    }
}



/*************************************************************************/
/*									 */
/*	Sort cases from Fp to Lp on attribute Att			 */
/*									 */
/*************************************************************************/


void Quicksort(CaseNo Fp, CaseNo Lp, Attribute Att)
/*   ---------  */
{
    CaseNo	i, Middle, High;
    ContValue	Thresh, Val;

    if ( Fp < Lp )
    {
	Thresh = CVal(Case[(Fp+Lp) / 2], Att);

	/*  Divide cases into three groups:
		Fp .. Middle-1: values < Thresh
		Middle .. High: values = Thresh
		High+1 .. Lp:   values > Thresh  */

	for ( Middle = Fp ; CVal(Case[Middle], Att) < Thresh ; Middle++ )
	    ;

	for ( High = Lp ; CVal(Case[High], Att) > Thresh ; High-- )
	    ;

	for ( i = Middle ; i <= High ; )
	{
	    if ( (Val = CVal(Case[i], Att)) < Thresh )
	    {
		Swap(Middle, i);
		Middle++;
		i++;
	    }
	    else
	    if ( Val > Thresh )
	    {
		Swap(High, i);
		High--;
	    }
	    else
	    {
		i++;
	    }
	}

	/*  Sort the first and third groups  */

	Quicksort(Fp, Middle-1, Att);
	Quicksort(High+1, Lp, Att);
    }
}
