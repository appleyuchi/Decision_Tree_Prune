#-*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
import pandas as pd
model={'Viscera': {'>0.0145': {'Shell': {'<=0.0345': {'Viscera': {'<=0.0285': ' 5 (50.0/9.0)', '>0.0285': ' 4 (3.0)'}}, '>0.0345': {'Sex': {'=M': ' 6 (6.0/3.0)', '=F': ' 5 (3.0)', '=I': ' 5 (59.0/12.0)'}}}}, '<=0.0145': {'Shucked': {'>0.007': ' 4 (66.0/31.0)', '<=0.007': {'Shucked': {'>0.0045': {'Shucked': {'>0.005': {'Height': {'<=0.02': ' 4 (2.0)', '>0.02': ' 3 (4.0)'}}, '<=0.005': ' 4 (3.0)'}}, '<=0.0045': {'Height': {'<=0.025': ' 1 (2.0/1.0)', '>0.025': ' 3 (2.0)'}}}}}}}}
import copy
from split import splitdatasets


def get_Attribute(path):
    feature_list=[]
    for line in open(path):
        if ":" in line:
            feature_list.append(line.split(":")[0])
    return feature_list



#here we set the frequency of each class as priority probability,of course it's not enough precise
#
#if you know the real priority probability of each class,set the pae_list manuallly please
def pae_list(path):
    dicts={}
    data=read_data(path)
    length=len(data)*1.0
    items=[item[-1] for item in data ]
    # classes=list(set(items))
    for item in items:
        if item not in dicts:
            dicts[item]=1
        else:
            dicts[item]=dicts[item]+1

    dicts_backup=copy.deepcopy(dicts)
    for key in dicts:
        print dicts[key]/length
        dicts[key]=dicts[key]/length
    return dicts,dicts_backup

#              N-ne+(1-pae)ｍ
# Es=min{-------------}
#                    N+m
#Let's put denominator aside .
#Let's consider numerator 
#please don't think that just choose the subsets who ows the largest number of data
#because when ne is largest,pae may be very small ,then the whole numerator may not be the minimum

# ｄａｔａｓｅｔｓ:changes when iterate
# pae_lists:never change,determined by datasets
# count_list:changes when iterate


def static_error(datasets,pae_list,count_list,class_list,m):
    #pae_list=count_list/len(datasets)
    N=len(datasets)

#because we do Not know which "ne" will make "Es" reach minumum,
#so we get "ne candidata lists" from class_count
    ne_candidate_lists=[]
    for item in count_list:
        ne_candidate_lists.append(item)
    print"ne_candidata_lists=",ne_candidate_lists

    Es_candidate_list=[]
    for ind,ne_candidate in enumerate(ne_candidate_lists):
        Es_candidate=(N-ne_candidate+(1.0-pae_list[ind])*m)*1.0/(N+m)
        Es_candidate_list.append(Es_candidate)

    Es_index=-1
    Es_min=99999999
    for index,item in enumerate(Es_candidate_list):
        if item<Es_min:
            Es_min=item
            Es_index=index
    # print"Es_min=",Es_min
    # print"Es_min_index=",Es_index
    # print"满足Es的类别是",class_list[Es_index]
    return Es_min,Es_index,class_list[Es_index]

def subdata_counts(sub_data,class_list):
    count_list=[0 for item in range(0,len(class_list))]
    for item in sub_data:
        count_list[class_list.index(item[-1])]+=1
    return count_list




# def backed_up_error(model,datasets,pae_list,class_list,m,fea_list):
#     best_feature=model.items()[0][0]
#     print"best_feature=",best_feature
#     branches=model[best_feature]
#     length=len(datasets)

#     pi_list=[]
#     Eb=0.0
#     # print"fea_list=",fea_list
#     for index,branch in enumerate(branches):#获取ｋｅｙ，也就是ｂｒａｎｃｈ上面的取值
#         sub_data=splitdatasets(best_feature,fea_list,branch,datasets)
#         pi_list.append(len(sub_data)*1.0/length)
#         count_list=subdata_counts(sub_data,class_list)#子数据集可能存在“有些类别没有数据”的情况，所以必须根据类别来进行统计
#         # print "count_list=",count_list
#         Ei,_,_=static_error(sub_data,pae_list,count_list,class_list,m)
#         # print "pi_list[index]=",pi_list[index]
#         # print "Ei=",Ei
#         # print"pi_list[index]=",pi_list[index]
#         # print"pi_list[index]*Ei=",pi_list[index]*Ei
#         Eb=Eb+pi_list[index]*Ei#Eb=ΣＰｉ·Ｅｉ
#     return Eb


def backed_up_error(model,datasets,pae_list,class_list,m,fea_list):
    if isinstance(model,str):
        count_list=subdata_counts(datasets,class_list)
        Es_min,_,_=static_error(datasets,pae_list,count_list,class_list,m)
        return Es_min

    best_feature=model.items()[0][0]
    print"best_feature=",best_feature
    branches=model[best_feature]
    length=len(datasets)


    # print"fea_list=",fea_list
    pi_list=[]
    Eb=0.0
    for index,branch in enumerate(branches):#获取ｋｅｙ，也就是ｂｒａｎｃｈ上面的取值
        sub_data=splitdatasets(best_feature,fea_list,branch,datasets)
        pi_list.append(len(sub_data)*1.0/length)#Pi
        Ei=backed_up_error(model[best_feature][branch],sub_data,pae_list,class_list,m,fea_list)
        Eb=Eb+pi_list[index]*Ei#Eb=ΣＰｉ·Ｅｉ
    return Eb
   






def read_data(path):
    source_data=pd.read_csv(path,header=None)#第一行不是标题
    lists=source_data.values.tolist()
    return lists
if __name__ == '__main__':
    path="./abalone_parts.data"
    name_path='./abalone.names'
    m=2
    datasets=read_data(path)
    pae_dict,class_count=pae_list(path)#不要进入递归，这个是剪枝前就要确定下来，并且在剪枝的过程中不可改变的。
    pae_list=[pae_dict[key] for key in pae_dict]#获得先验概率列表
    print "pae_list=",pae_list
    class_list=[key for key in class_count]#获取数据集的类别列表
# －－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－
    feature_list=get_Attribute(name_path)
    # print "feature_list=",feature_list
    Eb=backed_up_error(model,datasets,pae_list,class_list,m,feature_list)
    print "Eb=",Eb

# －－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－
    # 下面这个要根据子数据集进行额外的计算，不可以使用上面的
    count_list=[class_count[key] for key in class_count]#每一类有多少数据，这个用来计算先验概率
    current_data=datasets
    print"----------------------"#注意ｐａｅ的求解最后要剥离出来，不要根据当前数据集来求解。
    static,_,_=static_error(current_data,pae_list,count_list,class_list,m)#计算当前节点的Ｓｔａｔｉｃ　ｅｒｒｏｒ，不需要传入模型，直接对当前数据集进行分析即可
    print"static_error=",static







