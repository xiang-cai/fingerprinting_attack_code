import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.File;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collections;


public class DLDataPoint{
	
	//Matrix to store pairwise distance -- reduce calculation time
/*
   private static double[][] Matrix;
	private static int websites = 1000;
	private static int trials = 100;
	private static int dim = websites*trials;
	private static ArrayList<Double> line;

	*/
	private static int count = 0; 
	private static ArrayList<DLDataPoint> tmp;
	private static ArrayList<Double> taus;
	private static final int samplesize = 50;

	static{
//		line = new ArrayList<Double>();
		tmp = new ArrayList<DLDataPoint>();
		taus = new ArrayList<Double>();
/*
		Matrix = new double[dim][dim];
		for(int i = 0; i < dim; i++){
			for(int j = 0; j < dim; j++){
				Matrix[i][j] = Double.NEGATIVE_INFINITY;
			}
		}
		for(int i = 0; i < dim; i++){
			Matrix[i][i] = 0;
		}
*/
		//Load Pre-calculated Matrix from a file
/*
		try {
			BufferedReader br = new BufferedReader(new FileReader("/tmp/matrix_converted"));
			String thisLine;
			int line = 0;
			while ((thisLine = br.readLine()) != null) { // while loop
				String[] a = thisLine.split(";");
				int x = trials * (Integer.valueOf(a[0]) - 1) + Integer.valueOf(a[1]) - 1;
				int y = trials * (Integer.valueOf(a[2]) - 1) + Integer.valueOf(a[3]) - 1;
				Matrix[x][y] = Double.valueOf(a[6]);
				line += 1;
				//    if (line % 100000 == 0) System.out.println("haha" + line);
			} // end while
		} // end try
		catch (IOException e) {
			System.err.println("Error: " + e);
		}
*/
		System.out.println("static initialization finished");
	
		
		/*
		try {
			PrintWriter pw = new PrintWriter(new File("./javamatrix.txt"));
			for (int i = 0; i < dim; i++) {
				for (int j = 0; j < dim; j++) {
					pw.print(Matrix[i][j] + " ");
				}
				pw.println();
			}
			pw.close();
		} // end try
		catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		*/
	}

	
	// these two variables together can be used as Node identifier
	public int website;
	public int trial;
	private int p; // number of distances to VPs it keeps
	public double[] path; // the array is used to keep the distances to VPs
	
	public ArrayList<Integer> sequence;

	//constructor
	public DLDataPoint(int website, int trial, int p){
		this.website = website;
		this.trial = trial;
		this.p = p;
		path = new double[this.p];
		for(int i = 0; i < this.p; i++)
			path[i] = -1;
		
		this.sequence = new ArrayList<Integer>();
		this.sequence.clear();
		this.sequence.add(0, 0); // add 0 at the beginning of the array, for DL calculation.
	}

/*	
	public double gettau(int n){
		if(n <= 0)
			return Double.MAX_VALUE;
		line.clear();
		for(int i = 0; i < dim; i++){
			if((i != (trials*(website-1)+trial-1)) && (false == Double.isInfinite(Matrix[trials*(website-1)+trial-1][i])))
				line.add(Matrix[trials*(website-1)+trial-1][i]);
		}
	
		if(line.size() == 0)
			return Double.MAX_VALUE;

		Collections.sort(line);
		if(line.size() <= n)
			return line.get(line.size()-1);
		return line.get(n-1);
	}
*/

	public double gettau(ArrayList<DLDataPoint> universe, int n){
		tmp.clear();
		taus.clear();
	
		if(n <= 0)
			return Double.MAX_VALUE;

//		double ret = gettau(n);

		for(int i = 0; i < universe.size(); i++)
			tmp.add(universe.get(i));

		// shuffle s, then choose first samplesize elements as Candidate set
		Collections.shuffle(tmp);
		
		int size = tmp.size() < samplesize ? tmp.size() : samplesize;
		for(int i = 0; i < size; i++){
			taus.add(computeDistance(tmp.get(i)));
		}

		Collections.sort(taus);

		if(taus.size() == 0)
			return Double.MAX_VALUE;
		if(taus.size() <= n)
			return taus.get(taus.size()-1);
		return taus.get(n-1);
	}

