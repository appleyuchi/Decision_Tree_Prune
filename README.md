This repository is collecting popular pruning methods(Continuous updating)
--------

Environment

Ubuntu Linux 16.04

Python 2.7.12

---------------------------------------------------------

For REP(Reduced Error Pruning) of ID3:

    cd ID3-REP-post_prune-Python-draw
    python jianzhi.py
Attention:

    continuous feature and feature with unKnown value are both Not supported to deal currently.

For EBP(Error Based Pruning) of C4.5-Release8:

    you will need two files:
    for example:crx.names,crx.data.
    and put them under the path: 
    Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src/quinlan-src/

    cd Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src
    python shell_execute.py crx > result.txt(change "crx" here when you use other datasets please.)
    python result_get.py
    python predict.py

Attention:

    data who contains unknown value is Not supported to predict.
    It's  an Python interface from C-type code http://www.rulequest.com/Personal/c4.5r8.tar.gz
