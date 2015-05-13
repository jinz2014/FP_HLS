#define BIG 1000 // a big weight

#include "hungarian.h"

void TableUpdateMUXR(var_struct_ptr var_MUXR_table, int reg_nu, int varID);

void TableUpdateMUXP(var_struct_ptr var_MUXP_table, int reg_nu, int varID);

void hungarian_method(int *match, int *cost, int size);

void PrintMatrix(int *cost, int row, int col);

int** array_to_matrix(int* m, int rows, int cols);

varList *CreateCluster(varList *setV, int clus_nu, int varTableIdx, int minRegNu);

varList *CreateWBMClusters(varList *setV, int delay, int* minRegNu, int *clusterNu);

varList *CreateWBMClustersMax(varList *setV, int delay, int *minRegNu, int *clusterNu);

void FindMaxVarSet(varList head, int entry, int delay, int *maxRegNu, int *maxTime);