	public double minimum(double a, double b, double c){
		double min = a;
		if(b < min)
			min = b;
		if(c < min)
			min = c;
		return min;
	}
/*	
	// return the distance between (web_i1,trial_j1) and (web_i2, trial_j2)
	public double get_distance(DLDataPoint x, DLDataPoint y){
	
		int web_x = x.website;
		int trial_x = x.trial;
		int web_y = y.website;
		int trial_y = y.trial;
		return Matrix[trials*(web_x-1)+trial_x-1][trials*(web_y-1)+trial_y-1];
	}
	
	// set the distance between (web_i1,trial_j1) and (web_i2, trial_j2) -- symmetric
	public void set_distance(DLDataPoint x, DLDataPoint y, double dis){	
		
		int web_x = x.website;
		int trial_x = x.trial;
		int web_y = y.website;
		int trial_y = y.trial;
	
		Matrix[trials*(web_x-1)+trial_x-1][trials*(web_y-1)+trial_y-1] = Matrix[trials*(web_y-1)+trial_y-1][trials*(web_x-1)+trial_x-1] = dis;

		count += 2;
		if(count % 10000 == 0)
			System.out.println("count is: "+count);
	}
*/
	public void Parse_data(String fname, boolean rm_ack, int type)
			throws FileNotFoundException, IOException {
		/*
		 * type : 
		 * 1: use increment_of_600
		 * 2: use add_chunk_marker
		 * default: convert streams to +/-1
		 */

		String s;
		int i, pkt, round;
		int increment = 600;
		int unit = 512;

		BufferedReader br = new BufferedReader(new FileReader(fname));

		switch (type) {
		case 1:
			while ((s = br.readLine()) != null) {
				if (s.equals(""))
					continue;
				pkt = Integer.valueOf(s);
				if (Math.abs(pkt) > 1500)
					continue;
				if (rm_ack) {
					if (Math.abs(pkt) == 52 || Math.abs(pkt) == 40)
						continue;
				}
				pkt = pkt / Math.abs(pkt) * increment
						* (int) Math.ceil(Math.abs(pkt) / 1.0 / increment);
				sequence.add(pkt);
			}
			break;

		case 2:
			while ((s = br.readLine()) != null) {
				if (s.equals(""))
					continue;
				pkt = Integer.valueOf(s);
				if (Math.abs(pkt) > 1500)
					continue;
				if (rm_ack) {
					if (Math.abs(pkt) == 52 || Math.abs(pkt) == 40)
						continue;
				}
				round = (int) Math.ceil(Math.abs(pkt) / 1.0 / unit);
				pkt /= Math.abs(pkt);
				sequence.add(pkt > 0 ? 2 : -2);
				for (i = 1; i <= round; i++)
					sequence.add(pkt);
			}
			break;

		default:
			while ((s = br.readLine()) != null) {
				if (s.equals(""))
					continue;
				pkt = Integer.valueOf(s);
				if (Math.abs(pkt) > 1500)
					continue;
				if (rm_ack) {
					if (Math.abs(pkt) == 52 || Math.abs(pkt) == 40)
						continue;
				}
				sequence.add(pkt > 0 ? 1 : -1);
			}
			break;
		}
		
		br.close();
	}
/*
	public static void printMatrix(int dim){
		System.out.println("count is: "+count);
		if(dim > 2000)
			return;
		for(int i = 0; i < dim; i++){
			for(int j = 0; j < dim; j++)
				System.out.print(Matrix[i][j]+" , ");
			System.out.println();
		}
	}
*/

	public static void printcount(){
		System.out.println("count is: "+count);
	}


	public double computeDistance(DLDataPoint dp){
		if(dp == null)
			return Double.POSITIVE_INFINITY;

		DLDataPoint other = dp;
		
		double dist;
		/*
		getdist = get_distance(this, other);
		
		if(false == Double.isInfinite(getdist))
			return getdist;
		*/

		int i,j;
		double idcost = 2;
		double subcost;
		double transcost;
	
		int m = sequence.size();
		int n = other.sequence.size();

		double[][] dis = new double[m][n];

		for(i=0; i<m; i++)
			dis[i][0]=i*idcost;
		for(j=0; j<n; j++)
			dis[0][j]=j*idcost;

		for(i=1; i<m; i++){
			for(j=1; j<n; j++){
				if(sequence.get(i).equals(other.sequence.get(j)))
					subcost = transcost = 0;
				else{
					subcost = 2;
					transcost = 0.1;
				}
				dis[i][j] = minimum
				(
					dis[i-1][j] + idcost,  // a deletion
					dis[i][j-1] + idcost,  // an insertion
					dis[i-1][j-1] + subcost // a substitution
				);

				if(i>1 && j>1 && sequence.get(i).equals(other.sequence.get(j-1)) && sequence.get(i-1).equals(other.sequence.get(j)))
					dis[i][j] = dis[i][j] < dis[i-2][j-2]+transcost ? dis[i][j] : dis[i-2][j-2]+transcost;
			}
		}

		dist = dis[m-1][n-1];
		count++;
		if(count % 100000 == 0)
			printcount();
//		set_distance(this, other, dist);
/*
		if(dist != getdist){
			System.out.printf("inconsistant! %d,%d --> %d,%d  java: %f , c++ %f\n", website,trial,other.getwebsite(),other.gettrial(),dist,getdist);
			System.exit(1);
		}
		*/
		return  dist;
	}
}
