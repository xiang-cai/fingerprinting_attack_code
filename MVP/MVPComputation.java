import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.*;
import java.text.*;

/*
 * This class has all of the computation, for the population of the universe, the construction of
 * the VP-tree and the search method.
 */

public class MVPComputation{

	//The universe: ArrayList of all DataPoints.
	private  ArrayList<DLDataPoint> universe;
	private  ArrayList<DLDataPoint> universe_clone;
	
	//The global search query. Use this when looking for n nearest neighbors!!!!
	public static DLDataPoint gsquery;
	public static double[] PATH;
	//global currentVP, used in comparator_asc
	public static DLDataPoint currentVP;
	//websites and trials
	private  int websites;
	private  int trials;
	
	//sample size
	private static final int samplesize = 50;
	//dplistsize in a node
	private static final int dplistsize = 1000;
	//path size in a datapoint
	private static final int pathsize = 20;
	//nodetype
	private static final int internal_node = 0;
	private static final int leaf_node = 1;
	
	//The global list of nearest neighbors
	private static PriorityQueue<DLDataPoint> neighbors;
	
	

	//constructor
	public MVPComputation(int web, int trial){
		this.websites = web;
		this.trials = trial;
		PATH = new double[pathsize];
	}

	/*
	 * Populates the universe with data and returns it.
	 */
	public  ArrayList<DLDataPoint> populateUniverse(){
		//Read in web page traces from text file or whatever method appropriate.
		//Construct DataPoints for all of the data and populate variable universe with them.
		universe = new ArrayList<DLDataPoint>();
		universe_clone = new ArrayList<DLDataPoint>();
		
		boolean rm_ack = true;
		int type = 1; //use increment_of_600
		String fname = null;
		String folder = "/tmp/dislog_1000_100/";
		int i,j;
		
		for(i = 1; i <= websites; i++){
			for(j = 1; j <= trials; j++){
				fname = String.format("%s%d_%d.txt", folder,i,j);
				DLDataPoint tmpDP = new DLDataPoint(i,j,pathsize);				
				try {
					tmpDP.Parse_data(fname, rm_ack, type);
				} catch (FileNotFoundException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				
				universe.add(tmpDP);
			}
		}
		
		for(i = 0; i < universe.size(); i++)
			universe_clone.add(universe.get(i));
		return universe;
	}
	
	/*
	 * Given ArrayList S of euclidean space elements, returns pointer
	 * to the root of an optimized vp-tree
	 */
	public  MVPNode makeTree(ArrayList<DLDataPoint> s, int level){
		//level should be 0 when makeTree is called for the first time
		//base case
		if(s.size() == 0)
			return null;
		MVPNode root = null;
		double maxdist;
		double dist;
		
		if(s.size() <= dplistsize+2){
			root = new MVPNode(leaf_node,dplistsize);
			root.sv1 = selectVP(s);	
			if(level < pathsize)
				root.sv1.path[level] = 0;
			s.remove(root.sv1);
			
			//choose sv2
			maxdist = -1;
			root.sv2 = null;
			for(int i = 0; i < s.size(); i++){
				dist = s.get(i).computeDistance(root.sv1);
				if(dist > maxdist){
					root.sv2 = s.get(i);
					maxdist = dist;
				}
			}

			if(root.sv2 != null && (level+1 < pathsize)){
				root.sv2.path[level+1] = root.sv2.computeDistance(root.sv1);
				s.remove(root.sv2);
			}
			// assign leaves
			for(int i = 0; i < s.size(); i++){
				root.DPlist[i] = s.get(i);
				root.D1[i] = s.get(i).computeDistance(root.sv1);
				if(level < pathsize)
					root.DPlist[i].path[level] = root.D1[i];
			}
			if(null != root.sv2){
				for(int i = 0; i < s.size(); i++){
					root.D2[i] = s.get(i).computeDistance(root.sv2);
					if(level+1 < pathsize)
						root.DPlist[i].path[level+1] = root.D2[i];
				}
			}
			
			return root;
		}
		
		else{
			// create root as an internal node, and recursively call makeTree
			root = new MVPNode(internal_node, dplistsize);
			root.sv1 = selectVP(s);
			if(level < pathsize)
				root.sv1.path[level] = 0;
			s.remove(root.sv1);
			for(int i = 0; i < s.size(); i++){
				dist = s.get(i).computeDistance(root.sv1);
				if(level < pathsize)
					s.get(i).path[level] = dist;
			}
			currentVP = root.sv1;
			DLDataPointComparator_asc comparator_asc = new DLDataPointComparator_asc();
			Collections.sort(s,comparator_asc);
			
			//find the median
			int mindex = s.size()/2;
			root.M1 = s.get(mindex).computeDistance(currentVP);
			ArrayList<DLDataPoint> left = new ArrayList<DLDataPoint>();
			ArrayList<DLDataPoint> right = new ArrayList<DLDataPoint>();
			for(int i = 0; i < s.size(); i++){
				if(i <= mindex)
					left.add(s.get(i));
				else
					right.add(s.get(i));
			}
			
			//now choose sv2
			root.sv2 = selectVP(right);
			right.remove(root.sv2);
			s.remove(root.sv2);		
			if(level+1 < pathsize)
				root.sv2.path[level+1] = root.sv2.computeDistance(root.sv1);
		
			// set path[] to sv2
			for(int i = 0; i < s.size(); i++){
				dist = s.get(i).computeDistance(root.sv2);
				if(level+1 < pathsize)
					s.get(i).path[level+1] = dist;
			}
			s.clear();
			
			//sort left and right
			currentVP = root.sv2;
			Collections.sort(left,comparator_asc);
			Collections.sort(right,comparator_asc);
			
			int lmindex = left.size()/2;
			int rmindex = right.size()/2;
			root.M2[0] = left.get(lmindex).computeDistance(currentVP);
			root.M2[1] = right.get(rmindex).computeDistance(currentVP);
		
			//split left and right both into 2 lists
			ArrayList<DLDataPoint> left2 = new ArrayList<DLDataPoint>();
			ArrayList<DLDataPoint> right2 = new ArrayList<DLDataPoint>();
		
			for(int i = lmindex+1; i < left.size(); i++)
				left2.add(left.get(i));
			for(int i = rmindex+1; i < right.size(); i++)
				right2.add(right.get(i));
			
			//chop left and right
			while(left.size() > lmindex+1)
				left.remove(left.size()-1);
			while(right.size() > rmindex+1)
				right.remove(right.size()-1);
			
			root.left[0] = makeTree(left, level+2);
			root.left[1] = makeTree(left2, level+2);
			root.right[0] = makeTree(right, level+2);
			root.right[1] = makeTree(right2, level+2);
			
			return root;
		}
	}
	
	/*
	 * Returns the best vantage point (a DataPoint object) for
	 * the current node
	 */
	public  DLDataPoint selectVP(ArrayList<DLDataPoint> s){
		
		//Empty set
		if(s.size() == 0)
			return null;

		//Set of size 1 or 2 has DataPoint which is itself a datapoint
		if(s.size() < 3)
			return s.get(0);
		
		//This will keep track of the maximum variance
		double maxVariance = 0;
		
		//This will be useful in the construction method, to set the mu of the current node(s).
		double currentMu;
		
		//Stores the current computed value of the variance
		double currentVariance;
		
		//Stores the best vantage point seen so far
		DLDataPoint best = null;
		
		double dist;

		
		int candidate_size,neighbor_size; 
		ArrayList<DLDataPoint> Candidateset = new ArrayList<DLDataPoint>();
		ArrayList<Double> distanceSet = new ArrayList<Double>();
//		ArrayList<DLDataPoint> tmp = new ArrayList<DLDataPoint>();

//		for(int i = 0; i < s.size(); i++)
//			tmp.add(s.get(i));

		// shuffle s, then choose first samplesize elements as Candidate set
		Collections.shuffle(s);
		neighbor_size = candidate_size = s.size() > samplesize ? samplesize : s.size();
		for(int i = 0; i < candidate_size; i++)
			Candidateset.add(s.get(i));
		
		// calculate mu and var for each candidate
		for (int i = 0; i < candidate_size; i++) {
			distanceSet.clear();
			DLDataPoint candidateVP = Candidateset.get(i);
			
			// for each candidate, choose a random subset from s as the neighborset
			Collections.shuffle(s);
			for (int j = 0; j < neighbor_size; j++) {
				DLDataPoint queryVP = s.get(j);
				dist = queryVP.computeDistance(candidateVP);
				distanceSet.add(dist);
			}

			// Sets properties of current node - useful in construction method
			currentMu = computeMean(distanceSet);
			currentVariance = computeVariance(distanceSet, currentMu);
			// update vantage point information. Added '=' for the case where
			// all the distances are
			// the same and hence variance will be 0. Need to return a valid
			// best.
			if (currentVariance >= maxVariance) {
				maxVariance = currentVariance;
				best = candidateVP;
			}
		}
		
		/*
		for(int i = 0; i < s.size(); i++){
			DataPoint candidateVP = s.get(i);
			
			//stores the distances encountered for each comparison
			ArrayList<Double> distanceSet = new ArrayList<Double>();
			
			for(int j = 0; j < s.size(); j++){
				
				//no need to compare a DataPoint with itself
				if(j != i){
					DataPoint queryVP = s.get(j);
					dist = queryVP.computeDistance(candidateVP);
					distanceSet.add(dist);
				}
			}
			
			//Sets properties of current node - useful in construction method
			currentMu = computeMean(distanceSet);
			currentVariance = computeVariance(distanceSet, currentMu);
			
			//update vantage point information. Added '=' for the case where all the distances are
			//the same and hence variance will be 0. Need to return a valid best.
			if(currentVariance >= maxVariance){
				maxVariance = currentVariance;
				setCurrentMu(currentMu);
				best = candidateVP;
			}
		}
		*/	
		return best;
	}
	
	/*
	 * Returns the mean of the elements in the ArrayList passed
	 */
	public  double computeMean(ArrayList<Double> distanceSet){
		double sum = 0;
		int size = distanceSet.size();
		ArrayList<Double> s = new ArrayList<Double>();
		s.addAll(distanceSet);
		while(!s.isEmpty()){
			double element = s.get(0);
			sum += element;
			s.remove(element);
		}
		return sum/size;
		
	}

	/*
	 * Returns the variance of the values in the ArrayList passed. Need the mean for computation.
	 */
	public  double computeVariance(ArrayList<Double> distanceSet, double currentMu2){
		double sum = 0;
		int size = distanceSet.size();
		TreeSet<Double> s = new TreeSet<Double>();
		s.addAll(distanceSet);
		while(!s.isEmpty()){
			double element = s.first();
			double difference = element - currentMu2;
			sum += difference*difference;
			s.remove(element);
		}
		return sum/size;
	}
	

	/*
	 * Searches n nearest neighbors and returns a list of n datapoints closest to the query
	 * provided.
	 */
	public static PriorityQueue<DLDataPoint> searchNNearestNeighbors(int n, DLDataPoint query,
			MVPNode root, double tau, int level){
		
		if(n == 0 || query == null)
			return null;
		
		//Return the current value of best because we're done searching
		if(root==null)
			return neighbors;
		
		double d_sv1, d_sv2, dist;
		DLDataPoint furthest;
		
		d_sv1 = query.computeDistance(root.sv1);
		d_sv2 = query.computeDistance(root.sv2);
		
		if (d_sv1 < tau) {
			// System.out.println("dis: "+ distance + " tau: "+ tau);
			neighbors.offer(root.sv1);
			// We're only interested in the n nearest elements. The comparator
			// is set up so the
			// furthest element is the head of the queue.
			if (neighbors.size() > n) {
				neighbors.remove();
				// Once we've got n elements we can select better points. Update
				// tau to be the
				// distance to the current farthest element in the list of
				// nearest neighbors.
				furthest = neighbors.peek();
				tau = furthest.computeDistance(query);
			}
		}		
		if (d_sv2 < tau) {
			// System.out.println("dis: "+ distance + " tau: "+ tau);
			neighbors.offer(root.sv2);
			// We're only interested in the n nearest elements. The comparator
			// is set up so the
			// furthest element is the head of the queue.
			if (neighbors.size() > n) {
				neighbors.remove();
				// Once we've got n elements we can select better points. Update
				// tau to be the
				// distance to the current farthest element in the list of
				// nearest neighbors.
				furthest = neighbors.peek();
				tau = furthest.computeDistance(query);
			}
		}
		if(level < pathsize)
			PATH[level] = d_sv1;
		if(level+1 < pathsize)
			PATH[level+1] = d_sv2;
		
		if(root.nodetype == leaf_node){
			// filter out faraway leaves
			for(int i = 0; i < dplistsize; i++){
				if(root.DPlist[i] == null)
					break;
				if((d_sv1-tau <= root.D1[i] && root.D1[i] <= d_sv1+tau)
						&& (d_sv2-tau <= root.D2[i] && root.D2[i] <= d_sv2+tau)){
					//take this leaf node i, check node_i.path[]
					boolean skip = false;
					for(int j = 0; j < pathsize; j++){
						if(root.DPlist[i].path[j] == -1)
							break;
						if((root.DPlist[i].path[j] >=  PATH[j]-tau)
								&& (root.DPlist[i].path[j] <= PATH[j]+tau))
							continue;
						else{
							skip = true;
							break;
						}	
					}
					// end of for loop
					if(!skip){
						dist = query.computeDistance(root.DPlist[i]);
						if (dist < tau) {
							neighbors.offer(root.DPlist[i]);
							if (neighbors.size() > n) {
								neighbors.remove();
								furthest = neighbors.peek();
								tau = furthest.computeDistance(query);
							}
						}
					}
				}
			}//finished filtering all DPs in this leaf node
		}
		/*
		else if(root.nodetype == internal_node){
			if(d_sv1 + tau <= root.M1){
				if(d_sv2+tau <= root.M2[0])
					searchNNearestNeighbors(n, query, root.left[0], tau, level+2);
				if(d_sv2-tau >= root.M2[0])
					searchNNearestNeighbors(n, query, root.left[1], tau, level+2);
			}
			if(d_sv1 - tau >= root.M1){
				if(d_sv2+tau <= root.M2[1])
					searchNNearestNeighbors(n, query, root.right[0], tau, level+2);
				if(d_sv2-tau >= root.M2[1])
					searchNNearestNeighbors(n, query, root.right[1], tau, level+2);
			}
		}
		*/
	
		else if(root.nodetype == internal_node){
			if(d_sv1 < root.M1){
				//search left[0] or left[1]
				if(d_sv2 < root.M2[0]){
					searchNNearestNeighbors(n, query, root.left[0], tau, level+2);
					if(d_sv2 >= root.M2[0] - tau)
						searchNNearestNeighbors(n, query, root.left[1], tau, level+2);
				}
				else{
					searchNNearestNeighbors(n, query, root.left[1], tau, level+2);
					if(d_sv2 < root.M2[0] + tau)
						searchNNearestNeighbors(n, query, root.left[0], tau, level+2);
				}
				//end searching left part

				if(d_sv1 >= root.M1 - tau){
					//search right[0] or right[1]
					if(d_sv2 < root.M2[1]){
						searchNNearestNeighbors(n, query, root.right[0], tau, level+2);
						if(d_sv2 >= root.M2[1] - tau)
							searchNNearestNeighbors(n, query, root.right[1], tau, level+2);
					}
					else{
							searchNNearestNeighbors(n, query, root.right[1], tau, level+2);
							if(d_sv2 < root.M2[1] + tau)
								searchNNearestNeighbors(n, query, root.right[0], tau, level+2);
					}
					//end searching right part
				}
			}
			else{
				//search right[0] or right[1]
				if(d_sv2 < root.M2[1]){
					searchNNearestNeighbors(n, query, root.right[0], tau, level+2);
					if(d_sv2 >= root.M2[1] - tau)
						searchNNearestNeighbors(n, query, root.right[1], tau, level+2);
				}
				else{
					searchNNearestNeighbors(n, query, root.right[1], tau, level+2);
					if(d_sv2 < root.M2[1] + tau)
						searchNNearestNeighbors(n, query, root.right[0], tau, level+2);
				}
				//end searching right part

				if(d_sv1 < root.M1 + tau){
					//search left[0] or left[1]
					if(d_sv2 < root.M2[0]){
						searchNNearestNeighbors(n, query, root.left[0], tau, level+2);
						if(d_sv2 >= root.M2[0] - tau)
							searchNNearestNeighbors(n, query, root.left[1], tau, level+2);
					}
					else{
						searchNNearestNeighbors(n, query, root.left[1], tau, level+2);
						if(d_sv2 < root.M2[0] + tau)
							searchNNearestNeighbors(n, query, root.left[0], tau, level+2);
					}
					//end searching left part
				}
			}
		}

		else{
                    System.out.println("shouldn't reach here!");
                    return null;
                }
		return neighbors;	
	}
	
	
	/**
	 * Define main method
	 */
	public static void main(String[] args) {
	
		DateFormat dateFormat = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
		Date start = new Date();
		System.out.println("start time: "+dateFormat.format(start));

		//root of the VP tree. Make sure the populateUniverse() method is defined.
		MVPComputation MVP = new MVPComputation(800,50);
		MVP.populateUniverse();
		
		Date pop = new Date();
		System.out.println("finished populating: "+dateFormat.format(pop));
		System.out.println("universe size: "+ MVP.universe.size());

		MVPNode root = MVP.makeTree(MVP.universe, 0);

		Date x = new Date();
		System.out.println("finished makeTree "+dateFormat.format(x));
		DLDataPoint.printcount();

//		DLDataPoint Query;
		//Create a query DataPoint and pass it to the nearestNeighbor() method. Also pass root and
		//the tau of your choosing. The search method will return a DataPoint which is the nearest
		//neighbor.

		int correct = 0;
		int total = MVP.universe_clone.size();
	
		//Use the global variable DataPoint gsquery defined in this class as the query since the 
		//DataPointComparator uses it to compute the relative distance of two DataPoints with 
		//respect to the query.
		
		//The comparator for our queue
		Comparator<DLDataPoint> comparator_desc = new DLDataPointComparator_desc();
		int n = 2;
		neighbors = new PriorityQueue<DLDataPoint>(n, comparator_desc);
		ArrayList<DLDataPoint> nearest = new ArrayList<DLDataPoint>();
		//The last argument here is tau. Sometimes the queue won't have n values, this is because
		//it probably didn't find enough neighbors that were close enough. I set tau to be the 
		//furthest any point can be from another to get all the DataPoints I could get.
		
		for(int i = 0; i < total; i++){
			gsquery = MVP.universe_clone.get(i);

//			double tau = gsquery.gettau(n);
			double tau = gsquery.gettau(MVP.universe_clone,n);
//			double tau = Double.MAX_VALUE;

			neighbors.clear();
			for(int j = 0; j < pathsize; j++)
				PATH[j] = -1;
			searchNNearestNeighbors(n, gsquery, root, tau, 0);
			System.out.println("after NN "+i);
			DLDataPoint.printcount();
			Date xx = new Date();
			System.out.println("time: "+dateFormat.format(xx));


			//The nearest neighbors are in priority queue neighbors. The furthest element is the head
			//of the queue so you have to reverse if you need to display the DataPoints.
//			Object[] nearest = neighbors.toArray();
			nearest.clear();
			DLDataPoint head;

			while(null != (head = neighbors.poll()))
				nearest.add(head);

			for(int j = nearest.size()-1; j>=0; j--){
				DLDataPoint neighbor = nearest.get(j);
				if(neighbor == null)
					continue;
				if(neighbor == gsquery)
					continue;
				if(gsquery.website == neighbor.website && gsquery.trial != neighbor.trial){
					correct++;
//					System.out.println("query: "+ gsquery.website+" trial: "+ gsquery.trial);
//					System.out.println("neighbor: web: "+ neighbor.website+" trial: "+ neighbor.trial+" dis: "+gsquery.computeDistance(neighbor));
//					System.out.println("query: "+ gsquery.website+" trial: "+ gsquery.trial);
//					System.out.println("neighbor: "+ neighbor.website+" trial: "+ neighbor.trial);
//					System.out.println("nearest size: "+ nearest.size());
					break;
				}
				//Print statements
			}
		}

//		DLDataPoint.printMatrix(100);

		Date end = new Date();
		System.out.println("finished main "+dateFormat.format(end));
		System.out.println("correct: "+correct+ " total: "+ total);
		System.out.println("success rate "+ correct/1.0/total);
	}
}
