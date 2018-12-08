#-*- coding: utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
# @Author: appleyuchi
# @Date:   2018-11-08 19:01:14
# @Last Modified by:   appleyuchi
# @Last Modified time: 2018-12-04 21:01:30
# 这个文件的目的是把实验结果一分为二进行保存
import os
path="result.txt"
def mkdir(path):
    # 引入模块
    import os
    # 去除首位空格
    path=path.strip()
    # 去除尾部 \ 符号
    path=path.rstrip("\\")
 
    # 判断路径是否存在
    # 存在     True
    # 不存在   False
    isExists=os.path.exists(path)
 
    # 判断结果
    if not isExists:
        # 如果不存在则创建目录
        # 创建目录操作函数
        os.makedirs(path) 
 
        print path+' 创建成功'
        return True
    else:
        # 如果目录存在则不创建，并提示目录已存在
        print path+' 目录已存在'
        return False
 



def test():
    unprune_start=0
    unprune_end=0
    prune_start=0
    prune_end=0

    with open(path, 'r') as f:
        data = f.readlines()
    for index,item in enumerate(data):
        data[index]=data[index].replace('\n','')
    print data

    for item in data:
        if item=='Decision Tree:':
            record1=data.index(item)
            unprune_start=record1+2

        if item=='Simplified Decision Tree:':
            record2=data.index(item)
            unprune_end=record2-4
            prune_start=record2+2

        if item=='Tree saved':
            record3=data.index(item)
            prune_end=record3-3

    print"-----------------Print unpruned tree--------------------------"
    f = open('./result/unprune.txt','wb')

    for index in range( unprune_start,unprune_end+1):

        if index==unprune_end:
            f.write(data[index])
        else:
            f.write(data[index]+'\n')
    f.close()
    print"-----------------Print pruned tree--------------------------"

    f = open('./result/prune.txt','wb')

    for index in range( prune_start,prune_end+1):
        if index==prune_end:
            # print"尾巴"
            # print data[index]
            f.write(data[index])
        else:
            # print"正常"
            # print data[index]
            f.write(data[index]+'\n')
    f.close()
    # print unprune_start
    # print unprune_end
    # print prune_start
    # print prune_end

if __name__ == '__main__':
    # 定义要创建的目录
    mkpath="./result"
    # 调用函数
    mkdir(mkpath)

    test()