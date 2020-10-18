#include "costs.hh"

#include <iostream>
#include "math.h"

#include "vdata.hh"


/*
 * Global data needed for cost computations
 */
float weighted_cost_alpha = .5;
VData* verticalDataset = NULL;
float item_tol_ratio = 1.0; // 1.0 means accept all
float tran_tol_ratio = 1.0;
/*
 * Initialization of global data
 */
void costAuxSetup(float alpha, void* vert, float item_tolerance_ratio, float tran_tolerance_ratio) {
	weighted_cost_alpha = alpha;
	verticalDataset = (VData*)vert;
	item_tol_ratio = item_tolerance_ratio;
	tran_tol_ratio = tran_tolerance_ratio;
}


/*
 * Plain un-weighted Panda cost.
 */
float PANDA_delta_cost(int n_items, int n_trans, int noise, int already_cov) {
  return (float) (n_items+n_trans - n_items*n_trans + already_cov + 2*noise);
}
float PANDA_initial_cost(void) {
	return (float) verticalDataset->getSize();
}

/*
 * Weighted Panda cost.
 */
float PANDA_weighted_delta_cost(int n_items, int n_trans, int noise, int already_cov){
  return    weighted_cost_alpha *(float)(n_items+n_trans) +
         (1.0-weighted_cost_alpha)*(float)(already_cov + 2*noise- n_items*n_trans);
}
float PANDA_weighted_initial_cost(void){
	return (1.0-weighted_cost_alpha)*(float)verticalDataset->getSize();
}

/*
 * Weighted Panda L2 cost.
 */
float PANDA_weighted_L2_delta_cost(int n_items, int n_trans, int noise, int already_cov){
	float delta_P = sqrt( (float)n_items ) + sqrt( (float)n_trans );

	float old_N = 0.0;
	for (int i=0; i<verticalDataset->getItems(); i++) {
		old_N += verticalDataset->getFalsePositives(i);
		old_N += verticalDataset->getFalseNegatives(i);
	}
	float new_N = old_N - n_items*n_trans + already_cov + 2*noise;

	float delta_N = sqrt(new_N) - sqrt(old_N);

	/*
	std::cout << n_items << " - " << n_trans << " - " << noise << " - " << already_cov << endl;
	std::cout << delta_P << " - " << delta_N << endl;
	std::cout << new_N << " - " << old_N << endl;
	std::cout << weighted_cost_alpha*delta_P << " - " << (1.0-weighted_cost_alpha)*delta_N << endl;
	*/

	return weighted_cost_alpha*delta_P + (1.0-weighted_cost_alpha)*delta_N;
}

/*
 * MDL encoding size
 */
float MDL_encoding_size(float len, float ones){
	if (len<=0.0) return 0.0;
	if (len==ones || ones==0.0) return log2(len);
	float size = log2(len) - ones*log2(ones/len) - (len-ones)*log2((len-ones)/len);
	if (ones>len/2.0)
		size = 2.0*( log2(len) - len*log2(0.5) ) - size;
	return size;
}
/*
 * MDL typed Xor implementation.
 */
