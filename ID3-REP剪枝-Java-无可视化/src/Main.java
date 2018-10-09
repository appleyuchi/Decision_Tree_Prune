import java.io.File;
import java.io.IOException;
import java.util.ArrayList;

public class Main {

	public static void main(String[] args) throws IOException {
		System.out.println("Start...");
		
		System.out.println("\nPrepocessing Training data");
		String class1 = "<=50K", class2 = ">50K";
		ArrayList<Integer> continuousAttributes = new ArrayList<>();
		continuousAttributes.add(0);
		continuousAttributes.add(2);
		continuousAttributes.add(4);
		continuousAttributes.add(10);
		continuousAttributes.add(11);
		continuousAttributes.add(12);
		
		Preprocess p = new Preprocess(class1, class2, continuousAttributes);
		ArrayList<String[]> trainData = p.discretize(new File("../Data/train.data"));
		int noOfClass1 = p.c1, noOfClass2 = p.c2;
		Preprocess.predictMissingValues(trainData, "train");
		
		Preprocess.computeDiscreteValues(trainData);
		
		System.out.println("\nPrepocessing Testing data");
		ArrayList<String[]> testData = Preprocess.preprocessTestData(new File("../Data/test.data"), continuousAttributes);
		
		ArrayList<Integer> remAttr = new ArrayList<>();
		for(int i = 0; i < trainData.get(0).length-1; i++)	remAttr.add(i);
		System.out.println("\nGenerating Decision Tree using ID3 Algorithm");
		//<=50K = class1, >50K = class2
		ID3 decisionTree = new ID3(trainData, testData, noOfClass1, noOfClass2, class1, class2, Preprocess.discreteValues, remAttr);
		decisionTree.printTrainingTime();
		decisionTree.printAnalysis();
		
		System.out.println("\nApplying Reduced Error Pruning on the decision tree generated");
		//if you want to preserve original tree then create another ID3 instance and pass that to rep
		ReducedErrorPruning rep = new ReducedErrorPruning(decisionTree);
		rep.tree.printAnalysis();
		
		int noOftrees = 10;
		double fractionOfAttributesToTake = 0.5, fractionOfTrainingInstancesToTake = 0.33;
		System.out.println("\nInitializing Random Forest with "+noOftrees+" trees, "+fractionOfAttributesToTake
				+" fraction of attributes and "+fractionOfTrainingInstancesToTake+" fraction of training instances in each tree");
		RandomForest rf = new RandomForest(noOftrees, fractionOfAttributesToTake, fractionOfTrainingInstancesToTake, 
				trainData, testData, noOfClass1, noOfClass2, class1, class2, Preprocess.discreteValues);
		rf.printAnalysis();
		
		System.out.println("\nEnd...");
	}

}
