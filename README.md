This repository is targeted at popular pruning implementions(Continuous updating)
--------
	Tips:
	The command to delete all .pyc files:
    find . -name "*.pyc"  | xargs rm -f

--------

	Environment
	Ubuntu Linux 16.04-Amd64
	Python 2.7.12

Note that :  
①All pruning algorithm are operated on the model generated from:
Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src/quinlan-src/
which is from  
http://www.rulequest.com/Personal/c4.5r8.tar.gz  
its author is Ross Quinlan.
Only its interface is modified for much more convient use.  
②datasets with unKnown value is Not Supported,because under different cases or in different papers,different people have different methods to deal with unknown value.


----------------REP---Operation method---------------------------------

For REP(Reduced Error Pruning) of ID3:

    cd ID3-REP-post_prune-Python-draw
    python jianzhi.py
Attention:

    continuous feature and feature with unKnown value are both Not supported currently.
----------------REP---Operation method---------------------------------

----------------EBP--Operation method----------------------------------  

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
    python result_get.py
    python predict.py
***************************

    It's  an Python interface for C-type code in:  
    http://www.rulequest.com/Personal/c4.5r8.tar.gz
    This interface is used to get model and EBP pruned model from Quinlan's implemention.

----------------EBP--Operation method--------------------------------- 

----------------PEP--Operation method---------------------------------  

For PEP(Pessimistic Error Pruning):  
1.download datasets from:  
https://archive.ics.uci.edu/ml/machine-learning-databases/abalone/  
2.reorder it with the final column from small to large and get the former 200 items,and save them as abalone_parts.data(this step is just for easy to visualize afterwards)  
3.

	cp abalone_parts.data abalone.names   /decision_tree/Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src/quinlan-src/
    python shell_execute.py abalone > result.txt
	python result_get.py  

get the model from:  
Decision_tree/Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src/result/unprune.txt  

4.

	cp abalone_parts.data  abalone.names  decision_tree/PEP-finish/
    paste the model gotten from step 3 into top.py 
    python top.py  

***************************
	If you do not want to change datasets,
    you can skip the former 4 steps and run step 5th only.


----------------PEP--Operation method---------------------------------


You may also interested in the inventer、history of Pruning Algorithms,I have collected them together in the following Link:  
https://blog.csdn.net/appleyuchi/article/details/83692381