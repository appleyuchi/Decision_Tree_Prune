This repository is targeted at popular pruning implementations(Continuous updating):





|Name | Full name |  Tree type|On Which package is Pruning Algorithm Performed|Language|Unknown value supported|Continuous value supported|
|------------- |------------- |------------- |------------- |------------- |------------- |------------- |
|REP | Reduced Error Pruning | ID3|Personal<br>Manually Code|Python|No|No|
|MEP | Minimum Error Pruning| C4.5|[1]|Python|No|Yes|
|CVP | Critical Value Pruning| C4.5|[1]| Python|No|Yes|
|PEP | Pessimistic Error Pruning| C4.5|[1]| Python|No|Yes|
|EBP|Error Based Pruning|C4.5|[1]|Python|No|Yes|
|EBP|Error Based Pruning|C4.5|[1]|C|Yes|Yes|
|EBP|Error Based Pruning|C5.0|[4]|C|Yes|Yes|
|CCP|Cost Complexity Pruning|CART-<br>Classification Tree|sklearn|Python|No|Yes|
|ECP|Error Complexity Pruning|CART-<br>Regression Tree|sklearn|Python|No|Yes|

ID3 is just manually written code.
MEP、EBP、PEP、CVP are operated on the model generated from:
Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src/quinlan-src/
which is from[1],the inventer's homepage.



Environment Requirement(Not a must):

| Environment | Edition |Command to find Edition|
|------------- |------------- |------------- |
|XUbuntu 18.10|18.10-Amd64|uname -a|
|Python|2.7.12|python|
|Make|4.2.1|make --version|
|Glibc|2.28|ldd --version|



--------
	Tips:
	The command to delete all .pyc files:
    find . -name "*.pyc"  | xargs rm -f



Note that :  
In C4.5 and C5.0,when your datasets is very very small ,

you'll get very small model,then,

you will NOT get a "Simplified Tree"(pruned model)

from quinlan's implementation.

This means "pruned model"="unpruned model",

when under this case,copy the content of "result/unprune.txt" to  "result/prune.txt"please.


----------------REP---Operation method(start)-------------------------------

Running Flows:

1.cd ID3-REP-post_prune-Python-draw

2.python jianzhi.py

----------------REP---Operation method(end)---------------------------------





----------------EBP--Operation method(start)--------------------------------

Ross Quinlan has already implemented EBP with C[1],
so the following is just a Python interface.




Running Flows:

1.
put crx.names and crx.data(change "crx"  please when you use other datasets)
under the path: 
    Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src/quinlan-src/
2.

    cd Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src
    python shell_execute.py crx > result.txt
    python result_get.py(transform C model to Python model)
    python predict.py

----------------EBP--Operation method(end)---------------------------------

----------------PEP--Operation method(start)-------------------------------

Running Flows:
1.
Download datasets from[2]  

2.
Reorder it with the final column from small to large and get the former 200 items,and save them as abalone_parts.data(this step is just for easy to visualize afterwards)  
3.

	cp abalone_parts.data abalone.names   /decision_tree/Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src/quinlan-src/
    python shell_execute.py abalone > result.txt
	python result_get.py(transform C model to Python model)  

	get the model from the output of "python result_get.py"
	and paste this model into top.py  

4.
cp abalone_parts.data  abalone.names  decision_tree/PEP-finish/

5.
python top.py  

***************************
	If you do not want to change datasets,
    you can skip the former 4 steps and run step 5th only.


----------------PEP--Operation method(end)---------------------------------

----------------MEP--Operation method(start)---------------------------------  

Running Flows:

	1.cp abalone_parts.data abalone.names   /decision_tree/Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src/quinlan-src/
    python shell_execute.py abalone > result.txt
	python result_get.py(transform C model to Python model)  

	2.get the model from the output of "python result_get.py"
	and paste this model into MEP_topmodule_down_to_top.py  

	3.python MEP_topmodule_down_to_top.py　

----------------MEP--Operation method(end)---------------------------------  

----------------CVP--Operation method(start)--------------------------------- 

Running Flows:

	1.cp abalone_parts.data abalone.names   /decision_tree/Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src/quinlan-src/
    python shell_execute.py abalone > result.txt
	python result_get.py(transform C model to Python model)  

	2.get the model from the output of "python result_get.py"
	and paste this model into CVP_top.py 

	3.search "critical_value" in CVP_top.py and change it to be what you want.
	4.python CVP_top.py
Of course,you can skip the first 3 steps if you just want to see its performance with default datasets(abalone_parts and credit-a) 

----------------CVP--Operation method(end)---------------------------------  


----------------CCP--Operation method(some relevant information-start)---------------------------------  





Attention:
Previous Work of CCP on CART From other repositories in Github and Google:

