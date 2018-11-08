/*************************************************************************/
/*									 */
/*	Evaluation of a test on a discrete valued attribute		 */
/*      ---------------------------------------------------		 */
/*									 */
/*************************************************************************/


#include "buildex.i"
void PrintDistribution();
void Sprout();
void ResetFreq();
void ComputeFrequencies();
/*************************************************************************/
/*									 */
/*  Set Info[] and Gain[] for discrete partition of items Fp to Lp	 */
/*									 */
/*************************************************************************/

void EvalDiscreteAtt(Att, Fp, Lp, Items)
/*  ---------------  */ 
    Attribute Att;
    ItemNo Fp, Lp; 
    ItemCount Items;
{ 
    ItemCount KnownItems;
    float DiscrKnownBaseInfo(), ComputeGain(), TotalInfo();//声明了一些函数

    ComputeFrequencies(Att, Fp, Lp);

    KnownItems = Items - ValFreq[0];//ValFreq[0]是含有缺失值的数据的条数

    /*  Special case when no known values of the attribute  */

    if ( Items <= ValFreq[0] )
    {
	Verbosity(2) printf("\tAtt %s: no known values\n", AttName[Att]);

	Gain[Att] = -Epsilon;
	Info[Att] = 0.0;
	return;
    }

    Gain[Att] = ComputeGain(DiscrKnownBaseInfo(KnownItems, MaxAttVal[Att]),UnknownRate[Att], MaxAttVal[Att], KnownItems);

    Info[Att] = TotalInfo(ValFreq, 0, MaxAttVal[Att]) / Items;
    // Dlog2(D)-ΣDi·log2（D）
    //其中Items=D
    //所以Info[Att] =（D·log2(D)-ΣDi·log2（D））/D
    //注意上面存在关系：ΣDi=D

    Verbosity(2)
    {
        printf("\tAtt %s", AttName[Att]);
        Verbosity(3) PrintDistribution(Att, MaxAttVal[Att], true);
        printf("-----------------Verbosity------------------\n");
        printf("\tinf %.3f, gain %.3f\n", Info[Att], Gain[Att]);
        printf("---------------Verbosity--------------------\n");
    }

} 



/*************************************************************************/
/*									 */
/*  Compute frequency tables Freq[][] and ValFreq[] for attribute	 */
/*  Att from items Fp to Lp, and set the UnknownRate for Att		 */
/*									 */
/*************************************************************************/
  

// 这个函数是在计算缺失值的频率，暂时认为没啥用
void ComputeFrequencies(Att, Fp, Lp)
/*  ------------------  */
    Attribute Att;
    ItemNo Fp, Lp;
{

    //上面三个值初始时都是一样的
    Description Case; 
    ClassNo c;
    DiscrValue v;
    ItemCount CountItems();
    ItemNo p;

    ResetFreq(MaxAttVal[Att]);

    /*  Determine the frequency of each class amongst cases
	with each possible value for the given attribute  */
    ForEach(p, Fp, Lp)
    { 
	Case = Item[p];
	Freq[ DVal(Case,Att) ][ Class(Case) ] += Weight[p];
    // printf("Freq[%d][%d]=%f\n",DVal(Case,Att),Class(Case),Freq[ DVal(Case,Att) ][ Class(Case) ]);
    } 

    /*  Determine the frequency of each possible value for the
	given attribute  */

    ForEach(v, 0, MaxAttVal[Att]) 
    { 
	ForEach(c, 0, MaxClass)
	{
	    ValFreq[v] += Freq[v][c];
	}
    }

    /*  Set the rate of unknown values of the attribute  */

    //这里的ｕｎｋｎｏｗｎ　ｖａｌｕｅｓ对于正常的数据集来说，没啥用
 
    UnknownRate[Att] = ValFreq[0] / CountItems(Fp, Lp);

}



/*************************************************************************/
/*									 */
/*  Return the base info for items with known values of a discrete	 */
/*  attribute, using the frequency table Freq[][]			 */
/*	 								 */
/*************************************************************************/

//Info(D)
float DiscrKnownBaseInfo(KnownItems, MaxVal)
/*    ------------------  */
    DiscrValue MaxVal;
    ItemCount KnownItems;
{
    ClassNo c;
    ItemCount ClassCount;
    double Sum=0;
    DiscrValue v;
    //v是离散属性的取值
//
    ForEach(c, 0, MaxClass)
    {
	ClassCount = 0;
	ForEach(v, 1, MaxVal)
	{
	    ClassCount += Freq[v][c];
	}
	Sum += ClassCount * Log(ClassCount);
    }
 float result=(KnownItems * Log(KnownItems) - Sum) / KnownItems;


 // printf("base entropy=%f\n",result);//这个是在计算的当前数据集的类别标签的那一列的信息熵
    return result;
}



/*************************************************************************/
/*									 */
/*  Construct and return a node for a test on a discrete attribute	 */
/*									 */
/*************************************************************************/


void DiscreteTest(Node, Att)
/*  ----------  */
    Tree Node;
    Attribute Att;
{
    ItemCount CountItems();

    Sprout(Node, MaxAttVal[Att]);

    Node->NodeType	= BrDiscr;
    Node->Tested	= Att;
    Node->Errors	= 0;
}
