Environment
Ubuntu Linux 16.04
Python 2.7.12


For REP(Reduced Error Pruning) of ID3:
    cd ID3-REP-post_prune-Python-draw
    python jianzhi.py


For EBP(Error Based Pruning) of C4.5-Release8:
    cd Quinlan-C4.5-Release8_and_python_interface_for_EBP/Src
    change "crx" first when you use other datasets please.
    python shell_execute.py crx > result.txt
    python result_get.py
    python predict.py