| Link | Defects |
|------------- |------------- |
| https://github.com/Rudo-erek/decision-tree/tree/ | It can NOT work,and it can NOT deal with continuous Attribute.<br>The computation of R(t) is wrong,this link use Gini to compute R(t). | 
| https://github.com/Jasper-Dong/Decision-Tree| It can work,but it can NOT deal with continuous Attribute. |
|[Decision Trees Part 3: Pruning your Tree](https://triangleinequality.wordpress.com/2013/09/01/decision-trees-part-3-pruning-your-tree/)|It can work,<br>but the author modified the original CCP,<br>which result in no candidate trees to select,<br>and so no cross-validation to select best pruned tree.<br>He set a fixed alpha before running this“modified CCP”,his method is to pursue:<br> min&#124;R(t)-R(Tt)-α(&#124;Tt&#124;-1)&#124;|



All of above three implementations are stored and annotated in the folder:"several_wrong_implementations_CCP"


	Note1:
    This repository's CCP-implementation is performed based on sklearn's CART classification-model.
	Here's the relevant issuse of official github of Sklearn.
    https://github.com/scikit-learn/scikit-learn/issues/6557
    They are trying to prune on sklearn's CART model with Cython(a faster python with C),which is still un-available.My  CCP implementation on sklearn's CART model is pure python.

	Note2:
    CCP  can also be used on C4.5(Not supported in this github),and the article which use CCP on C4.5 is《Simplifying Decision Trees》part2.1-J.R.Quinlan



----------------CCP--Operation method(some relevant information-end)---------------------------------


----------------CCP--Operation method(start)---------------------------------  
	Attention again,datasets with unKnown value is NOT supported.


	The main flow to implement CCP on sklearn's model is as follows:
	1.transform sklearn model to json-model. 
	(I'm Not contributor of Sklearn,so the sklearn model can NOT be pruned directly,it need transformation.)
	2.perform CCP on json model
	3.get the best json-model from Tree Sets in CCP,and synchronized the original sklearn model with the best json-model
	(we only synchronize the"Tree shape" between sklearn-model and json-style model,which is very helpful for drawing CCP-pruned json-model with graphviz)


	The step to run CCP on sklearn's model is as follows:
    1.delete all the files in the folder "visualization"
	2.make sure your datasets has no unKnown value,or you need to pre-process it(It is a must).
	If you don't pre-process the unKnown value,strange errors will occur.
	3.cd decision_tree/sklearn_cart_CCP/sklearnCART2json_CCP/ 
	and change the "name_path" and "data_path" in sklearn_CCP_top.py
	4.python sklearn_CCP_top.py 
	and get the best CCP-json-model and CCP-pruned  precision
	5.Enjoy the pictures of all the TreeSets and "final bestTree" in the folder "visualization".

	Note:
        1.
        Don't Set "max_depth" too large,
        or you'll need to wait for a very long time,
        if you persist running it with very large "max_depth",then graphviz may NOT be able to draw pictures under the folder "visualization".

        2.
	datasets from UCI which have been tested:
	credit-a
	abalone


----------------CCP--Operation method(end)--------------------------------- 

-------------------ECP-Operation method(start)-----------------------

Running Flows:


        1.change your datasets path in file sklearn_ECP_TOP.py
        2.set b_SE=True in sklearn_ECP_TOP.py if you want this rule to select the best pruned tree.
        3.python sklearn_ECP_TOP.py in the path decision_tree/sklearn_cart-regression_ECP-finish/
        4.Enjoy the results in the folder＂visualization＂.

        datasets from UCI which have been tested:
        housing(boston)
        

-------------------ECP-Operation method(end)----------------------- 

\----------------------------------------



-------------------C5.0-EBP-Operation method(Start)-------------------

Training Flows:
C50-EBP-finish/Train/Traing_Method.txt

Validation Flows:
C50-EBP-finish/ValidateAndTest/Validation_Testing_Method.txt


The resource of C5.0 is from [4] (For Training)
and [5] (For Validation and Testing)

-------------------C5.0-EBP-Operation method(end)-------------------


You may also interested in the inventer、history of Pruning Algorithms,and you may want to compare the unpruned effects and pruned effects.I have collected them together[3].



\----------------------------Formatting  For Kaggle---------------------------------------

Change .csv to .data and .names.
See folder:
sCSV_Formatting



\----------------------------Contact me------------------------------------------------

|Contact Style | Information|
|------------- |------------- |
|Email | appleyuchi@foxmail.com|
|Wechat|appleyuchi|

Reference:

[1][C4.5 Package](http://www.rulequest.com/Personal/c4.5r8.tar.gz)

[2][Abalone Datasets](https://archive.ics.uci.edu/ml/machine-learning-databases/abalone/  )

[3][History of pruning algorithm development and python implementation(finished)](https://blog.csdn.net/appleyuchi/article/details/83692381)

[4][C5.0 Package](https://rulequest.com/GPL/C50.tgz)

[5][See5/C5.0](https://rulequest.com/see5-public.tgz)



