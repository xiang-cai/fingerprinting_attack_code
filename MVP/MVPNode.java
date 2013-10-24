//MVPNode class, it's either an internal node or a leaf node -- determined by nodetype
public class MVPNode{
	
	public int nodetype;	// nodetype = 0 or 1 (internal or leaf) 
	
	public DLDataPoint sv1;
	public DLDataPoint sv2;
	
	//fields for internal node
	public double M1;
	public double[] M2;	
	public MVPNode[] left;
	public MVPNode[] right; // M2, left and right are of size 2
	
	//fields for leaf node
	private int k; // size of DPlist in a leaf node
	public double[] D1;
	public double[] D2;
	public DLDataPoint[] DPlist; // D1, D2 and DPlist are of size k

	//constructor
	public MVPNode(int type, int dplistsize){
		this.k = dplistsize;
		this.nodetype = type;
		this.sv1 = this.sv2 = null;
		
		if(nodetype == 0){
			// internal node
			M2 = new double[2];
			left = new MVPNode[2];
			right = new MVPNode[2];
			for(int i = 0; i < 2; i++){
				M2[i] = -1;
				left[i]=right[i]=null;
			}
		}
		else if(nodetype == 1){
			D1 = new double[k];
			D2 = new double[k];
			DPlist = new DLDataPoint[k];
			for(int i = 0; i < k; i++){
				D1[i]=D2[i] = -1;
				DPlist[i]=null;
			}
		}
		else{
			System.out.println("Nodetype unknown!!");
			System.exit(1);
		}
	}
}