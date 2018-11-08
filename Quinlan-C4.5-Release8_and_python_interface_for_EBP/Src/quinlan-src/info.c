/*************************************************************************/
/*									 */
/*	Calculate information, information gain, and print dists	 */
/*	--------------------------------------------------------	 */
/*									 */
/*************************************************************************/


#include "buildex.i"


/****************************************************************/
/*									 */
/*  Determine the worth of a particular split according to the		 */
/*  operative criterion							 */
/*									 */
/*	    Parameters:							 */
/*		SplitInfo:	potential info of the split		 */
/*		SplitGain:	gain in info of the split		 */
/*		MinGain:	gain above which the Gain Ratio		 */
/*				may be used				 */
/*									 */
/*  If the Gain criterion is being used, the information gain of	 */
/*  the split is returned, //这里的意思是，如果使用熵增益，那么返回熵增益

/*  but if the Gain Ratio criterion is		 *///如果使用熵增益率，那么返回熵增益率
/*  being used, the ratio of the information gain of the split to	 */
/*  its potential information is returned.				 */
/*									 */
/*************************************************************************/

//ThisGain->Gain(D,T)分子
//ThisInfo->Split(D,T)分母

// Worth的含义是，如果当前的熵增益比MinGain还小，就没必要计算熵增益率
float Worth(ThisInfo, ThisGain, MinGain)//MinGain---->AvGain
    float ThisInfo, ThisGain, MinGain;
{

    if ( GAINRATIO )//默认GAINRATIO＝Ｔｒｕｅ
    {

	if ( ThisGain >= MinGain - Epsilon && ThisInfo > Epsilon )//Epsilon=0.001,全局变量
        //这里的限制条件是：
        //分子>=MinGain-Epsilon
        //分母＞Epsilon
	{
        float gain_ratio=ThisGain / ThisInfo;// --------->Gain(D,T)/Split(D,T)


	    return gain_ratio;
	}
	else
	{
	    return -Epsilon;
	}
    }
    //下面的代码不用关心
    else
    {
    return ( ThisInfo > 0 && ThisGain > -Epsilon ? ThisGain : -Epsilon );
    }
}



/*************************************************************************/
/*									 */
/*  Zero the frequency tables Freq[][] and ValFreq[] up to MaxVal	 */
/*									 */
/*************************************************************************/


void ResetFreq(MaxVal)
/*  ---------  */
    DiscrValue MaxVal;
{
    DiscrValue v;
    ClassNo c;

    ForEach(v, 0, MaxVal)
    { 
    ForEach(c, 0, MaxClass)
    {
        Freq[v][c] = 0;
    }
    ValFreq[v] = 0;
    } 
}




/*************************************************************************/
/*									 */
/*  Given tables Freq[][] and ValFreq[], compute the information gain.	 */
/*									 */
/*	    Parameters:							 */
/*		BaseInfo:	average information for all items with	 */
/*				known values of the test attribute	 */
/*		UnknownRate:	fraction of items with unknown ditto	 */
/*		MaxVal:		number of forks				 */
/*		TotalItems:	number of items with known values of	 */
/*				test att				 */
/*									 */
/*  where Freq[x][y] contains the no. of cases with value x for a	 */
/*  particular attribute that are members of class y,			 */
/*     and ValFreq[x] contains the no. of cases with value x for a		 */
/*  particular attribute						 */
/*									 */
/*************************************************************************/
// Freq[x][y]，属于某个值并且属于类别y的数量
// ValFreq[x]属于某个值的数量


