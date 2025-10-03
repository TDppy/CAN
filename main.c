#include <stdio.h>
#include <stdlib.h>

#include "data_structure.h"
#include "func_declaration.h"
FILE *file;
int main() {
    SimParams* params = (SimParams*)malloc(sizeof(SimParams));
    setup_sim_params("config.txt",params);
    double inj_rate_start = params->inj_rate_start;
    double inj_rate_end   = params->inj_rate_end;
    double inj_rate_step  = params->inj_rate_step;
    int size_x         	  = params->size_x;
    //重定向输出
    file = fopen("noc_results_TP_GISU_4X4_BF8_VC2_SP50.txt","w");
    if (file == NULL) {
        perror("fopen failed");  // 打印具体错误原因（如“Permission denied”“No such file or directory”）
        free(params);                   // 避免内存泄漏
        return 1;                       // 退出程序，避免后续 fprintf 崩溃
    }
    print_sim_params(params);
    srand(1);
    for(double inj_rate = inj_rate_start ; inj_rate <= inj_rate_end ; inj_rate += inj_rate_step ){
        Network network;
        initialize_network(&network , params);
        NoC_congestion_matrix* congestion_matrix = (NoC_congestion_matrix*)malloc(sizeof(NoC_congestion_matrix));
        initialize_congestion_matrix(congestion_matrix, params);
        fprintf(file,"inj_rate = %.2f\n",inj_rate);
        printf("inj_rate = %.2f\n",inj_rate);
        simulate(&network, params, inj_rate, congestion_matrix);
        print_metrics(network);
        // 释放内存
        for (int i = 0; i < size_x; i++) {
            free(network.routers[i]);
        }
        free(network.routers);
    }
    fprintf(file,"-------------------------\n\n\n\n");
    free(params);
    return 0;
}
