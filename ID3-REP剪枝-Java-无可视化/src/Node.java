import java.util.ArrayList;
import java.util.HashMap;

public class Node {
	int attribute;//attribute on which classification is done
	ArrayList<Integer> remainingAttributes;
	int noOfClass1, noOfClass2;//class1 = <=50K, class2 = >50K
	double entropy;
	Node[] children;
	ArrayList<String[]> data;//Datasets satisfying conditions of current node and all ancestors
	boolean isLeaf = false;
	int classification;//applicable if leaf, classification = 1 or 2 for class1 and class2 respectively
	
	public Node(int attr, String value, int c1, int c2, double d, ArrayList<String[]> data, ArrayList<Integer> remainingAttributes){
		attribute = attr;
		noOfClass1 = c1;
		noOfClass2 = c2;
		entropy = d;
		this.data = data;
		this.remainingAttributes = remainingAttributes;
	}
	
	public Node(int classification){
		isLeaf = true;
		this.classification = classification;
	}
	
	public Node(){
		
	}
	
	public String toString(){
			return isLeaf ? "Leaf,Classification="+classification : "Entropy="+entropy+",Split="+attribute;
	}
	
	public static int predictClass(Node root, String[] data, HashMap<Integer, ArrayList<String>> discreteValues){
		if(root==null)	return 1;
		else if(root.isLeaf)	return root.classification;
		String s = data[root.attribute];
		ArrayList<String> discrVals = discreteValues.get(root.attribute);
		for(int i = 0; i < discrVals.size(); i++){
			if(s.equals(discrVals.get(i))){
				return predictClass(root.children[i], data, discreteValues);
			}
		}
		return 1;
	}
	
	public static void inOrder(Node root){
		if(root==null)	return;
		System.out.println(root);
		if(root.isLeaf)	return;
		for(Node c : root.children)	inOrder(c);
	}
}
