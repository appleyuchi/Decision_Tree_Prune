This repository is targeted at popular pruning implementions(Continuous updating):

REP:Reduced Error Pruning(finished)

MEP:Minimum Error Pruning(finished)

PEP:Pessimistic Error Pruning(finished)

EBP:Error Based Pruning(finished)

CVP：Critical Value Pruning(finished)

CCP:Cost Complexity Pruning(NOT finished yet)

ECP:Error Complexity Pruning(NOT finished yet)


--------
	Tips:
	The command to delete all .pyc files:
    find . -name "*.pyc"  | xargs rm -f

--------

	Environment
	Ubuntu Linux 16.04-Amd64
	Python 2.7.12

Note that :  
①Except ID3,MEP、EBP、PEP、CVP are operated on the model generated from:
Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src/quinlan-src/
which is from  
http://www.rulequest.com/Personal/c4.5r8.tar.gz  
its author is Ross Quinlan.
The model from Quinlan's implemention is C-type,and it will be transformed to be "Python-compliant" model afterwards.

②datasets with unKnown value is Not Supported,because under different cases or in different papers,different people have different methods to deal with unknown value.　　

③The "C4.5-Release decision tree model" produced by quinlan's implemention in the above link is C-type model,Not python-type model,So we need to do transformation before running pruning algorithm.  
For further details about transformation,continue reading the following contents please.

④when your datasets is very very small ,you'll get very small model,then,you will NOT get a "Simplified Tree"(pruned model)from quinlan's implemention.
This means "pruned model"="unpruned model",
when this happend,copy the content of "result/unprune.txt" to  "result/prune.txt"please.


----------------REP---Operation method(start)---------------------------------

For REP(Reduced Error Pruning) of ID3:

    cd ID3-REP-post_prune-Python-draw
    python jianzhi.py
Attention:

    continuous feature and feature with unKnown value are both Not supported currently.
----------------REP---Operation method(end)---------------------------------

----------------EBP--Operation method(start)----------------------------------  

Ross Quinlan has already implemented EBP with C,
so the following is just a Python interface.  

For EBP(Error Based Pruning),Operation method is:

    For any datasets choosed,you need prepare two files:
    for example:  
    crx.names  
    crx.data.
    and put them under the path: 
    Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src/quinlan-src/

    cd Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src
    python shell_execute.py crx > result.txt(change "crx" here when you use other datasets please.)
    python result_get.py(transform C model to Python model)  
    python predict.py
***************************

    It's  an Python interface for C-type code in:  
    http://www.rulequest.com/Personal/c4.5r8.tar.gz
    This interface is used to get model and EBP pruned model from Quinlan's implemention.

----------------EBP--Operation method(end)--------------------------------- 

----------------PEP--Operation method(start)---------------------------------  

For PEP(Pessimistic Error Pruning):  
1.download datasets from:  
https://archive.ics.uci.edu/ml/machine-learning-databases/abalone/  
2.reorder it with the final column from small to large and get the former 200 items,and save them as abalone_parts.data(this step is just for easy to visualize afterwards)  
3.

	cp abalone_parts.data abalone.names   /decision_tree/Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src/quinlan-src/
    python shell_execute.py abalone > result.txt
	python result_get.py(transform C model to Python model)  

	get the model from the output of "python result_get.py"
	and paste this model into top.py  

4.

	cp abalone_parts.data  abalone.names  decision_tree/PEP-finish/
 
    python top.py  

***************************
	If you do not want to change datasets,
    you can skip the former 4 steps and run step 5th only.


----------------PEP--Operation method(end)---------------------------------

----------------MEP--Operation method(start)---------------------------------  

	1.cp abalone_parts.data abalone.names   /decision_tree/Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src/quinlan-src/
    python shell_execute.py abalone > result.txt
	python result_get.py(transform C model to Python model)  

	2.get the model from the output of "python result_get.py"
	and paste this model into MEP_topmodule_down_to_top.py  

	3.python MEP_topmodule_down_to_top.py　

----------------MEP--Operation method(end)---------------------------------  

----------------CVP--Operation method(start)---------------------------------  

	1.cp abalone_parts.data abalone.names   /decision_tree/Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src/quinlan-src/
    python shell_execute.py abalone > result.txt
	python result_get.py(transform C model to Python model)  

	2.get the model from the output of "python result_get.py"
	and paste this model into CVP_top.py 

	3.search "critical_value" in CVP_top.py and change it to be what you want.
	4.python CVP_top.py
of course,you can skip the first 3 steps if you just want to see its performance with default datasets(abalone_parts and credit-a) 

----------------CVP--Operation method(end)---------------------------------  


----------------CCP--Operation method(some information)---------------------------------  

Attention:
    By far,there have are 3 implementions of CCP on CART by others,but all of them have defects.

1.
https://github.com/Rudo-erek/decision-tree/tree/master/data
It can NOT work,and it can NOT deal with continuous Attribute.
The computation of R(t) is wrong,this link use Gini to compute R（t）

2.
https://github.com/Jasper-Dong/Decision-Tree
It can work,but it can NOT deal with continuous Attribute

3.
https://triangleinequality.wordpress.com/2013/09/01/decision-trees-part-3-pruning-your-tree/
It can work,but the author modified the original CCP,which result in no candidate trees to select,
and so no cross-validation to select best pruned tree.
He set a fixed alpha,before run this"modified CCP",the method is to pursue min|R(t)-R(Tt)-a(|Ｔｔ|－１)|.

All of above three implementions are stored and annotated in the folder:"several_wrong_implementions_CCP"


Note:
    This repositories' CCP-implemention is performed based on Quinlan's C4.5 model.

    Of course,we know CCP is proposed based on CART,CCP on CART of sklearn is still on discussion by sklearn's contributors:
    https://github.com/scikit-learn/scikit-learn/issues/6557
    Pruning on sklearn's CART model is still un-available,because some parts of it are written in Cython which is hard to modify.

    CCP  can also be used on C4.5,and the article which use CCP on C4.5 is《Simplifying Decision Trees》ｐａｒｔ2.1-J.R.Quinlan

    Our target is:Use the best decision model,and implement the pruning algorithm identical to the paper which proposed it.
    So,we do NOT plan to use the CART model which are not written by experts,because they have too much defects as mentioned earlier.

----------------CCP--Operation method(some information)---------------------------------  



You may also interested in the inventer、history of Pruning Algorithms,I have collected them together in the following Link:  
https://blog.csdn.net/appleyuchi/article/details/83692381

Don't hesitate to contact appleyuchi@foxmail.com please if you have any question.

