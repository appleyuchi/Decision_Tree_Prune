import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.StringTokenizer;

//Discretize into 2 classes: <=val and >val such that resulting entropy is minimum
public class Preprocess {
	int c1, c2;
	static int[] partitionAt;//the value at which continuous attributes are partitioned
	String class1, class2;
	ArrayList<Integer> continuousAttributes;
	/*no of distinct values for a attribute type
	*	e.g.: 0, [<=19,>19]
	*		  1, [Private, Self-emp-not-inc, Self-emp-inc, Federal-gov, Local-gov, State-gov, Without-pay, Never-worked]
	*/
	static HashMap<Integer, ArrayList<String>> discreteValues = new HashMap<>();
	
	public Preprocess(String class1, String class2, ArrayList<Integer> continuousAttributes){
		this.class1 = class1;
		this.class2 = class2;
		this.continuousAttributes = continuousAttributes;
	}
	
	public ArrayList<String[]> discretize(File trainOriginal) throws IOException{
		BufferedReader br = new BufferedReader(new FileReader(trainOriginal));
		
		String s = br.readLine();
		br.close();
		StringTokenizer st = new StringTokenizer(s, ",");
		int noOfAttributes = st.countTokens();
		partitionAt = new int[noOfAttributes];
		ArrayList<String[]> data = new ArrayList<>();
		br = new BufferedReader(new FileReader(trainOriginal));
		while((s = br.readLine())!=null){
			st = new StringTokenizer(s, ",");
			String[] dataset = new String[noOfAttributes]; 
			for(int i = 0; i < noOfAttributes; i++){
				dataset[i] = st.nextToken();
			}
			if(dataset[noOfAttributes-1].equals(class1))	c1++;
			else 											c2++;
			data.add(dataset);
		}
		for(int i : continuousAttributes){
			int c1LeftPart = 0, c1RightPart=c1;
			int c2LeftPart = 0, c2RightPart=c2;
			int total = c1+c2;
			Collections.sort(data, new Comp(i));
			double minEndtropy = Double.MAX_VALUE;
			int partAt=1;
			String prev = data.get(0)[i];
			for(String[] dataset: data){
				if(dataset[noOfAttributes-1].equals(class1)){
					c1LeftPart++;
					c1RightPart--;
				}else{
					c2LeftPart++;
					c2RightPart--;
				}
				String curr = dataset[i];
				if(curr.equals(prev) || curr.equals("?"))	continue;
				double p11 = (c1LeftPart+0.0)/(c1LeftPart+c2LeftPart), p12 = (c2LeftPart+0.0)/(c1LeftPart+c2LeftPart);
				double p21 = (c1RightPart+0.0)/(c1RightPart+c2RightPart), p22 = (c2RightPart+0.0)/(c1RightPart+c2RightPart);	
				double entropy = ((c1LeftPart+c2LeftPart)/total+0.0)*(-1*p11*Math.log(p11) -1*p12*Math.log(p12)) + ((c1RightPart+c2RightPart)/total+0.0)*(-1*p21*Math.log(p21) -1*p22*Math.log(p22));
				if(entropy < minEndtropy){
					minEndtropy = entropy;
					partAt = (Integer.parseInt(prev)+Integer.parseInt(curr))/2;
					partitionAt[i] = partAt;
				}
				prev = curr;
			}
			String newc1Name = "<="+partAt, newc2Name = ">"+partAt;
			for(String[] dataset : data){
				if(dataset[i]=="?")	continue;
				if(Integer.parseInt(dataset[i]) <= partAt)	dataset[i] = newc1Name;
				else										dataset[i] = newc2Name;
			}
		}
		br.close();
		return data;
	}
	
	/*
	 * data should not be continuous
	 * Sets ? to the value that occurs max times for that attribute
	 */
	public static void predictMissingValues(ArrayList<String[]> data, String trainOrTest) throws IOException{
		int n = data.get(0).length-1;
		String[] predictedValue = new String[n];
		for(int i = 0; i < n; i++){
			HashMap<String,Integer> count = new HashMap<>();
			for(String[] s : data){
				if(s[i].equals("?"))	continue;
				if(count.containsKey(s[i]))	count.put(s[i], count.get(s[i])+1);
				else						count.put(s[i], 1);
			}
			int max = 0;
			for(String s : count.keySet()){
				int c = count.get(s);
				if(c > max){
					max = c;
					predictedValue[i] = s;
				}
			}
		}
		
		for(String[] s : data){
			for(int i = 0; i < n; i++){
				if(s[i].equals("?"))	s[i] = predictedValue[i];
			}
		}
		
		File trainDiscretized = new File("../Data/"+trainOrTest+"_data_preprocessed.data");
		BufferedWriter bw = new BufferedWriter(new FileWriter(trainDiscretized));
		for(String[] dataset : data){
			bw.write(Arrays.toString(dataset).replace("[", "").replace("]", "").replace(" ", "")+"\n");
		}
		bw.close();	
	}
	
	//use partitionAt calculated when discretizing train data
	public static ArrayList<String[]> preprocessTestData(File test, ArrayList<Integer> continuousAttributes) throws IOException{
		BufferedReader br = new BufferedReader(new FileReader(test));
		
		String s = br.readLine();
		br.close();
		StringTokenizer st = new StringTokenizer(s, ",");
		int noOfAttributes = st.countTokens();
		ArrayList<String[]> data = new ArrayList<>();
		br = new BufferedReader(new FileReader(test));
		while((s = br.readLine())!=null){
			st = new StringTokenizer(s, ",");
			String[] dataset = new String[noOfAttributes]; 
			for(int i = 0; i < noOfAttributes; i++){
				dataset[i] = st.nextToken();
			}
			data.add(dataset);
		}
		for(int i : continuousAttributes){
			String newc1Name = "<="+partitionAt[i], newc2Name = ">"+partitionAt[i];
			for(String[] dataset : data){
				if(dataset[i]=="?")	continue;
				if(Integer.parseInt(dataset[i]) <= partitionAt[i])	dataset[i] = newc1Name;
				else												dataset[i] = newc2Name;
			}
		}
		br.close();
		predictMissingValues(data,"test");
		return data;
	}
	
	public static void computeDiscreteValues(ArrayList<String[]> data){
		HashSet<String> observedAttributes = new HashSet<>();
		String[] sampleDataset = data.get(0);
		for(int i = 0; i < sampleDataset.length-1; i++){
			discreteValues.put(i, new ArrayList<String>());
		}
		for(String[] dataset : data){
			for(int i = 0; i < dataset.length-1; i++){//last attribute is Y/N
				String attribute = dataset[i];
				if(attribute.equals("?"))	continue;
				if(!observedAttributes.contains(attribute)){
					discreteValues.get(i).add(attribute);
					observedAttributes.add(attribute);
				}
			}
		}
	}
}

class Comp implements Comparator<String[]>{
	int index;
	public Comp(int i){index = i;}
	public int compare(String[] s1, String[] s2){
		return s1[index].compareTo(s2[index]);
	}
}