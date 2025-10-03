#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "data_structure.h"
#include "func_declaration.h"
// 功能:初始化虚通道，将head,tail,count赋值为0
// 输入:VirtualChannel* vc
void initialize_virtual_channel(VirtualChannel* vc) {
    vc->head = 0;
    vc->tail = 0;
    vc->count = 0;
}

// 初始化路由器
// 输入:路由器指针、id、横纵坐标
void initialize_router(Router* router, int router_id, int x, int y,int num_vcs) {
    router->router_id = router_id;
    router->x = x;
    router->y = y;
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        for (int j = 0; j < num_vcs; j++) {
            initialize_virtual_channel(&router->vcs[i][j]);
        }
    }
}

// 初始化网络
// 输入:Network* network,横轴长度，纵轴长度
void initialize_network(Network* network, SimParams* params) {
    int size_x = network->size_x = params->size_x;
    int size_y = network->size_y = params->size_y;
    int num_vcs = params->num_vcs;
    network->max_num_cycles = 0;
    network->cur_cycles = 0;
    network->total_flits_injected = 0;
    network->total_flits_received = 0;
    network->total_latency = 0;
    network->total_link_utilization = 0;
    memset(network->total_flits_received_with_type, 0, QOS_TYPE_NUM * sizeof(int));
    memset(network->total_latency_with_type,0,QOS_TYPE_NUM * sizeof(int));
    network->routers = (Router**)malloc(size_x * sizeof(Router*));

    // 初始化网络:
    //   遍历网络中所有Router
    //      把网络中每个router_id根据坐标赋值
    //      把网络中每个router的虚通道初始化head、tail、count
    for (int i = 0; i < size_x; i++) {
        network->routers[i] = (Router*)malloc(size_y * sizeof(Router));
        for (int j = 0; j < size_y; j++) {
            int router_id = i * size_y + j;
            initialize_router(&network->routers[i][j], router_id, i, j,num_vcs);
        }
    }
}

void initialize_congestion_matrix(NoC_congestion_matrix* matrix , SimParams* params){
    for (int i = 0 ; i < params->size_x ; i++) {
        for (int j = 0 ; j < params->size_y ; j++) {
            matrix->down_link[i][j] = 0;
            matrix->up_link[i][j] = 0;
            matrix->left_link[i][j] = 0;
            matrix->right_link[i][j] = 0;
        }
    }
}

void initialize_scheduler_table(SimParams* params ,InterConnectSchedulerTable* scheduler_table) {
    for (int i = 0; i < params->size_x ; i++) {
        for (int j = 0; j < params->size_y ; j++) {
            scheduler_table->north_east_dp[i][j] = 0;
            scheduler_table->north_east_port[i][j] = LOCAL;
            scheduler_table->north_west_dp[i][j] = 0;
            scheduler_table->north_west_port[i][j] = LOCAL;
            scheduler_table->south_east_dp[i][j] = 0;
            scheduler_table->south_east_port[i][j] = LOCAL;
            scheduler_table->south_west_dp[i][j] = 0;
            scheduler_table->south_west_port[i][j] = LOCAL;
        }
    }
}