//这个是用来计算信息熵增益的
// (可以用来计算离散特征的熵增益和连续特征的熵增益)
//ＴｏｔａｌＩｔｅｍｓ是当前数据集的数量
// SplitGain[i] = ComputeGain(BaseInfo, UnknownRate[Att], 2, KnownItems);这个是contin.c中的代码
float ComputeGain(BaseInfo, UnknFrac, MaxVal, TotalItems)//当计算连续取值的特征时，MaxVal=2
/*    -----------  */
    float BaseInfo, UnknFrac;
    DiscrValue MaxVal;
    ItemCount TotalItems;
{
    // printf("UnknFrac=%f\n",UnknFrac);
    DiscrValue v;
    float ThisInfo=0.0, ThisGain, TotalInfo();
    short ReasonableSubsets=0;

    /*  Check whether all values are unknown or the same  */

    if ( ! TotalItems )
     return -Epsilon;

    /*  There must be at least two subsets with MINOBJS(这里是min_objects的意思) items  */ 

//计算连续特征的条件熵的时候，MaxVal=2
    ForEach(v, 1, MaxVal)
    {
	if ( ValFreq[v] >= MINOBJS ) //MINOBJS=2
        ReasonableSubsets++;
    }
    if ( ReasonableSubsets < 2 ) 
        return -Epsilon;

    /*  Compute total info after split, by summing the
	info of each of the subsets formed by the test  */


//------------上面是条件判断，下面才是开始计算条件熵-----------------


    ForEach(v, 1, MaxVal)//这里是在计算条件熵
    {

    // printf("-------------下面ThisInfo计算-----------------------------------\n");
    // printf("ThisInfo_for_start=%f\n",ThisInfo);
    ThisInfo += TotalInfo(Freq[v], 0, MaxClass);

    // printf("--------------上面面是ThisInfo计算-----------------------------------\n");
    }
    // printf("TotalItems=%f\n",TotalItems);
    // printf("BaseInfo=%f\n",BaseInfo);

    /*  Set the gain in information for all items, adjusted for unknowns  */
    // printf("--------------下面是ThisGain-----------------------------------\n");


    ThisGain = (1 - UnknFrac) * (BaseInfo - ThisInfo / TotalItems);


    // printf("---------------上面是ThisGain----------------------------------\n");

    Verbosity(5)
        printf("ComputeThisGain: items %.1f info %.3f base %.3f unkn %.3f result %.3f\n",TotalItems + ValFreq[0], ThisInfo, BaseInfo, UnknFrac, ThisGain);
    return ThisGain;
}



/*************************************************************************/
/*									 */
/*  Compute the total information in V[ MinVal..MaxVal ]		 */
/*									 */
/*************************************************************************/
float TotalInfo(V, MinVal, MaxVal)
/*    ---------  */
    ItemCount V[];
    DiscrValue MinVal, MaxVal;
{


    DiscrValue v;
    float Sum=0.0;
    ItemCount N, TotalItems=0;

    ForEach(v, MinVal, MaxVal)
    {
	N = V[v];
//这里的Ｎ是整数
	Sum += N * Log(N);
	TotalItems += N;

    }



    double result_here=TotalItems * Log(TotalItems) - Sum;// log底数为２
//                                                        k
//                                     |D|·log2（|D|）-  Σ  |Di|log2（|Di|）
// 对应论文中TotalItems * Log(TotalItems)      i=1    Sum
//      计算实例  17log2(17) - [6log2(6)+6log2(6)+5log2(5)]
//                 = 69.48       -         42.629192
//                 = 26.851
//然后discr.c中的item[Attr]= 26.851/17=1.57947,这个也就是“离散特征的计算熵增益率时的分母的值”
//论文中对于离散特征是没有提及Split(D,T)的

// 案例：6log2(6)-6log2(3)

    return result_here;


}



/*************************************************************************/
/*									 */
/*	Print distribution table for given attribute		 */
/*									 */
/*************************************************************************/

void PrintDistribution(Att, MaxVal, ShowNames)
/*  -----------------  */
    Attribute Att;
    DiscrValue MaxVal;
    Boolean ShowNames;
{
    DiscrValue v;
    ClassNo c;
    String Val;

    printf("\n\t\t\t ");
    ForEach(c, 0, MaxClass)
    {
	printf("%7.6s", ClassName[c]);
    }
    printf("\n");

    ForEach(v, 0, MaxVal)
    {
	if ( ShowNames )
	{
	    Val = ( !v ? "unknown" :
		    MaxAttVal[Att] ? AttValName[Att][v] :
		    v == 1 ? "below" : "above" );
	    printf("\t\t[%-7.7s:", Val);
	}
	else
	{
	    printf("\t\t[%-7d:", v);
	}

	ForEach(c, 0, MaxClass)
	{
	    printf(" %6.1f", Freq[v][c]);
	}

	printf("]\n");
    }
}
