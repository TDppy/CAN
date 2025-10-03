#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "data_structure.h"
#include "func_declaration.h"


void print_metrics(Network network){
	fprintf(file,"Total Flits Injected: %d\n", network.total_flits_injected);
    if (network.total_flits_received > 0) {
        double avg_latency = (double)network.total_latency / network.total_flits_received;
        fprintf(file,"total_latency = %d total_flits_received = %d\n",network.total_latency,network.total_flits_received);
        fprintf(file,"Average Latency: %.2f cycles\n", avg_latency);
        // for(int i = 0;i < 2; i++){
        // 	if(network.total_flits_received_with_type[i]>0)
        // 	fprintf(file,"Average Latency with QoS%d = %.2f\n",i,(double)network.total_latency_with_type[i]/network.total_flits_received_with_type[i]);
        // }
    } else {
        fprintf(file,"Average Latency: N/A (No flits received)\n");
    }

    int total_links = 4 * network.size_x * network.size_y - 2 * network.size_x -2 * network.size_y;

    // 一共用了多少次  除以 （总链路数量 * 最后一个flit收到的周期）
    if (network.max_num_cycles!=0 ) {
        double utilization = (double)network.total_link_utilization / (total_links * network.max_num_cycles) * 100;
        fprintf(file,"Link Utilization: %.2f%%\n", utilization);
    }else {
        double utilization = (double)network.total_link_utilization / (total_links * network.cur_cycles) * 100;
        fprintf(file,"Link Utilization: %.2f%%\n", utilization);
    }

    if (network.max_num_cycles!=0) {
        double throughput = (double)network.total_flits_received / network.max_num_cycles;
        fprintf(file,"Throughput: %.2f flits/cycle\n", throughput);
    }else {
        double throughput = (double)network.total_flits_received / network.cur_cycles;
        fprintf(file,"Throughput: %.2f flits/cycle\n", throughput);
    }

    fprintf(file,"\n");
}


SimParams* setup_sim_params(const char* filename,SimParams* params){
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("无法打开配置文件");
        return NULL;
    }

    char line[256];
    char output_file[256] = "";

    while (fgets(line, sizeof(line), file)) {
        // 去掉行尾的换行符
        line[strcspn(line, "\n")] = '\0';

        // 跳过空行和注释行
        if (line[0] == '\0' || strncmp(line, "//", 2) == 0) {
            continue;
        }

        // 解析 key=value 形式的行
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "=");

        if (key && value) {
            // 去掉 key 和 value 前后的空格
            key = strtok(key, " \t");
            value = strtok(value, " \t");
            
            if(strcmp(key,"size_x")==0){
            	params->size_x = atoi(value);
            }else if(strcmp(key,"size_y")==0){
            	params->size_y = atoi(value);
            }else if(strcmp(key,"inj_rate_start")==0){
            	params->inj_rate_start = atof(value);
            }else if(strcmp(key,"inj_rate_end")==0){
            	params->inj_rate_end = atof(value);
            }else if(strcmp(key,"inj_rate_step")==0){
            	params->inj_rate_step = atof(value);
            }else if (strcmp(key, "buffer_size") == 0) {
                params->buffer_size = atoi(value);
            }else if (strcmp(key, "total_cycles") == 0) {
                params->total_cycles = atoi(value);
            }else if (strcmp(key, "max_flits_num") == 0) {
                params->max_flits_num = atoi(value);
            }else if (strcmp(key, "num_vcs") == 0) {
                params->num_vcs = atoi(value);
            }else if(strcmp(key,"traffic_pattern") == 0){
            	params->traffic_pattern = atoi(value);
            }else if(strcmp(key,"prior_strategy")==0){
            	params->prior_strategy = atoi(value);
            }else if (strcmp(key,"routing_algorithm") == 0) {
                params->routing_algorithm = atoi(value);
            }else if (strcmp(key,"global_sample_cycles")==0) {
                params->global_sample_cycles = atoi(value);
            }else;
        }
    }

    fclose(file);	
	
	return params;
}
void print_sim_params(SimParams* params){
    printf("prior_strategy    = %d     \n",params->prior_strategy);
    printf("traffic_pattern   = %d     \n",params->traffic_pattern);
    printf("routing_algorithm = %d     \n",params->routing_algorithm);
    printf("inj_rate_start    = %.2f   \n",params->inj_rate_start);
    printf("inj_rate_end      = %.2f   \n",params->inj_rate_end);
    printf("inj_rate_step     = %.2f   \n",params->inj_rate_step);
    printf("size_x            = %d     \n",params->size_x);
    printf("size_y            = %d     \n",params->size_y);
    printf("buffer_size       = %d     \n",params->buffer_size);
    printf("total_cycles      = %d     \n",params->total_cycles);
    printf("max_flits_num     = %d     \n",params->max_flits_num);
    printf("num_vcs           = %d     \n",params->num_vcs);
    printf("---------------------------\n");

    fprintf(file,"prior_strategy    = %d     \n",params->prior_strategy);
    fprintf(file,"traffic_pattern   = %d     \n",params->traffic_pattern);
    fprintf(file,"routing_algorithm = %d     \n",params->routing_algorithm);
    fprintf(file,"inj_rate_start    = %.2f   \n",params->inj_rate_start);
    fprintf(file,"inj_rate_end      = %.2f   \n",params->inj_rate_end);
    fprintf(file,"inj_rate_step     = %.2f   \n",params->inj_rate_step);
    fprintf(file,"size_x            = %d     \n",params->size_x);
    fprintf(file,"size_y            = %d     \n",params->size_y);
    fprintf(file,"buffer_size       = %d     \n",params->buffer_size);
    fprintf(file,"total_cycles      = %d     \n",params->total_cycles);
    fprintf(file,"max_flits_num     = %d     \n",params->max_flits_num);
    fprintf(file,"num_vcs           = %d     \n",params->num_vcs);
    fprintf(file,"---------------------------\n");
}



