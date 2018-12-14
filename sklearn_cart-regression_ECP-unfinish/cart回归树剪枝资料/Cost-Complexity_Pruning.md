CCP:cost complexity pruning algorithm(for classification tree)
ECP:Error complexity pruning algorithm(for regression tree)


Cost-Complexity Function
need to optimize the cost-complexity function
$R_\alpha (T) = R(T) + \alpha \cdot | f(T) |$ where
$R(T)$ is the training/learning error
$f(T)$ a function that returns the set of leaves of tree $T$
$\alpha$ is a Regularization parameter
$R(T) = \sum_{t \in f(T)} r(t) \cdot p(t) = \sum_{t \in f(T)} R(t)$
$\sum_{t \in f(T)} R(t)$ - sum of misclassification errors at each leaf
$r(t) = 1 - \max_k p(C_k - t)$ - misclassification rate
$p(t) = \cfrac{n(t)}{n}$ with $n(t)$ being the # of records in node $t$ and $n$ - total # of records
Pruning Subtrees
Subtrees:

Pruning a subtree $T_{t}$

$R_\alpha(T - T_t) - R_\alpha(T)$ - variation of the cost-complexity function
this is the cont-complexity when pruning subtree $T_t$
$R_\alpha(T - T_t) - R_\alpha(T) = R(T - T_t) - R(T) + \alpha ( | f(T - T_t) | - |f(T)| ) = R(t) - R(T_t) + \alpha ( 1 - |f(T_t)| )$
let $\alpha' = \cfrac{R(t) - R(T_t)}{|f(T_t)| - 1}$
variation is
null if $\alpha = \alpha'$
negative if $\alpha < \alpha'$
positive if $\alpha > \alpha'$