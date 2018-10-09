
public class ReducedErrorPruning {
	ID3 tree;
	double trainingTime;
	double initialAccuracy, maxAccuracy;
	
	public ReducedErrorPruning(ID3 tree){
		this.tree = tree;
		initialAccuracy = tree.accuracy;
		maxAccuracy = initialAccuracy;
		trainingTime = System.currentTimeMillis();
		pruneTree(tree.root);
		trainingTime = (System.currentTimeMillis() - trainingTime)/1000.0;
		System.out.println("Training Time="+trainingTime+"secs");
	}
	
	private void pruneTree(Node root){//return true if more accuracy obtained 
		if(root==null)	return;
		root.isLeaf = true;
		root.classification = root.noOfClass1>=root.noOfClass2?1:2;
		tree.analyse();
		if(tree.accuracy > maxAccuracy){
			maxAccuracy = tree.accuracy;
			return;
		}
		root.isLeaf = false;
		for(Node c : root.children){
			if(c.isLeaf)	continue;
			pruneTree(c);
		}
		
	}
}
