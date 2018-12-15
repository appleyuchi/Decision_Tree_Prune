#-*- coding:UTF-8 -*-


#第一步：读取波士顿房价数据
from sklearn.datasets import load_boston
boston = load_boston()
print boston.DESCR
#从输出结果来看，该数据共有506条波士顿房价的数据，每条数据包括对指定房屋的13项数值型特征和目标房价
#此外，该数据中没有缺失的属性/特征值，更加方便了后续的分析

#第二步：波士顿房价数据分割
from sklearn.model_selection import train_test_split
import numpy as np
X_train,X_test,y_train,y_test = train_test_split(boston.data,boston.target,test_size=0.25,random_state=33)
#分析回归目标值的差异
# print 'The max target value is ',np.max(boston.target)
# print 'The min target value is ',np.min(boston.target)
# print 'The average target value is ',np.mean(boston.target)

#第三步：训练数据和测试数据标准化处理
from sklearn.preprocessing import StandardScaler
#分别初始化对特征值和目标值的标准化器
ss_X = StandardScaler()
ss_y = StandardScaler()
#训练数据都是数值型，所以要标准化处理
print"type(X_train)=",type(X_train)#<type 'numpy.ndarray'>
print"type(X_test)=",type(X_test)  #<type 'numpy.ndarray'>
X_train = ss_X.fit_transform(X_train)
X_test = ss_X.transform(X_test)
#目标数据（房价预测值）也是数值型，所以也要标准化处理
#说明一下：fit_transform与transform都要求操作2D数据，而此时的y_train与y_test都是1D的，因此需要调用reshape(-1,1)，例如：[1,2,3]变成[[1],[2],[3]]
y_train = ss_y.fit_transform(y_train.reshape(-1,1))
y_test = ss_y.transform(y_test.reshape(-1,1))

#第四步：使用线性回归模型LinearRegression和SGDRegressor分别对美国房价进行预测
#不要搞混了，这里用的是LinearRegression而不是线性分类的LogisticRegression
from sklearn.linear_model import LinearRegression
lr = LinearRegression()
lr.fit(X_train,y_train)
lr_y_predict = lr.predict(X_test)
from sklearn.linear_model import SGDRegressor
sgdr = SGDRegressor()
sgdr.fit(X_train,y_train)
sgdr_y_predict = sgdr.predict(X_test)

#第五步：性能测评
#主要是判断预测值与真实值之间的差距，比较直观的评价指标有
#平均绝对值误差MAE(mean absolute error)
#均方误差MSE(mean squared error)
#R-squared评价函数
#使用LinearRegression模型自带的评估模块，并输出评估结果
# print 'the value of default measurement of LR：',lr.score(X_test,y_test)
from sklearn.metrics import r2_score,mean_squared_error,mean_absolute_error
# print 'the value of R-squared of LR is',r2_score(y_test,lr_y_predict)
#可以使用标准化器中的inverse_transform函数还原转换前的真实值
# print 'the MSE of LR is',mean_squared_error(ss_y.inverse_transform(y_test),ss_y.inverse_transform(lr_y_predict))
# print 'the MAE of LR is',mean_absolute_error(ss_y.inverse_transform(y_test),ss_y.inverse_transform(lr_y_predict))
#使用SGDRegressor自带的评估模块，并输出评估结果
# print 'the value of default measurement of SGDR：',sgdr.score(X_test,y_test)
from sklearn.metrics import r2_score,mean_squared_error,mean_absolute_error
# print 'the value of R-squared of SGDR is',r2_score(y_test,sgdr_y_predict)
# print 'the MSE of SGDR is',mean_squared_error(ss_y.inverse_transform(y_test),ss_y.inverse_transform(sgdr_y_predict))
# print 'the MAE of SGDR is',mean_absolute_error(ss_y.inverse_transform(y_test),ss_y.inverse_transform(sgdr_y_predict))

#总结：
#从输出结果来看，回归模型自带的评估结果与r2_score的值是一样的，推荐使用第一种方式
#SGDRegressor在性能上表现略逊于LinearRegression，前者是随机梯度下降的方式估计参数，后者是精确解析参数
#在数据量十分庞大（10W+）的时候，推荐使用SGDRegressor
