/*************************************************************************/
/*									 */
/*	Get case descriptions from data file				 */
/*	--------------------------------------				 */
/*									 */
/*************************************************************************/


#include "defns.i"
#include "types.i"
#include "extern.i"
#include <string.h>
#define Inc 2048
int Which();
void Error();

/*************************************************************************/
/*									 */
/*  Read raw case descriptions from file with given extension.		 */
/*									 */
/*  On completion, cases are stored in array Item in the form		 */
/*  of Descriptions (i.e. arrays of attribute values), and		 */
/*  MaxItem is set to the number of data items.				 */
/*									 */
/*************************************************************************/

void GetData(Extension)
/*  --------  */
    String Extension;
{
    FILE *Df, *fopen();
    char Fn[100];
    ItemNo i=0, j, ItemSpace=0;
    Description GetDescription();

    /*  Open data file  */

    strcpy(Fn, FileName);//令Fn=FileName
    strcat(Fn, Extension);//令Fn=Extension
    if ( ! ( Df = fopen(Fn, "r") ) ) Error(0, Fn, "");
    do
    {
	MaxItem = i;

	/*  Make sure there is room for another item  */

	if ( i >= ItemSpace )
	{
	    if ( ItemSpace )
	    {
		ItemSpace += Inc;
		Item = (Description *)
			realloc(Item, ItemSpace*sizeof(Description));
	    }
	    else
	    {
		Item = (Description *)
			malloc((ItemSpace=Inc)*sizeof(Description));
	    }
	}

	Item[i] = GetDescription(Df);

    } while ( Item[i] != Nil && ++i );

    fclose(Df);
    MaxItem = i - 1;
}



/*************************************************************************/
/*									 */
/*  Read a raw case description from file Df.（Df指的是.data文件）				 */
/*									 */
/*  For each attribute, read the attribute value from the file.		 */
/*  If it is a discrete valued attribute, find the associated no.	 */
/*  of this attribute value (if the value is unknown this is 0).	 */
//意思是，对于.data中的数据的离散特征，找个这个特征的取值在.names文件中对应特征的第几个取值，然后返回序号Dv
/*									 */
/*  Returns the Description of the case (i.e. the array of		 */
/*  attribute values).							 */
/*									 */
/*************************************************************************/


Description GetDescription(Df)
    FILE *Df;
{
    Attribute Att;
    char name[500], *endname, *CopyString();
    Boolean ReadName();
    int Dv;
    float Cv;
    Description Dvec;
    double strtod();

    if ( ReadName(Df, name) )
    {
	Dvec = (Description) calloc(MaxAtt+2, sizeof(AttValue));//分配空间

//这里的MaxAtt+1指的是当前数据集有多少属性
        ForEach(Att, 0, MaxAtt)//这里Att是一个变量
        {
        //printf ("IGNORE=%d\n",IGNORE);
        //printf("--------------------------\n");
        //printf ("MaxAtt=%d\n",MaxAtt);


	    if ( SpecialStatus[Att] == IGNORE )
	    {
		/*  Skip this value  */

		DVal(Dvec, Att) = 0;
	    }
	    else
	    if ( MaxAttVal[Att] || SpecialStatus[Att] == DISCRETE )
	    {
		/*  Discrete value  */ 
		
//printf ("---------------------看看crx这里进来没------------1-------\n");
	        if ( ! ( strcmp(name, "?") ) )//如果name和"?"相等，那么strcmp会返回0
		{
		    Dv = 0;
		    //printf ("-------------------设置Dv=0-----------------\n");
		}
	        else
	        {
	         //printf ("---------------------看看crx这里进来没--------进来了--3-------\n");
	         //AttValName[Att]:指的是.names文件夹中的某个特征的取值列表
		    Dv = Which(name, AttValName[Att], 1, MaxAttVal[Att]);
		    //Which函数的定义在getnames.c中
		    //printf("---------------这里重点检查------1-----------\n");
		    //printf ("Dv=%d\n",Dv); //DISCRETE=2
		    //printf("name=%s\n",name);
		    //printf("AttValName[Att][Dv]=AttValName[%d][%d]=%s\n",Att,Dv,AttValName[Att][Dv]);//这个地方输出不对
		    //printf("MaxAttVal[Att]=%d\n",MaxAttVal[Att]);
		    //printf("---------------这里重点检查-------2----------\n");
		    //AttValName[Att][Dv]:这个指的是.names文件中的第Att个（Att从0开始算起）特征的第Dv个(Dv从1开始算起)取值
		    //如果一堆特征中，既有离散特征和连续特征，例如.names文件中：
                    //先后顺序是3个离散特征A1、A2、A3，1个连续特征A4，紧接着又是一个离散特征A5，那么获取A5时，Att=4
		    //如果Dv=0，那么就表明数据集存在故障,正常运行情况下，必须满足Dv≠0
		    //在代码的眼里，.names文件就是一个二维的矩阵，这个矩阵就是AttValName[Att][Dv]
		    
		    
		    if ( ! Dv )
		    { 
		        printf ("---------------------看看crx这里进来没--------没进来---2-------\n");
		        printf ("DISCRETE=%d\n",DISCRETE); //DISCRETE=2
		        printf("SpecialStatus[Att]=%d\n",SpecialStatus[Att]);//这里输出是0

			if ( SpecialStatus[Att] == DISCRETE )
			{
			    /*  Add value to list  */

			    Dv = ++MaxAttVal[Att];
			    if ( Dv > (int)AttValName[Att][0] )
			    {
				printf("\nToo many values for %s (max %d)\n",
					AttName[Att], (int) AttValName[Att][0]);
				exit(1);
			    }

			    AttValName[Att][Dv] = CopyString(name);
			}
			else
			{
					//printf("------------1-xxxxxx------");
			    Error(4, AttName[Att], name);
			}
		    }
	        }
	        DVal(Dvec, Att) = Dv;
	    }
	    else
	    {
		/*  Continuous value  */

	        if ( ! ( strcmp(name, "?") ) )
		{
		    Cv = Unknown;
		}
	        else
		{

		    Cv = strtod(name, &endname);
		    if ( endname == name || *endname != '\0' )
		    	{
		    	//printf("---------2----xxxxxx------");
			Error(4, AttName[Att], name);
			}
		}
		CVal(Dvec, Att) = Cv;
	    }

	    ReadName(Df, name);
        }

        if ( (Dv = Which(name, ClassName, 0, MaxClass)) < 0 )
        {
	    Error(5, "", name);
	    Dv = 0;
        }
	Class(Dvec) = Dv;

	return Dvec;
    }
    else
    {
	return Nil;
    }
}
