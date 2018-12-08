# -*- coding: utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
from treePlotter import *
# @Author: appleyuchi
# @Date:   2018-12-05 11:25:47
# @Last Modified by:   appleyuchi
# @Last Modified time: 2018-12-05 13:11:42

def leaves_count(model):#完成
    count=0
    if isinstance(model,str):
        print"model=",model
        return 1
    else:
        bestfeature=model.keys()[0]
        branches=model[bestfeature]
        for branch in branches:
            count+=leaves_count(model[bestfeature][branch])
    return count

def total_node_count(model):#完成
    counts=0
    if isinstance(model,str):
        return 1
    else:
        bestfeature=model.keys()[0]
        branches=model[bestfeature]
        counts=1#current decision node
        for branch in branches:
            counts=counts+total_node_count(model[bestfeature][branch])
    return counts

number=[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19]
def pre_order(model):
    model=str(model)
    for item in number:










if __name__ == '__main__':
    model={'Viscera': {'>0.0145': {'Shell': {'<=0.0345': {'Viscera': {'<=0.0285': ' 5 (50.0/9.0)', '>0.0285': ' 4 (3.0)'}}, '>0.0345': {'Sex': {'=M': ' 6 (6.0/3.0)', '=F': ' 5 (3.0)', '=I': ' 5 (59.0/12.0)'}}}}, '<=0.0145': {'Shucked': {'>0.007': ' 4 (66.0/31.0)', '<=0.007': {'Shucked': {'>0.0045': {'Shucked': {'>0.005': {'Height': {'<=0.02': ' 4 (2.0)', '>0.02': ' 3 (4.0)'}}, '<=0.005': ' 4 (3.0)'}}, '<=0.0045': {'Height': {'<=0.025': ' 1 (2.0/1.0)', '>0.025': ' 3 (2.0)'}}}}}}}}
    # model={'Shell': {'<=0.0345': {'Viscera': {'<=0.0285': ' 5 (50.0/9.0)', '>0.0285': ' 4 (3.0)'}}, '>0.0345': {'Sex': {'=M': ' 6 (6.0/3.0)', '=F': ' 5 (3.0)', '=I': ' 5 (59.0/12.0)'}}}}
    Tt=leaves_count(model)
    createPlot(model)
    print"Tt=",Tt

    total_node=total_node_count(model)
    print"total_node=",total_node

