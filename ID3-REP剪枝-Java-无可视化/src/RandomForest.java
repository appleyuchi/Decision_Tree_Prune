import java.util.ArrayList;
import java.util.HashMap;
import java.util.Random;

public class RandomForest {
	int noOfTrees;
	HashMap<Integer, ArrayList<String>> discreteValues;
	String class1, class2;
	ArrayList<String[]> data, testData;//data = training data
	double trainingTime=0, precision, recall, fscore, accuracy;
	ID3[] tree;
	
	public RandomForest(int noOfTrees, double fractionOfAttributesToTake, double fractionOfInstancesToTake, ArrayList<String[]> data, ArrayList<String[]> testData, int noOfClass1, int noOfClass2, 
			String class1, String class2, HashMap<Integer, ArrayList<String>> discreteValues){
		if(fractionOfAttributesToTake>1 || fractionOfAttributesToTake<0 || fractionOfInstancesToTake>1 || fractionOfInstancesToTake<0){
			System.out.println("Random Forest input invalid");
			return;
		}
			
		this.noOfTrees = noOfTrees;
		this.data = data;
		this.class1 = class1;
		this.class2 = class2;
		this.testData = testData;
		this.discreteValues = discreteValues;
		
		int noOfAttributes = data.get(0).length-1, noOfTrainingInstances = data.size();
		int noOfRandomInstances = (int)(noOfTrainingInstances*fractionOfInstancesToTake);
		int noOfRandomAttributes = (int)(noOfAttributes*fractionOfAttributesToTake);
		
		String[][] dataInArray = new String[noOfTrainingInstances][];//to access randomly in constant time
		int x = 0;
		for(String[] s : data){
			dataInArray[x++] = s;
		}
		
		trainingTime = System.currentTimeMillis();
		
		Random rand = new Random();
		tree = new ID3[noOfTrees];
		for(int i = 0; i < noOfTrees; i++){
			ArrayList<Integer> remAttr = new ArrayList<>();
			for(int j = 0; j < noOfRandomAttributes; j++){
				int r = rand.nextInt(noOfAttributes);
				if(remAttr.contains(r))	j--;
				else remAttr.add(r);
			}
			ArrayList<String[]> randData = new ArrayList<>();
			for(int j = 0; j < noOfRandomInstances; j++){
				randData.add(dataInArray[rand.nextInt(noOfTrainingInstances)]);
			}
			tree[i] = new ID3(randData, testData, noOfClass1, noOfClass2, class1, class2, discreteValues, remAttr);
		}
		
		trainingTime = (System.currentTimeMillis() - trainingTime)/1000.0;
		
		analyse();
	}
	
	public void analyse(){
		if(tree==null)	return;//Invalid inputs
		int correctClassification=0, incorrectClassification=0;
		int truePositive = 0, falsePositive = 0, falseNegative = 0;
		for(String[] s : testData){
			int tempPrediction1=0, tempPrediction2=0;
			for(int i = 0; i < noOfTrees; i++){
				if(Node.predictClass(tree[i].root, s, discreteValues)==1)	tempPrediction1++;
				else tempPrediction2++;
			}
			int predicted=tempPrediction1>=tempPrediction2?1:2, actual = s[s.length-1].equals(class1)?1:2;
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
	}
	
	public void printAnalysis(){
		System.out.println("Training Time="+trainingTime+"secs");
		System.out.println("Accuracy="+accuracy+"\nPrecision="+precision+" Recall="+recall+" F-Score="+fscore);
	}
}
