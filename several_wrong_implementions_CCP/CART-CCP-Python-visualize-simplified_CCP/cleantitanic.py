# -*- coding: utf-8 -*-
from __future__ import print_function
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
import pandas as pd
import numpy as np
from scipy.stats import mode

# pd.set_option('display.width', None)  # 设置字符显示宽度
pd.set_option('display.max_rows', None)  # 设置显示最大行


def cleandf(df):
    #cleaning fare column
    df.fare = df.fare.map(lambda x: np.nan if x==0 else x)#①
    classmeans = df.pivot_table('fare', index='pclass', aggfunc='mean') 
    # print("type(classmeans)=",type(classmeans))
    # print("classmeans.fare=",classmeans.fare)
    # print("classmeans['1']=",classmeans.fare[1])#☆☆☆☆☆☆☆☆☆☆☆☆
    # print("type(classmeans.fare)=",type(classmeans.fare))
    # print("####################")
    # df.fare = df.apply(lambda x:print("这里的x",x['pclass'])  if pd.isnull(x['fare']) else x['fare'], axis=1 )#这句话是用来为下面一句话调试用的.
    # df.fare = df[['fare', 'pclass']].apply(lambda x:classmeans[x['pclass']] if pd.isnull(x['fare']) else x['fare'], axis=1 )
    #上面这句是原来的代码,会出现KeyError: (1, u'occurred at index 49')的报错
    #为什么会出现keyerror呢?
    # 因为这里的classmeans是<class 'pandas.core.frame.DataFrame'>类型
    #如果直接classmeans[1]的话,导致keyerror,因为DataFrame的key是列名,这里传入的1是该列中的一个取值
    #会被dataframe误以为是列名,所以会报错
    df.fare = df[['fare', 'pclass']].apply(lambda x:classmeans.fare[x['pclass']] if pd.isnull(x['fare']) else x['fare'], axis=1 )
    #这句代码有些不带好理解,注意,python中有如下规律:
    # >>> pd.isnull(np.nan)
    # True
    #这里的isnull并不是直接对原始数据集进行判断,而是对①处理后的数据进行判断
    #①中的处理是:如果fare这一列中有一个单元格为0,那么df.fare中对应的值就是np.nan
    # --------------------------------------------------------------------
    #这里整句话的意思是,如果发现某个船上的游客的收入为0,那就采用pclass相同的这一类人的平均fare作为填充值
    #详细些来讲,如果x(某个游客)的收入是np.nan,那么if语句中会判定为True,就会执行x->classmeans.fare[x['pclass']的映射关系

    # https://stackoverflow.com/questions/2970858/why-doesnt-print-work-in-a-lambda
    #在lambda中调试的函数
    
    #cleaning the age column
    meanage=np.mean(df.age)
    df.age=df.age.fillna(meanage)
    
    #cleaning the embarked column
    df.cabin = df.cabin.fillna('Unknown')
    modeEmbarked = mode(df.embarked)[0][0]
    df.embarked = df.embarked.fillna(modeEmbarked)
    return df

def cleaneddf(no_bins=0):
    #you'll want to tweak this to conform with your computer's file system
    trainpath ="./data/train.csv"
    testpath  ="./data/test.csv"
    # trainpath = '../input/train.csv'
    # testpath = '../input/test.csv'
    traindf = pd.read_csv(trainpath)
    # print("traindf=",traindf)
    testdf = pd.read_csv(testpath)
    
    #discretise fare
    if no_bins==0:
        result1=cleandf(traindf)
        result2=cleandf(testdf)
        return [result1,result2]
    traindf=cleandf(traindf)
    testdf=cleandf(testdf)
    bins_and_binned_fare = pd.qcut(traindf.fare, no_bins, retbins=True)
    bins=bins_and_binned_fare[1]
    traindf.fare = bins_and_binned_fare[0]
    testdf.fare = pd.cut(testdf.fare, bins)
    
    #discretise age
    bins_and_binned_age = pd.qcut(traindf.age, no_bins, retbins=True)
    bins=bins_and_binned_age[1]
    
    traindf.age = bins_and_binned_age[0]
    testdf.age = pd.cut(testdf.age, bins)
    
    #create a submission file for kaggle
    predictiondf = pd.DataFrame(testdf['PassengerId'])
    predictiondf['Survived']=[0 for x in range(len(testdf))]
    predictiondf.to_csv('prediction.csv',index=False)
    traindf,to_csv('traindf.csv',index=False)
    testdf.to_csv('testdf.csv',index=False)
    return [traindf, testdf]
if __name__ == '__main__':
    df=cleaneddf()