float MDL_TypedXOR_delta_cost(int n_items, int n_trans, int noise, int already_cov){
	// --------------------- COMPUTE L(E) -----------------------
	// compute old factors
	float old_BC_area = verticalDataset->getTotalCoveredArea();
	float old_fp      = verticalDataset->getTotalFalsePositives();
	float old_fn      = verticalDataset->getTotalFalseNegatives();

	float old_BC_complement = verticalDataset->getTrans()*verticalDataset->getItems() - old_BC_area;
	//float old_BC_complement = 1000*50 - old_BC_area;
	float old_fp_cost = MDL_encoding_size(old_BC_area, old_fp);
	float old_fn_cost = MDL_encoding_size(old_BC_complement, old_fn);
	float old_E_cost = old_fp_cost + old_fn_cost;

	// compute new factors
	float new_BC_area = old_BC_area + n_items*n_trans - already_cov;
	float new_fp = old_fp + noise;
	float new_fn = old_fn - n_items*n_trans + already_cov + noise;
	float new_BC_complement = verticalDataset->getTrans()*verticalDataset->getItems() - new_BC_area;
	//float new_BC_complement = 1000*50 - new_BC_area;
	float new_fp_cost = MDL_encoding_size(new_BC_area, new_fp);
	float new_fn_cost = MDL_encoding_size(new_BC_complement, new_fn);
	float new_E_cost = new_fp_cost + new_fn_cost;

	// --------------------- COMPUTE L(B) and L(C) -----------------------
	float delta_B = MDL_encoding_size(verticalDataset->getItems(), n_items);
	float delta_C = MDL_encoding_size(verticalDataset->getTrans(), n_trans);
	//float delta_B = MDL_encoding_size(50, n_items);
	//float delta_C = MDL_encoding_size(1000, n_trans);

	float delta_cost = delta_B + delta_C + new_E_cost - old_E_cost;

/*
	std::cout << old_BC_area << " - " << old_fp << " - " << old_fn << endl;
	std::cout << verticalDataset->getTotalCoveredArea() << " - " << verticalDataset->getTotalFalsePositives() << " - " << verticalDataset->getTotalFalseNegatives() << endl;
 	std::cout << old_BC_complement << " - " << old_fn << endl;
	std::cout << new_BC_area << " - " << new_fp << endl;
	std::cout << new_BC_complement << " - " << new_fn << endl;
	std::cout << "old noise cost " << old_E_cost << endl;
	std::cout << "new noise cost " << new_E_cost << endl;
	std::cout << "delta cost " << delta_cost << endl;
*/

	return delta_cost;
}
float MDL_TypedXOR_initial_cost(void){
	return MDL_NaiveXOR_initial_cost();
}

/*
 * MDL naive Xor implementation.
 */
float MDL_NaiveXOR_delta_cost(int n_items, int n_trans, int noise, int already_cov){
	float mn = verticalDataset->getTrans()*verticalDataset->getItems();
	//float mn = 1000*50;
	// --------------------- COMPUTE L(E) -----------------------
	// compute old factors
	float old_noise = verticalDataset->getTotalFalsePositives() + verticalDataset->getTotalFalseNegatives();
	float old_noise_cost = MDL_encoding_size(mn, old_noise);

	// compute new factors
	float new_noise = old_noise - n_items*n_trans + already_cov + 2*noise;
	float new_noise_cost = MDL_encoding_size(mn, new_noise);


	// --------------------- COMPUTE L(B) and L(C) -----------------------
	float delta_B = MDL_encoding_size(verticalDataset->getItems(), n_items);
	float delta_C = MDL_encoding_size(verticalDataset->getTrans(), n_trans);
	//float delta_B = MDL_encoding_size(50, n_items);
	//float delta_C = MDL_encoding_size(1000, n_trans);

	float delta_cost = delta_B + delta_C + new_noise_cost - old_noise_cost;

	/*
	std::cout << " ------------------ " << endl;
	std::cout << verticalDataset->getItems() << " x " << verticalDataset->getTrans() << endl;
	std::cout << verticalDataset->getSize() << std::endl;
	std::cout << mn << " - " << new_noise << endl;
	std::cout << "old noise cost " << old_noise_cost << endl;
	std::cout << "new noise cost " << new_noise_cost << endl;
	std::cout << "delta BC " << delta_B + delta_C << endl;
	std::cout << "delta cost " << delta_cost << endl;
	 */

	return delta_cost;
}

float MDL_NaiveXOR_initial_cost(void) {
	float mn = verticalDataset->getTrans()*verticalDataset->getItems();
	//float mn = 50*1000;
	return MDL_encoding_size(mn, verticalDataset->getSize());
}


/*
 * Per-row and per-column errors
 */
bool isTolerableTrans(int len, int noise){
	return (float)noise/(float)len <= item_tol_ratio;
}
bool isTolerableItem(int support, int noise){
	return (float)noise/(float)support <= tran_tol_ratio;
}


