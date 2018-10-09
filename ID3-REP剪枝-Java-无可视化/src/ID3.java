import java.util.ArrayList;
import java.util.HashMap;

public class ID3 {
	HashMap<Integer, ArrayList<String>> discreteValues;
	String class1, class2;
	ArrayList<String[]> data, testData;//data = training data
	Node root;
	double trainingTime=0, precision, recall, fscore, accuracy;
	int noOfNodes;
	
	public ID3(ArrayList<String[]> data, ArrayList<String[]> testData, int noOfClass1, int noOfClass2, String class1, String class2, HashMap<Integer, ArrayList<String>> discreteValues, ArrayList<Integer> remAttr){
		trainingTime = System.currentTimeMillis();
		
		this.data = data;
		this.class1 = class1;
		this.class2 = class2;
		this.testData = testData;
		this.discreteValues = discreteValues;
		
		root = new Node();
		double p1 = noOfClass1/(noOfClass1+noOfClass2+0.0), p2 = noOfClass2/(noOfClass1+noOfClass2+0.0);
		root.entropy = -1*pLogP(p1) - 1*pLogP(p2);
		root.data = data;
		root.noOfClass1 = noOfClass1;
		root.noOfClass2 = noOfClass2;
		
		root.remainingAttributes = remAttr;
		generateDecisionTree(root);
		
		trainingTime = (System.currentTimeMillis() - trainingTime)/1000.0;
		
		analyse();//calculate precision, recall, fscore, accuracy
	}
	
	public void analyse(){
		int correctClassification=0, incorrectClassification=0;
		int truePositive = 0, falsePositive = 0, falseNegative = 0;
		for(String[] s : testData){
			int predicted = Node.predictClass(root, s, discreteValues), actual = s[s.length-1].equals(class1)?1:2;
			if(predicted  == actual )	correctClassification++;
			else 						incorrectClassification++;
			
			//1-->yes, 2-->no
			if(predicted==1 && actual==1)			truePositive++;
			else if(predicted==1 && actual==2)		falseNegative++;
			else if(predicted==2 && actual==1)		falsePositive++;
		}
		precision = truePositive/(truePositive+falsePositive+0.0);
		recall = truePositive/(truePositive+falseNegative+0.0);
		fscore = 2*precision*recall/(precision+recall);
		accuracy = (correctClassification)/(correctClassification+incorrectClassification+ 0.0);
		
		noOfNodes = 0;
		countNodes(root);
	}
	
	public void printAnalysis(){
		System.out.println("Accuracy="+accuracy+"\nPrecision="+precision+" Recall="+recall+" F-Score="+fscore);
		System.out.println("No of nodes in tree = "+noOfNodes);
	}
	
	public void printTrainingTime(){
		System.out.println("Training Time="+trainingTime+"secs");
	}
	
	public void countNodes(Node root){
		if(root==null)	return;
		noOfNodes++;
		if(root.isLeaf)	return;
		for(Node n : root.children)	countNodes(n);
	}
	
	private static double pLogP(double p){
		return p==0?0:p*Math.log(p);
	}
	
	private void generateDecisionTree(Node root){
		if(root==null)	return;
		if(root.remainingAttributes.size()==1){//leaf node
			root.isLeaf = true;
			root.classification = root.noOfClass1>=root.noOfClass2?1:2;
		}else if(root.noOfClass1==0 || root.noOfClass2==0 || root.data.size()==0){//leaf
			root.isLeaf = true;
			root.classification = root.noOfClass1==0?2:1;
		}else{
			//find split attribute which gives max gain
			root.children = splitAttribute(root);
			ArrayList<String> discreteValuesOfThisAttribute = discreteValues.get(root.attribute);
			for(int j=0; j < discreteValuesOfThisAttribute.size(); j++){
				root.children[j].data = new ArrayList<>();
				root.children[j].remainingAttributes = new ArrayList<>();
				for(int rem : root.remainingAttributes){
					if(rem!=root.attribute)	root.children[j].remainingAttributes.add(rem);
				}
				String curr = discreteValuesOfThisAttribute.get(j);
				for(String[] s : root.data){
					if(s[root.attribute].equals(curr)){
						root.children[j].data.add(s);
					}
				}
				generateDecisionTree(root.children[j]);
			}
		}
	}
	
	public Node[] splitAttribute(Node root){
		double maxGain = -1.0;
		Node[] ans = null;
		for(int i : root.remainingAttributes){
			ArrayList<String> discreteValuesOfThisAttribute = discreteValues.get(i);
			Node[] child = new Node[discreteValuesOfThisAttribute.size()];
			for(int j=0; j < discreteValuesOfThisAttribute.size(); j++){
				String curr = discreteValuesOfThisAttribute.get(j);
				child[j] = new Node();
				for(String[] s : root.data){
					if(s[i].equals(curr)){
						if(s[s.length-1].equals(class1))	child[j].noOfClass1++;
						else 								child[j].noOfClass2++;
					}
				}
			}
			int total = root.data.size();
			double gain = root.entropy;
			for(int j = 0; j < discreteValuesOfThisAttribute.size(); j++){
				int c1 = child[j].noOfClass1, c2 = child[j].noOfClass2;
				if(c1==0 && c2==0)	continue;
				double p1 = c1/(c1+c2+0.0), p2 = c2/(c1+c2+0.0);
				child[j].entropy = -1*pLogP(p1) + -1*pLogP(p2);
				gain -= ((c1+c2)/(total+0.0))*child[j].entropy;
			}
			if(gain > maxGain){
				root.attribute = i;
				maxGain = gain;
				ans = child;
			}
		}
		return ans;
	}
}
