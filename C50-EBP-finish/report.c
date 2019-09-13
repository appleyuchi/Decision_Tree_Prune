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
/*	Program to produce average results from an xval			 */
/*	-----------------------------------------------			 */
/*									 */
/*************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void	PrintSummary(float **Val, int No, char *Title);
float	SE(float sum, float sumsq, int no);

int Boost=0, Composite=0, Costs=0, Rules;

#define	SIZE	0
#define	ERRP	1
#define	COST	2


int main(int argc, char *argv[])
/*  ----  */
{
    char	Line[100], *p;
    int		Cases, Folds, Repeats, f, r, i, N,
		Size=0, Errs=0, Form, OK;
    float	***Raw, **Average=0, FX, Tests, Cost=0;

    sscanf(argv[1], "%d", &Cases);
    sscanf(argv[2], "%d", &Folds);
    sscanf(argv[3], "%d", &Repeats);
    sscanf(argv[4], "%d", &Rules);

    /*  Assemble all data  */

    Raw = (float ***) calloc(Repeats, sizeof(float **));
    if ( Repeats > 1 )
    {
	Average = (float **) calloc(Repeats, sizeof(float *));
    }

    /*  Determine input type from the first line  */

    fgets(Line, 100, stdin);

    /*  Count the numbers on the line  */

    N = 0;
    for ( p = Line ; *p ; )
    {
	if ( isdigit(*p) )
	{
	    N++;
	    while ( isdigit(*p) || *p == '.' ) p++;
	}
	else
	{
	    p++;
	}
    }

    if ( ! memcmp(Line, "boost", 5) )
    {
	Boost = 1;
	Costs = ( N == 3 );
    }
    else
    if ( ! memcmp(Line, "composite", 9) )
    {
	Composite = 1;
	Rules = 0;
	Costs = ( N == 4 );
    }
    else
    {
	Costs = ( N == 4 );
    }
    Form = ( Composite ? 2 + Costs : Costs );

    for ( r = 0 ; r < Repeats ; r++ )
    {
	Raw[r] = (float **) calloc(Folds, sizeof(float *));
	if ( Repeats > 1 )
	{
	    Average[r] = (float *) calloc(3, sizeof(float));
	}

	for ( f = 0 ; f < Folds ; f++ )
	{
	    Raw[r][f] = (float *) calloc(3, sizeof(float));

	    if ( r + f != 0 && ! fgets(Line, 100, stdin) )
	    {
		printf("\nExpecting %d lines\n", Folds * Repeats);
		exit(1);
	    }

	    Tests = Cases / Folds + ( f >= Folds - Cases % Folds);

	    if ( ! memcmp(Line, "boost", 5) )
	    {
		Boost = 1;

		switch ( Form )
		{
		case 0:
		    N = sscanf(Line, "boost %d (%f%%)", &Errs, &FX);
		    OK = ( N == 2 );
		    break;

		case 1:
		    N = sscanf(Line, "boost %d (%f%%) %f", &Errs, &FX, &Cost);
		    OK = ( N == 3 );
		}
	    }
	    else
	    {
		switch ( Form )
		{
		case 0:
		    N = sscanf(Line, "%d %d (%f%%)", &Size, &Errs, &FX);
		    OK = ( N == 3 );
		    break;

		case 1:
		    N = sscanf(Line, "%d %d (%f%%) %f",
				     &Size, &Errs, &FX, &Cost);
		    OK = ( N == 4 );
		    break;

		case 2:
		    N = sscanf(Line+18, "%d %d (%f%%) %f",
					&Size, &Errs, &FX, &Cost);
		    OK = ( N == 4 );
		    break;
		}
	    }

	    if ( ! OK )
	    {
		printf("\nCannot parse line\n\t%s", Line);
		exit(1);
	    }

	    Raw[r][f][SIZE] = Size;
	    Raw[r][f][ERRP] = (100.0 * Errs) / Tests;
	    Raw[r][f][COST] = Cost;

	    if ( Average )
	    {
		for ( i = 0 ; i < 3 ; i++ )
		{
		    Average[r][i] += Raw[r][f][i];
		}
	    }
	}

	if ( Average )
	{
	    for ( i = 0 ; i < 3 ; i++ )
	    {
		Average[r][i] /= Folds;
	    }
	}
    }

    /*  Check that amount of data is correct  */

    if ( fgets(Line, 100, stdin) )
    {
	printf("\nExpecting %d lines\n", Folds * Repeats * 2);
	exit(1);
    }

    if ( Average )
    {
	PrintSummary(Average, Repeats, "XVal");
    }
    else
    {
	PrintSummary(Raw[SIZE], Folds, "Fold");
    }

    return 0;
}


char
     *StdP[]  = {	"    Decision Tree   ",
			"  ----------------  ",
			"    Size    Errors  " },

     *StdPC[] = {	"        Decision Tree      ",
			"  -----------------------  ",
			"    Size    Errors   Cost  " },

     *Extra[] = {	"        Rules     ",
			"  ----------------",
			"      No    Errors" },

     *ExtraC[]= {	"           Rules         ",
			"  -----------------------",
			"      No    Errors   Cost" };

void PrintSummary(float **Val, int No, char *Title)
/*   ------------  */
{
    int i, j;
    float Sum[3], SumSq[3];

    for ( i = 0 ; i < 3 ; i++ )
    {
	Sum[i] = SumSq[i] = 0;
    }

    for ( i = 0 ; i <= 2 ; i++ )
    {
	switch ( i )
	{
	    case 0:
		printf("\n\t%s  ", Title);
		break;

	    case 1:
		printf("\t----  ");
		break;

	    case 2:
		printf("\t      ");
	}

	printf("%s\n", ( Composite ?
			 ( Costs ? ExtraC[i] : Extra[i] ) :
			 Rules ?
			 ( Costs ? ExtraC[i] : Extra[i] ) :
			 ( Costs ? StdPC[i] : StdP[i] ) ));
    }
    printf("\n");

    for ( j = 0 ; j < No ; j++ )
    {
	for ( i = 0 ; i < 3 ; i++ )
	{
	    Sum[i] += Val[j][i];
	    SumSq[i] += Val[j][i] * Val[j][i];
	}

	printf("\t%3d   ", j+1);

	if ( Boost )
	{
	    printf("       *");
	}
	else
	{
	    printf("%8.1f", Val[j][SIZE]);
	}

	printf("     %4.1f%%  ", Val[j][ERRP]);

	if ( Costs )
	{
	    printf("%5.2f  ", Val[j][COST]);
	}

	printf("\n");
    }

    printf("\n\tMean  ");

    if ( Boost )
    {
	printf("        ");
    }
    else
    {
	printf("%8.1f", Sum[SIZE] / No);
    }

    printf("     %4.1f%%  ", Sum[ERRP] / No);

    if ( Costs )
    {
	printf("%5.2f  ", Sum[COST] / No);
    }

    printf("\n\tSE    ");

    if ( Boost )
    {
	printf("        ");
    }
    else
    {
	printf("%8.1f", SE(Sum[SIZE], SumSq[SIZE], No));
    }

    printf("     %4.1f%%  ", SE(Sum[ERRP], SumSq[ERRP], No));

    if ( Costs )
    {
	printf("%5.2f  ", SE(Sum[COST], SumSq[COST], No));
    }

    printf("\n");
}



float SE(float sum, float sumsq, int no)
/*    --  */
{
    float mean;

    mean = sum / no;

    return sqrt( ((sumsq - no * mean * mean) / (no - 1)) / no );
}
