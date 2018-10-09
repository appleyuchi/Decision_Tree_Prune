# ID3-Decision-Tree-Classifier-in-Java

```
Classes: 1 = >50K, 2 = <=50K

Attributes
age: continuous.
workclass: Private, Self-emp-not-inc, Self-emp-inc, Federal-gov, Local-gov, State-gov, Without-pay, Never-worked.
fnlwgt: continuous.
education: Bachelors, Some-college, 11th, HS-grad, Prof-school, Assoc-acdm, Assoc-voc, 9th, 7th-8th, 12th, Masters, 1st-4th, 10th, Doctorate, 5th-6th, Preschool.
education-num: continuous.
marital-status: Married-civ-spouse, Divorced, Never-married, Separated, Widowed, Married-spouse-absent, Married-AF-spouse.
occupation: Tech-support, Craft-repair, Other-service, Sales, Exec-managerial, Prof-specialty, Handlers-cleaners, Machine-op-inspct, Adm-clerical, Farming-fishing, Transport-moving, Priv-house-serv, Protective-serv, Armed-Forces.
relationship: Wife, Own-child, Husband, Not-in-family, Other-relative, Unmarried.
race: White, Asian-Pac-Islander, Amer-Indian-Eskimo, Other, Black.
sex: Female, Male.
capital-gain: continuous.
capital-loss: continuous.
hours-per-week: continuous.
native-country: United-States, Cambodia, England, Puerto-Rico, Canada, Germany, Outlying-US(Guam-USVI-etc), India, Japan, Greece, South, China, Cuba, Iran, Honduras, Philippines, Italy, Poland, Jamaica, Vietnam, Mexico, Portugal, Ireland, France, Dominican-Republic, Laos, Ecuador, Taiwan, Haiti, Columbia, Hungary, Guatemala, Nicaragua, Scotland, Thailand, Yugoslavia, El-Salvador, Trinadad&Tobago, Peru, Hong, Holand-Netherlands.

```
<hr>

**Procedure:**
1. Decision tree was generated using the data provided and the ID3 algorithm mentioned in Tom. M. Mitchell.
2. Missing values were filled using the value which appeared most frequently in the particular attribute column.
3. Continuous values were handled as mentioned in section 3.7.2 of Tom M. Mitchell. First the values were sorted in ascending order, then at the points where value was changing, gain was calculated and finally the column was
splited at the point where maximum gain was obtained.
4. Reduced Error Pruning was performed by removing a node (one by one) and then checking the accuracy. If accuracy was increased than the node was removed else we move on to check the next node.
5. Random forests were generated using 50% attributes and 33% data randomly. 10 forests were generated and accuracy increased compared to
the original ID3 algorithm.

<hr>

```
Output:
Start...

Prepocessing Training data

Prepocessing Testing data

Generating Decision Tree using ID3 Algorithm
Training Time=1.979secs
Accuracy=0.807874209200909
Precision=0.8762364294330519 Recall=0.8727272727272727 F-Score=0.874478330658106
No of nodes in tree = 33223

Applying Reduced Error Pruning on the decision tree generated
Training Time=10.7secs
Accuracy=0.8404889134574043
Precision=0.9467631684760756 Recall=0.8588415523781733 F-Score=0.9006617450177867
No of nodes in tree = 2640

Initializing Random Forest with 10 trees, 0.5 fraction of attributes and 0.33 fraction of training instances in each tree
Training Time=1.618secs
Accuracy=0.8313371414532277
Precision=0.944270205066345 Recall=0.8511779630300834 F-Score=0.8953107129241327

End...
```
