This repository is collecting popular pruning methods(Continuous updating)
--------

Environment

Ubuntu Linux 16.04

Python 2.7.12

Note that :All pruning algorithm are operated on the model generated from:
Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src/quinlan-src/
which is from http://www.rulequest.com/Personal/c4.5r8.tar.gz
its author is Ross Quinlan.


----------------REP---Operation method---------------------------------

For REP(Reduced Error Pruning) of ID3:

    cd ID3-REP-post_prune-Python-draw
    python jianzhi.py
Attention:

    continuous feature and feature with unKnown value are both Not supported to deal currently.
----------------REP---Operation method---------------------------------

----------------EBP--Operation method----------------------------------
Ross Quinlan has already implemented EBP with C,so the following is just a Python interface.  

For EBP(Error Based Pruning),Operation method is:

    For any datasets choosed,you need prepare two files:
    for example:crx.names,crx.data.
    and put them under the path: 
    Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src/quinlan-src/

    cd Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src
    python shell_execute.py crx > result.txt(change "crx" here when you use other datasets please.)
    python result_get.py
    python predict.py

Attention:

    data who contains unknown value is Not supported to predict.
    It's  an Python interface for C-type code http://www.rulequest.com/Personal/c4.5r8.tar.gz

----------------EBP--Operation method---------------------------------

----------------PEP--Operation method---------------------------------  

For PEP(Pessimistic Error Pruning):
1.download datasets from:  
https://archive.ics.uci.edu/ml/machine-learning-databases/abalone/  
2.reorder it with the final column from small to large and get the former 200 items,and save them as abalone_parts.data(this step is just for easy to visualize afterwards)  
3.put abalone_parts.data and abalone.names in:  
/decision_tree/Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src/quinlan-src/

then run the following commands:
	python shell_execute.py abalone > result.txt
	python result_get.py  

get the model from:  
Decision_tree/Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src/result/unprune.txt  

4.put  both abalone_parts.data and abalone.names under the path:  
decision_tree/PEP-finish/
and paste the model gotten from step 3 into top.py  

5.python top.py

----------------PEP--Operation method---------------------------------


You may also interested in the inventer、history of Pruning Algorithms,I have collected them together in the following Link:  
https://blog.csdn.net/appleyuchi/article/details/83692381