// （保留原有函数实现...）

void print_vc_occupancy(Network* network, SimParams* params, int cur_cycle) {
    // 从参数中获取关键配置（避免硬编码，与仿真参数同步）
    int size_x = params->size_x;       // 网络X轴路由器数量
    int size_y = params->size_y;       // 网络Y轴路由器数量
    int num_vcs = params->num_vcs;     // 每个方向的VC数量
    int buffer_size = params->buffer_size; // 每个VC的最大缓存容量
    int total_vc_used = 0;
    // 打印标题（标注当前周期，便于定位时间点）
    char title[128];
    snprintf(title, sizeof(title), "=== VC occupation situation:  -  cycles: %d ===", cur_cycle);
    printf("%s\n", title);
    fprintf(file, "%s\n", title); // 同步写入日志文件

    // 1. 遍历网络中所有路由器（按X→Y坐标顺序）
    for (int x = 0; x < size_x; x++) {
        for (int y = 0; y < size_y; y++) {
            Router* cur_router = &network->routers[x][y]; // 当前路由器指针

            // 打印当前路由器的基本信息（坐标+ID）
            printf("Router R[%d][%d] (ID: %d)\n", x, y, cur_router->router_id);
            fprintf(file, "Router R[%d][%d] (ID: %d)\n", x, y, cur_router->router_id);

            // 2. 遍历当前路由器的所有方向（LOCAL/EAST/SOUTH/WEST/NORTH）
            for (int dir = 0; dir < NUM_DIRECTIONS; dir++) {
                const char* dir_str = direction_to_string(dir); // 方向转字符串（复用现有函数）

                // 打印当前方向的VC汇总（先显示方向名称）
                printf("  direction: %-6s | ", dir_str); // %-6s 左对齐，保证格式整齐
                fprintf(file, "  direction: %-6s | ", dir_str);

                // 3. 遍历当前方向的所有VC（按VC ID顺序）
                for (int vc_id = 0; vc_id < num_vcs; vc_id++) {
                    VirtualChannel* cur_vc = &cur_router->vcs[dir][vc_id]; // 当前VC指针
                    int used = cur_vc->count; // 当前VC已占用的缓存数
                    int total = buffer_size;  // VC最大缓存数（来自配置）
                    total_vc_used+=used;
                    // 打印单个VC的占用情况（格式：VC[ID]: 已用/总容量）
                    printf("VC[%d]: %d/%d | ", vc_id, used, total);
                    fprintf(file, "VC[%d]: %d/%d | ", vc_id, used, total);

                                        // 4. 补充：遍历当前VC内的所有Flit，打印src/dest坐标
                    if (used > 0) { // 仅当VC有Flit时才打印详情
                        printf("Flits: [");
                        fprintf(file, "Flits: [");

                        // 循环队列遍历逻辑：从head开始，遍历count个Flit（处理tail < head的循环情况）
                        for (int i = 0; i < used; i++) {
                            // 计算当前Flit在buffer中的索引（循环队列公式）
                            int flit_idx = (cur_vc->head + i) % buffer_size;
                            Flit* cur_flit = &cur_vc->buffer[flit_idx];

                            // 计算Flit的源坐标（src_x/src_y）和目标坐标（dest_x/dest_y）
                            int src_x = cur_flit->source / size_y;
                            int src_y = cur_flit->source % size_y;
                            int dest_x = cur_flit->destination / size_y;
                            int dest_y = cur_flit->destination % size_y;

                            // 打印单个Flit的关键信息（ID + 源坐标 + 目标坐标）
                            if (i == used - 1) {
                                // 最后一个Flit，无 trailing comma
                                printf("ID:%d(from[%d][%d]to[%d][%d])",
                                       cur_flit->flit_id, src_x, src_y, dest_x, dest_y);
                                fprintf(file, "ID:%d(from[%d][%d]to[%d][%d])",
                                       cur_flit->flit_id, src_x, src_y, dest_x, dest_y);
                            } else {
                                // 非最后一个Flit，加逗号分隔
                                printf("ID:%d(from[%d][%d]to[%d][%d]), ",
                                       cur_flit->flit_id, src_x, src_y, dest_x, dest_y);
                                fprintf(file, "ID:%d(from[%d][%d]to[%d][%d]), ",
                                       cur_flit->flit_id, src_x, src_y, dest_x, dest_y);
                            }
                        }
                        printf("] | ");
                        fprintf(file, "] | ");
                    } else {
                        // VC无Flit时，打印空标识，保持格式对齐
                        printf("Flits: [] | ");
                        fprintf(file, "Flits: [] | ");
                    }
                }

                // 每个方向打印完换行，避免内容拥挤
                printf("\n");
                fprintf(file, "\n");
            }

            // 每个路由器打印完空一行，区分不同路由器
            printf("\n");
            fprintf(file, "\n");
        }
    }
    if (total_vc_used == 0) {
        printf("NO FLITS IN NETWORK!\n");
        fprintf(file,"NO FLITS IN NETWORK!\n");
    }
    // 整个网络VC信息打印完，添加分隔线，便于日志阅读
    printf("=========================================================\n\n");
    fprintf(file, "=========================================================\n\n");
}