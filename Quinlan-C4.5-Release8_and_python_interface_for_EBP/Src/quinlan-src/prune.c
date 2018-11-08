/*************************************************************************/
/*									 */
/*	Prune a decision tree and predict its error rate		 */
/*	------------------------------------------------		 */
/*									 */
/*************************************************************************/


#include "defns.i"
#include "types.i"
#include "extern.i"
#include <string.h>

void InitialiseWeights();
void CheckPossibleValues();
void Indent();

extern	ItemCount	*Weight;

Set	*PossibleValues=Nil;
Boolean	Changed;

#define	LocalVerbosity(x)	if (Sh >= 0 && VERBOSITY >= x)
#define	Intab(x)		Indent(x, "| ")



/*************************************************************************/
/*									 */
/*  Prune tree T, returning true if tree has been modified	 */
/*									 */
/*************************************************************************/


Boolean Prune(T)//因为可能剪枝失败，所以这里是判断剪枝前后决策树是否有变化
/*      -----  */
Tree T;
{
    ItemNo i;
    float EstimateErrors();
    Attribute a;//变量和函数声明

    InitialiseWeights();
    AllKnown = true;

    Verbosity(1) printf("\n");

    Changed = false;

    EstimateErrors(T, 0, MaxItem, 0, true);

    if ( SUBSET )
    {
       if ( ! PossibleValues )
       {
           PossibleValues = (Set *) calloc(MaxAtt+1, sizeof(Set));
       }

       ForEach(a, 0, MaxAtt)
       {
           if ( MaxAttVal[a] )
           {
              PossibleValues[a] = (Set) malloc((MaxAttVal[a]>>3) + 1);
              ClearBits((MaxAttVal[a]>>3) + 1, PossibleValues[a]);
              ForEach(i, 1, MaxAttVal[a])
              {
                  SetBit(i, PossibleValues[a]);
              }
          }
      }

      CheckPossibleValues(T);
  }

  return Changed;
}




/*************************************************************************/
/*									 */
/*	Estimate the errors in a given subtree				 */
/*									 */
/*************************************************************************/


