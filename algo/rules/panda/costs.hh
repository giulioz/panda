#ifndef __COST_FUNCTIONS_HH
#define __COST_FUNCTIONS_HH


/*
 * Sets up auxiliary data for complex cost functions.
 */
void costAuxSetup(float alpha=.5, void* vert=0, float item_tolerance_ratio=1.0, float tran_tolerance_ratio=1.0);

/*
 * Delta cost when adding a new pattern with n_items (param1),
 *   n_trans (param2), generating some noise (param3),
 *   and overlapping some already_cov bits (param4).
 */
typedef float cost_function_type(int, int, int, int);
typedef float initial_cost_function_type(void);

float PANDA_delta_cost(int n_items, int n_trans, int noise, int already_cov);
float PANDA_initial_cost(void);

float PANDA_weighted_delta_cost(int n_items, int n_trans, int noise, int already_cov);
float PANDA_weighted_initial_cost(void);

/*
 * Weighted Panda L2 cost.
 */
float PANDA_weighted_L2_delta_cost(int n_items, int n_trans, int noise, int already_cov);

/*
 * @pre: requires vertical dataset to be initialized
 * @note: From paper "Model order selection for boolean matrix factorization", KDD 2011
 */
float MDL_TypedXOR_delta_cost(int n_items, int n_trans, int noise, int already_cov);
float MDL_TypedXOR_initial_cost(void);

/*
 * @pre: requires vertical dataset to be initialized
 * @note: From paper "Model order selection for boolean matrix factorization", KDD 2011
 */
float MDL_NaiveXOR_delta_cost(int n_items, int n_trans, int noise, int already_cov);
float MDL_NaiveXOR_initial_cost(void);

/*
 * Per-row and per-column errors
 */
bool isTolerableTrans(int len, int noise);
bool isTolerableItem(int support, int noise);
#endif
