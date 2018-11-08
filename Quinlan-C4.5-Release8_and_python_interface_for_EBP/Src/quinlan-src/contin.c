/*************************************************************************/
/*                                                                         */
/*        Evaluation of a test on a continuous valued attribute                   */
/*        -----------------------------------------------------                   */
/*                                                                           */
/*************************************************************************/


#include "buildex.i"
void PrintDistribution();
void ResetFreq();
void Quicksort();
void Sprout();
float
        *SplitGain,        /* SplitGain[i] = gain with att value of item i as threshold */
        *SplitInfo;        /* SplitInfo[i] = potential info ditto */



/*************************************************************************/
/*                                                                           */
/*  Continuous attributes are treated as if they have possible values         */
/*        0 (unknown), 1 (less than cut), 2(greater than cut)                   */
/*  This routine finds the best cut for items Fp through Lp and sets         */
/*  Info[], Gain[] and Bar[]                                                 */
/*                                                                           */
/*************************************************************************/
//Xp:含有缺失值的数据条数
//Lp：当前数据集总条数


void EvalContinuousAtt(Att, Fp, Lp)
/*  -----------------  */ 
Attribute Att;
ItemNo Fp, Lp; 
{ 
    ItemNo i, BestI, Xp, Tries=0;
    ItemCount Items, KnownItems, LowItems, MinSplit, CountItems();
    ClassNo c;
    float AvGain=0, Val, BestVal, BaseInfo, ThreshCost,ComputeGain(), TotalInfo(), Worth();
    void Swap();

    Verbosity(2) printf("\tAtt %s", AttName[Att]);
    Verbosity(3) printf("\n");

    ResetFreq(2);

//以下是怕缺失值的数据太多，如果只有两条数据是完整的，就会打印出上述通知

    /*  Omit and count unknown values */

    Items = CountItems(Fp, Lp);
    Xp = Fp;
    ForEach(i, Fp, Lp)
    {
        if ( CVal(Item[i],Att) == Unknown )
        {

            Freq[ 0 ][ Class(Item[i]) ] += Weight[i];//经过对crx的数据集进行测试得知，所有的Weight[i]都等于1
            Swap(Xp, i);
            Xp++;
        }
    }

    ValFreq[0] = 0;

    ForEach(c, 0, MaxClass)//把数值相等但是类别不相等的进行累加
    {
        ValFreq[0] += Freq[0][c];
    }

    KnownItems = Items - ValFreq[0];
    UnknownRate[Att] = 1.0 - KnownItems / Items;

//下面这个不需要考虑
    /*  Special case when very few known values  */
    if ( KnownItems < 2 * MINOBJS )
    {
        Verbosity(2) printf("\tinsufficient cases with known values\n");

        Gain[Att] = -Epsilon;
        Info[Att] = 0.0;
        return;
    }

//以上是怕缺失值的数据太多，如果只有两条数据是完整的，就会打印出上述通知


    Quicksort(Xp, Lp, Att, Swap);//把数据集按照从小到大排序（根据连续特征的数值）

    /*  Count base values and determine base information  */

    ForEach(i, Xp, Lp)
    {   
        Freq[2][ Class(Item[i]) ] += Weight[i];//一开始是把所有数据的权重都塞一个分区里面了
        SplitGain[i] = -Epsilon;
        SplitInfo[i] = 0;
    }

    BaseInfo = TotalInfo(Freq[2], 0, MaxClass) / KnownItems;

    /*  Try possible cuts between items i and i+1, and determine the
        information and gain of the split in each case.  We have to be wary
        of splitting a small number of items off one end, as we can always
        split off a single item, but this has little predictive power.  */

    
    // 对于没有缺失值而言的数据集而言，这里的KnownItems和MaxClass的数值是一样的
    
    
    MinSplit = 0.10 * KnownItems / (MaxClass + 1);//这里的MaxClass是一个全局变量

    if ( MinSplit <= MINOBJS ) 
        MinSplit = MINOBJS;
    else
        if ( MinSplit > 25 ) 
            MinSplit = 25;

        LowItems = 0;
        ForEach(i, Xp, Lp - 1)
    //Xp的意思是含有缺失值的数据有几条,
    // 所以没有缺失数据的情况下,Xp=0
        {

            c = Class(Item[i]);
            LowItems   += Weight[i];

    //下面是因为，我们处理连续特征的取值时，是当做二分区间处理的
            Freq[1][c] += Weight[i];
            Freq[2][c] -= Weight[i];
        // 这个Freq[][c]的含义是不同区间的不同分类的数量

//☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
        if ( LowItems < MinSplit ) //因为分割点肯定不会在前面两项，这样数据就太不平衡了
            continue;
        else if ( LowItems > KnownItems - MinSplit ) //因为分割点肯定不会在倒数第两项，这样数据就太不平衡了
            break;

        //上面的这个If-else的作用就是第一个阈值和最后一个阈值不作为考虑
//☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
    if ( CVal(Item[i],Att) < CVal(Item[i+1],Att) - 1E-5 )//如果两个连续值挨得太近，那么就认为是同一个连续值
    {

            ValFreq[1] = LowItems;//这个表示分割点以前有几条数据
            ValFreq[2] = KnownItems - LowItems;//这个表示分割点以后有几条数据
            // 是为了后面计算特征熵Split(D,T)而服务的
            SplitGain[i] = ComputeGain(BaseInfo, UnknownRate[Att], 2, KnownItems);//Gain(D,T)
            //注意！！！这里会传入ValFreq参数

            // 注意，上面这句代码之所以取2的意思，是把连续区间当做二分区间进行处理的，也就是说，近似认为连续取值的特征只有两种取值：小于阈值、大于阈值
            SplitInfo[i] = TotalInfo(ValFreq, 0, 2) / Items;//Split(D,T)


            AvGain += SplitGain[i];


            Tries++;


            Verbosity(3)
            {        
                printf("\t\tCut at %.3f  (gain %.3f, val %.3f):",( CVal(Item[i],Att) + CVal(Item[i+1],Att) ) / 2,SplitGain[i],Worth(SplitInfo[i], SplitGain[i], Epsilon));
                PrintDistribution(Att, 2, true);
            }
        }
    }

    /*  Find the best attribute according to the given criterion  */
    ThreshCost = Log(Tries) / Items;//这个地方就是log2(N-1)/|D|
    // 但是这里的Ｎ并不是连续值的取值种数，而是经过筛选的结果。



    BestVal = 0;
    BestI   = None;
    // printf("Att=%d\n",Att);

    ForEach(i, Xp, Lp - 1)
    {

        if ( (Val = SplitGain[i] - ThreshCost) > BestVal )
        {
            BestI   = i;
            BestVal = Val;
        }
    }

    /*  If a test on the attribute is able to make a gain,
        set the best break point, gain and information  */ 

    if ( BestI == None )
    {
        Gain[Att] = -Epsilon;
        Info[Att] = 0.0;

        Verbosity(2) printf("\tno gain\n");
    }
    else
    {
        Bar[Att]  = (CVal(Item[BestI],Att) + CVal(Item[BestI+1],Att)) / 2;//计算阈值
        Gain[Att] = BestVal;
        Info[Att] = SplitInfo[BestI];

        Verbosity(2)
        printf("\tcut=%.3f, inf %.3f, gain %.3f\n",
           Bar[Att], Info[Att], Gain[Att]);
    }
} 