float EstimateErrors(T, Fp, Lp, Sh, UpdateTree)//这个是在估计整棵树的错误率，递归调用的函数
/*    --------------  */
Tree T;
ItemNo Fp, Lp; 
short Sh;
Boolean UpdateTree;
{ 
    ItemNo i, Kp, Ep, Group();
    ItemCount Cases, KnownCases, *LocalClassDist, TreeErrors, LeafErrors,
    ExtraLeafErrors, BranchErrors, CountItems(), Factor, MaxFactor, AddErrs();
    DiscrValue v, MaxBr;
    ClassNo c, BestClass;
    Boolean PrevAllKnown;

    /*  Generate the class frequency distribution  */

    Cases = CountItems(Fp, Lp);
    LocalClassDist = (ItemCount *) calloc(MaxClass+1, sizeof(ItemCount));

    ForEach(i, Fp, Lp)
    { 
       LocalClassDist[ Class(Item[i]) ] += Weight[i];
   } 

    /*  Find the most frequent class and update the tree  */

   BestClass = T->Leaf;
   ForEach(c, 0, MaxClass)
   {
       if ( LocalClassDist[c] > LocalClassDist[BestClass] )
       {
           BestClass = c;
       }
   }
   LeafErrors = Cases - LocalClassDist[BestClass];
   // printf("Cases=%f\n",Cases);
   // printf("若剪枝后将会误判LeafErrors=%f\n",LeafErrors);
   ExtraLeafErrors = AddErrs(Cases, LeafErrors);
   // printf("若剪枝后将会误判ExtraLeafErrors=%f\n",ExtraLeafErrors);
   // 1.506894=AddErrs(1,16)

   //cases最初是整个数据集的条数，随着迭代，变成每个子树的数据集总条数
    // LeafErrors=116.000000的意思是，如果整棵树变成一个节点，有116条数据会被误判

   // printf("ExtraLeafErrors初始化:%f\n",ExtraLeafErrors);

   if ( UpdateTree )
   {
       T->Items = Cases;
       T->Leaf  = BestClass;
       memcpy(T->ClassDist, LocalClassDist, (MaxClass + 1) * sizeof(ItemCount));
   }

    if ( ! T->NodeType )	/*  leaf  */
   {
	TreeErrors = LeafErrors + ExtraLeafErrors;//叶子的实际错误数+悲观错误数
    // printf("LeafErrors=%f\n" ,LeafErrors);
    // printf("ExtraLeafErrors=%f\n" ,ExtraLeafErrors);
    if ( UpdateTree )
    {
       T->Errors = TreeErrors;

       LocalVerbosity(1)
       {
          Intab(Sh);
          printf("%s (%.2f:%.2f/%.2f)\n", ClassName[T->Leaf],
             T->Items, LeafErrors, T->Errors);
      }
  }

  free(LocalClassDist);

  return TreeErrors;
}

    /*  Estimate errors for each branch  */

Kp = Group(0, Fp, Lp, T) + 1;
KnownCases = CountItems(Kp, Lp);

PrevAllKnown = AllKnown;
if ( Kp != Fp ) AllKnown = false;

TreeErrors = MaxFactor = 0;

ForEach(v, 1, T->Forks)
{
	Ep = Group(v, Kp, Lp, T);

	if ( Kp <= Ep )
	{
       Factor = CountItems(Kp, Ep) / KnownCases;

       if ( Factor >= MaxFactor )
       {
          MaxBr = v;
          MaxFactor = Factor;
      }

      ForEach(i, Fp, Kp-1)
      {
          Weight[i] *= Factor;
      }

      TreeErrors += EstimateErrors(T->Branch[v], Fp, Ep, Sh+1, UpdateTree);
      // printf("BranchError[%d]=%f\n",v,EstimateErrors(T->Branch[v], Fp, Ep, Sh+1, UpdateTree));

      Group(0, Fp, Ep, T);
      ForEach(i, Fp, Kp-1)
      {
          Weight[i] /= Factor;
      }
  }
}

AllKnown = PrevAllKnown;

if ( ! UpdateTree )
{
	free(LocalClassDist);

	return TreeErrors;
}

    /*  See how the largest branch would fare  */

BranchErrors = EstimateErrors(T->Branch[MaxBr], Fp, Lp, -1000, false);

LocalVerbosity(1)
{
    Intab(Sh);
    printf("%s:  [%d%%  N=%.2f  tree=%.2f  leaf=%.2f+%.2f  br[%d]=%.2f]\n",
      AttName[T->Tested],
      (int) ((TreeErrors * 100) / (T->Items + 0.001)),
      T->Items, TreeErrors, LeafErrors, ExtraLeafErrors,
      MaxBr, BranchErrors);
}

    /*  See whether tree should be replaced with leaf or largest branch  
    这里是在判断是否应该进行剪枝*/

if    ( LeafErrors + ExtraLeafErrors <= BranchErrors + 0.1 
    && LeafErrors + ExtraLeafErrors <= TreeErrors + 0.1 )
    {//如果剪枝后的错误比嫁接后的错误少，
    //并且剪枝后的错误比剪枝前的错误少
    // 那就进行剪枝
        // printf("-------剪枝start-------------\n");
        // printf("LeafErrors=%f\n",LeafErrors);
        // printf("ExtraLeafErrors=%f\n",ExtraLeafErrors);
        // printf("BranchErrors=%f\n",BranchErrors);
        // printf("TreeErrors =%f\n",TreeErrors);
        // printf("-------剪枝end-------------\n");
        LocalVerbosity(1)
        {
           Intab(Sh);
	    printf("Replaced with leaf %s\n", ClassName[T->Leaf]);//进行pruning（剪枝）操作
	}

	T->NodeType = 0;
	T->Errors = LeafErrors + ExtraLeafErrors;
	Changed = true;
}
else
    if ( BranchErrors <= TreeErrors + 0.1 )//如果嫁接后的错误比原来少
        {  
    // printf("-------嫁接start-------------\n");
    // printf("BranchErrors =%f\n",BranchErrors);
    // printf("TreeErrors=%f\n",TreeErrors);
    // printf("-------嫁接end-------------\n");

    LocalVerbosity(1)
    {
       Intab(Sh);
	    printf("Replaced with branch %d\n", MaxBr);//进行grafting（嫁接）操作
	}

	AllKnown = PrevAllKnown;
	EstimateErrors(T->Branch[MaxBr], Fp, Lp, Sh, true);
	memcpy((char *) T, (char *) T->Branch[MaxBr], sizeof(TreeRec));// 具体的嫁接操作，用数值替换掉原来的树
	Changed = true;
}
else
{
	T->Errors = TreeErrors;//否则维持决策树的原样
}

AllKnown = PrevAllKnown;
free(LocalClassDist);

return T->Errors;
}



/*************************************************************************/
/*									 */
/*	Remove unnecessary subset tests on missing values		 */
/*									 */
/*************************************************************************/


void CheckPossibleValues(T)
/*  -------------------  */
Tree T;
{
    Set HoldValues;
    int v, Bytes, b;
    Attribute A;
    char Any=0;

    if ( T->NodeType == BrSubset )
    {
       A = T->Tested;

       Bytes = (MaxAttVal[A]>>3) + 1;
       HoldValues = (Set) malloc(Bytes);

	/*  See if last (default) branch can be simplified or omitted  */

       ForEach(b, 0, Bytes-1)
       {
           T->Subset[T->Forks][b] &= PossibleValues[A][b];
           Any |= T->Subset[T->Forks][b];
       }

       if ( ! Any )
       {
           T->Forks--;
       }

	/*  Process each subtree, leaving only values in branch subset  */

       CopyBits(Bytes, PossibleValues[A], HoldValues);

       ForEach(v, 1, T->Forks)
       {
           CopyBits(Bytes, T->Subset[v], PossibleValues[A]);

           CheckPossibleValues(T->Branch[v]);
       }

       CopyBits(Bytes, HoldValues, PossibleValues[A]);

       free(HoldValues);
   }
   else
    if ( T->NodeType )
    {
       ForEach(v, 1, T->Forks)
       {
           CheckPossibleValues(T->Branch[v]);
       }
   }
}
