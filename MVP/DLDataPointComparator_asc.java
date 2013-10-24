import java.util.Comparator;

/*
 * This comparator class will be used to keep the n nearest neighbors sorted in order of their
 * distance from the query
 */

public class DLDataPointComparator_asc implements Comparator<DLDataPoint>{
	/*
	 * Comparator method for keeping n nearest neighbors sorted
	 */
/*
	public int compare(DataPoint arg0, DataPoint arg1) {
		DataPoint query = VPComputation.getQuery();
		double distance1 = VPComputation.computeEuclideanDistance(query, arg0);
		double distance2 = VPComputation.computeEuclideanDistance(query, arg1);
		if(distance1 == distance2)
			return 0;
		else if(distance1 < distance2)
			return 1;
		else return -1;
	}
	*/
	
	public int compare(DLDataPoint arg0, DLDataPoint arg1) {
		DLDataPoint vp = MVPComputation.currentVP;
		double distance1 = vp.computeDistance(arg0);
		double distance2 = vp.computeDistance(arg1);
		if(distance1 == distance2)
			return 0;
		else if(distance1 < distance2)
			return -1;
		else return 1;
	}

}
