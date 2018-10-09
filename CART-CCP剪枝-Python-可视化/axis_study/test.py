#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
import pandas as pd
import datetime   #用来计算日期差的包

def dataInterval(data1,data2):
    data1=data1.split(" ")[0]
    data2=data2.split(" ")[0]
    print"data1=",data1
    print"data2=",data2
    d1 = datetime.datetime.strptime(data1, '%Y-%m-%d')
    d2 = datetime.datetime.strptime(data2, '%Y-%m-%d')
    delta = d1 - d2
    print"delta=",delta
    return delta.days

def getInterval(arrLike):  #用来计算日期间隔天数的调用的函数
    print"type(arrLike)=",type(arrLike)#<class 'pandas.core.series.Series'>
    PublishedTime=arrLike['PublishedTime']
    ReceivedTime =arrLike['ReceivedTime']
    # days = dataInterval(PublishedTime.strip(),ReceivedTime.strip())  #注意去掉两端空白
    days = dataInterval(str(PublishedTime).strip(),str(ReceivedTime).strip())
    return days


if __name__ == '__main__':    
    fileName="test.xls"
    df = pd.read_excel(fileName) 
    print"df=",df
    print"type(df)=",type(df)#<class 'pandas.core.frame.DataFrame'>
    df['TimeInterval'] = df.apply(getInterval , axis = 1)
    print"#################################"
    print df.ReceivedTime
    print type(df.ReceivedTime)
    print"#################################"
    print"df['TimeInterval']=",df['TimeInterval']

#参考链接:
# https://blog.csdn.net/qq_19528953/article/details/79348929