/*************************************************************************/
/*                                                                         */
/*  Change a leaf into a test on a continuous attribute                    */
/*                                                                         */
/*************************************************************************/

//-----------------------下面部分的代码不予转化，因为没有用------------------------
void ContinTest(Node, Att)
/*  ----------  */
Tree Node;
Attribute Att;
{
    float Thresh, GreatestValueBelow();
    ItemCount CountItems();

    Sprout(Node, 2);

    Thresh = GreatestValueBelow(Att, Bar[Att]);

    Node->NodeType        = ThreshContin;
    Node->Tested        = Att;
    Node->Cut                =
    Node->Lower                =
    Node->Upper                = Thresh;
    Node->Errors        = 0;
}



/*************************************************************************/
/*                                                                         */
/*  Return the greatest value of attribute Att below threshold t           */
/*                                                                         */
/*************************************************************************/

//返回该特征中，低于分割阈值的最大连续值（应用用不到）
float GreatestValueBelow(Att, t)
/*    ------------------  */
Attribute Att;
float t;
{
    ItemNo i;
    float v, Best;
    Boolean NotYet=true;

    ForEach(i, 0, MaxItem)
    {
        v = CVal(Item[i], Att);
        if ( v != Unknown && v <= t && ( NotYet || v > Best ) )
        {
            Best = v;
            NotYet = false;
        }
    }

    return Best;
}
