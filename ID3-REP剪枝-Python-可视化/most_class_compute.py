# # -*- coding: utf-8 -*-
# import sys
# reload(sys)
# sys.setdefaultencoding('utf-8')

import numpy
import json
import collections
from collections import Counter
#下面这个是西瓜数据集2.0
#         #1
datasets=[['青绿', '蜷缩', '浊响', '清晰', '凹陷', '硬滑', '好瓜'],
        # 2
        ['乌黑', '蜷缩', '沉闷', '清晰', '凹陷', '硬滑', '好瓜'],
        # 3
        ['乌黑', '蜷缩', '浊响', '清晰', '凹陷', '硬滑', '好瓜'],
        # 4
        ['青绿', '蜷缩', '沉闷', '清晰', '凹陷', '硬滑', '好瓜'],
        # 5
        ['浅白', '蜷缩', '浊响', '清晰', '凹陷', '硬滑', '好瓜'],
#--------------------------------------------
        # 6
        ['青绿', '稍蜷', '浊响', '清晰', '稍凹', '软粘', '好瓜'],
        # 7
        ['乌黑', '稍蜷', '浊响', '稍糊', '稍凹', '软粘', '好瓜'],
        # 8
        ['乌黑', '稍蜷', '浊响', '清晰', '稍凹', '硬滑', '好瓜'],
        # 9
        ['乌黑', '稍蜷', '沉闷', '稍糊', '稍凹', '硬滑', '坏瓜'],
        # 10
        ['青绿', '硬挺', '清脆', '清晰', '平坦', '软粘', '坏瓜'],
#--------------------------------------------
        # 11
        ['浅白', '硬挺', '清脆', '模糊', '平坦', '硬滑', '坏瓜'],
        # 12
        ['浅白', '蜷缩', '浊响', '模糊', '平坦', '软粘', '坏瓜'],
        # 13
        ['青绿', '稍蜷', '浊响', '稍糊', '凹陷', '硬滑', '坏瓜'],
        # 14
        ['浅白', '稍蜷', '沉闷', '稍糊', '凹陷', '硬滑', '坏瓜'],
        # 15
        ['乌黑', '稍蜷', '浊响', '清晰', '稍凹', '软粘', '坏瓜'],
        # 16
        ['浅白', '蜷缩', '浊响', '模糊', '平坦', '硬滑', '坏瓜'],
        # 17
        ['青绿', '蜷缩', '沉闷', '稍糊', '稍凹', '硬滑', '坏瓜']]

#下面这个是西瓜数据集2.0a
# datasets = [
#         # 1
#         ['-', '蜷缩', '浊响', '清晰', '凹陷', '硬滑', '好瓜'],
#         # 2
#         ['乌黑', '蜷缩', '沉闷', '清晰', '凹陷', '-', '好瓜'],
#         # 3
#         ['乌黑', '蜷缩', '-', '清晰', '凹陷', '硬滑', '好瓜'],
#         # 4
#         ['青绿', '蜷缩', '沉闷', '清晰', '凹陷', '硬滑', '好瓜'],
#         # 5
#         ['-', '蜷缩', '浊响', '清晰', '凹陷', '硬滑', '好瓜'],
# #--------------------------------------------
#         # 6
#         ['青绿', '稍蜷', '浊响', '清晰', '-', '软粘', '好瓜'],
#         # 7
#         ['乌黑', '稍蜷', '浊响', '稍糊', '稍凹', '软粘', '好瓜'],
#         # 8
#         ['乌黑', '稍蜷', '浊响', '-', '稍凹', '硬滑', '好瓜'],
#         # 9
#         ['乌黑', '-', '沉闷', '稍糊', '稍凹', '硬滑', '坏瓜'],
#         # 10
#         ['青绿', '硬挺', '清脆', '-', '平坦', '软粘', '坏瓜'],
# #--------------------------------------------
#         # 11
#         ['浅白', '硬挺', '清脆', '模糊', '平坦', '-', '坏瓜'],
#         # 12
#         ['浅白', '蜷缩', '-', '模糊', '平坦', '软粘', '坏瓜'],
#         # 13
#         ['-', '稍蜷', '浊响', '稍糊', '凹陷', '硬滑', '坏瓜'],
#         # 14
#         ['浅白', '稍蜷', '沉闷', '稍糊', '凹陷', '硬滑', '坏瓜'],
#         # 15
#         ['乌黑', '稍蜷', '浊响', '清晰', '-', '软粘', '坏瓜'],
#         # 16
#         ['浅白', '蜷缩', '浊响', '模糊', '平坦', '硬滑', '坏瓜'],
#         # 17
#         ['青绿', '-', '沉闷', '稍糊', '稍凹', '硬滑', '坏瓜']
#     ]

def most_class_computes(datasets,satisfy_list):
    print"-------------------most_class_computes--------------"
    print"dataset=",datasets
    print"satisfiy_list=",json.dumps(satisfy_list,ensure_ascii=False)
    m=numpy.matrix(datasets)
    results=[]#保存当前层中,满足所需"特征取值"的所有数据的[行号,列号]
    for item in satisfy_list:
        a=numpy.argwhere(m ==item).tolist()
        b=[x[0] for x in a]#取第0列(也就是满足特征的数据的行序号)
        results.append(b)#找到包含item的那一行,返回[行号,列号]
        # print"------------"
    # print"results=",results
    # 把所有数据的[行号,列号]进行交集运算,找到"同时满足这些特征取值"的数据的下标
    print"results=",results

    results2=[]#保存交集运算结果
    # [val for val in results2 if val in item]
    #下面对ｒｅｓｕｌｔｓ中的所有元素进行交集运算
    for index,item in enumerate(results):
        if index==0:
            results2=item
            continue
        tmp=[val for val in results2 if val in item]#results2与ｉｔｅｍ之间的交集运算
        results2=tmp#初始化为ｒｅｓｕｌｔｓ列表的前两个元素的交集运算结果
    # print"results2=",results2#能走到当前根节点、满足所有特征取值的数据

    result3=[]
    for item in results2:
        # print"item=",item
        result3.append(datasets[item][-1])
    # print json.dumps(result3,ensure_ascii=False)

#统计满足＂特征取值组合＂要求的数量最多的类别并返回
    word_counts = Counter(result3)

    top_first = word_counts.most_common(1)
    print"top_first=",top_first
    print"results2=",results2
    #筛选后的数据集
    sub_datasets=[]
    for i in results2:
        sub_datasets.append(datasets[i])
    return sub_datasets,top_first[0][0]



#返回满足当前根节点的所有特征取值的数据中，占比例最大的分类名
if __name__ == '__main__':
    satisfy_list=['稍凹']
    sub_datasets,result=most_class_computes(datasets,satisfy_list)
    print"result=",result
    print"sub_datasets=",json.dumps(sub_datasets,ensure_ascii=False)