#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "../header/data_structure.h"
#include "../header/func_declaration.h"

SelectedFlit* find_flit_for_out_dir(Router* router,SimParams* params,int out_dir,Network* last_network,NoC_congestion_matrix* matrix,int cur_cycle){
	int prior_strategy				 = params->prior_strategy;
	int size_y						 = params->size_y;
	int num_vcs						 = params->num_vcs;
	SelectedFlit* selected_flit		 = (SelectedFlit*)malloc(sizeof(SelectedFlit));
	SelectedFlit* selected_flit_list = (SelectedFlit*)malloc(sizeof(SelectedFlit) * NUM_DIRECTIONS * num_vcs);

	int match_flits_num = 0;
	for(int in_dir = 0; in_dir < NUM_DIRECTIONS; in_dir++ ){
		for(int vc_id = 0; vc_id < num_vcs; vc_id++){
			VirtualChannel* cur_vc = &router->vcs[in_dir][vc_id];
			if(cur_vc->count == 0) continue;
			Flit* flit = &cur_vc->buffer[cur_vc->head];
			if(calculate_routing(router,flit,size_y,params,last_network,cur_cycle,matrix) != out_dir){
				continue;
			}
			selected_flit->flit   = *flit;
			selected_flit->vc_id  = vc_id;
			selected_flit->in_dir = in_dir;
			selected_flit_list[match_flits_num++] = *selected_flit;
		}
	}
	if(match_flits_num!=0){
		// qos排序
		if(prior_strategy == 1){
			*selected_flit = *compare_by_qos(selected_flit_list, match_flits_num);
		}
		// 默认注入周期先后排序
		else{
			*selected_flit = *compare_by_time(selected_flit_list, match_flits_num);
		}
		free(selected_flit_list);
		return selected_flit;
	}else {
		free(selected_flit_list);
		return NULL;
	}

}


// 更新操作到cur_network中，查看操作在pre_network和last_router中
void router_transfer(Network* last_network, Network* cur_network, Router* last_router, SimParams* params,NoC_congestion_matrix* matrix){
	int size_x			= params->size_x;
	int size_y			= params->size_y;
	int buffer_size		= params->buffer_size;
	int cur_cycles		= cur_network->cur_cycles;
	//遍历所有输出端口
	for(int out_dir = 0; out_dir < NUM_DIRECTIONS; out_dir++){
		if(!is_output_port_available(last_router,out_dir,size_x,size_y)){
			continue;
		}
		SelectedFlit* selected_flit = NULL;
		// 寻找指定输出端口的备选flit
		selected_flit = find_flit_for_out_dir(last_router,params,out_dir,last_network,matrix,cur_cycles);
		// if (cur_cycles>432&&cur_cycles%5==0&&cur_network->total_flits_received>1340&&cur_network->total_flits_received<1360)
		// 	print_vc_occupancy(cur_network,params,cur_cycles);
		// 如果没有找到合适的flit，跳过当前输出方向，所以下面out_dir不仅仅是正在遍历的方向，也是flit要去的方向
		if(selected_flit == NULL) continue;
		int selected_vc_id  = selected_flit -> vc_id;
		int selected_in_dir = selected_flit -> in_dir;

		// 不管输出方向是不是local，都要释放本地vc，拿到pop_vc指针
		VirtualChannel* pop_vc = &cur_network->routers[last_router->x][last_router->y].vcs[selected_in_dir][selected_vc_id];
		// 执行到这一步说明找到了flit输出方向和out_dir相同
		// 判断输出方向是否为LOCAL
		if(out_dir == LOCAL) {
			selected_flit->flit.hops = selected_flit->flit.hops + 1;
			selected_flit->flit.path[selected_flit->flit.hops] = last_router->router_id;
			//计算端到端时延并释放资源
			cur_network->total_flits_received = cur_network->total_flits_received + 1;
			cur_network->total_flits_received_with_type[selected_flit->flit.qos]++;
			cur_network->total_latency += cur_cycles - selected_flit->flit.cycle_injected;
			cur_network->total_latency_with_type[selected_flit->flit.qos] += cur_cycles - selected_flit->flit.cycle_injected;
			vc_pop_flit(pop_vc,buffer_size);
			free(selected_flit);
			// 如果收到了所有flits，就更新max_num_cycles为当前周期，即收到最后一个flits的周期是max_num_cycles
			if (cur_network->total_flits_received == params->max_flits_num) {
				cur_network->max_num_cycles = cur_cycles;
			}
			if (cur_network->total_flits_received % 50 == 0 || params->max_flits_num - cur_network->total_flits_received < 10){
				//printf("cur_cycles = %d , cur_network has received %d flits\n",cur_cycles,cur_network->total_flits_received);
				//fprintf(file,"cur_cycles = %d ,cur_network has received %d flits\n",cur_cycles,cur_network->total_flits_received);
				if (params->max_flits_num - cur_network->total_flits_received < 10 || cur_cycles>432 && cur_cycles<435) {
					//print_vc_occupancy(cur_network, params, cur_network->cur_cycles);
				}
			}
			// for (int i = 1 ; i <= (selected_flit->flit.hops) ; i++) {
			// 	if (i == 1) {
			// 		fprintf(file,"Flit id = %d,hop = %d",selected_flit->flit.flit_id,selected_flit->flit.hops);
			// 		fprintf(file," R[%d][%d]",selected_flit->flit.path[i] / size_y,selected_flit->flit.path[i] % size_y);
			// 	}
			// 	else
			// 		fprintf(file,"->R[%d][%d]",selected_flit->flit.path[i] / size_y,selected_flit->flit.path[i] % size_y);
			// }
			// fprintf(file,"\n");
		} else {
			//申请下一跳的虚通道资源
			int next_x = last_router->x;
			int next_y = last_router->y;
			get_next_router_position(&next_x,&next_y,out_dir);
            int next_vc_id = allocate_virtual_channel(&last_network->routers[next_x][next_y], opposite_dir(out_dir),params);
			Router* next_router = &cur_network->routers[next_x][next_y];
			// 分配成功
            if (next_vc_id != -1) {
            	VirtualChannel* push_vc = &next_router->vcs[opposite_dir(out_dir)][next_vc_id];
                // 将flit复制到下一跳对应虚通道
                vc_push_flit(push_vc,&selected_flit->flit,buffer_size);
            	selected_flit->flit.hops = selected_flit->flit.hops + 1;
            	selected_flit->flit.path[selected_flit->flit.hops] = last_router->router_id;
                cur_network->total_link_utilization++;
                // 更新下一跳虚通道队列的tail指针和count数量
                // 释放flit占用的本地资源
                vc_pop_flit(pop_vc,buffer_size);
            }else {
	   //          if (params->max_flits_num - cur_network->total_flits_received < 10) {
	   //          	printf("【警告】Flit(ID: %d) 无法分配VC - 下一跳路由器 R[%d][%d] 方向: %s\n",
	   // selected_flit->flit.flit_id, next_x, next_y, direction_to_string(opposite_dir(out_dir)));
	   //          	fprintf(file, "【警告】Flit(ID: %d) 无法分配VC - 下一跳路由器 R[%d][%d] 方向: %s\n",
				// 		   selected_flit->flit.flit_id, next_x, next_y, direction_to_string(opposite_dir(out_dir)));
	   //          	// 打印当前网络VC占用，定位拥塞点
	   //          	print_vc_occupancy(cur_network, params, cur_network->cur_cycles);
	   //          }
            }
			free(selected_flit);
		}
	}
}

void simulate(Network* cur_network , SimParams* params , double inj_rate,NoC_congestion_matrix* congestion_matrix){
	int size_x				= params->size_x;
	int size_y				= params->size_y;
	double total_cycles		= params->total_cycles;
	Network* last_network	= deep_copy_network(cur_network);
	// 如果收不到最终的数据，可能会陷入无限循环
	for (int cur_cycle = 1; cur_network->max_num_cycles == 0; cur_cycle++) {
		cur_network->cur_cycles = cur_cycle;
		traffic_injection(cur_network, cur_cycle , inj_rate,params);
		// ----------------------- 全局互联调度的计算和配置 --------------------------------
		// 更新权重矩阵congestion_matrix
		update_weight_matrix(cur_network,cur_cycle,params,congestion_matrix);
		// -----------------------------------------------------------------------------
		for (int i = 0; i < size_x; i++) {
			for (int j = 0; j < size_y; j++) {
				router_transfer(last_network, cur_network,&last_network->routers[i][j] , params,congestion_matrix);
			}
		}
		// if (cur_network->max_num_cycles == cur_cycle) break;
		free_network(last_network,size_x,size_y);
		last_network = deep_copy_network(cur_network);
		if (params->max_flits_num - cur_network->total_flits_injected <= 10 && cur_cycle % 15 == 0) {
			//printf("cur_cycles = %d ,",cur_cycle);
			//printf("inj_rate = %.2f has inject %d flits\n",inj_rate,cur_network->total_flits_injected);
			//fprintf(file,"inj_rate = %.2f has inject %d flits\n",inj_rate,cur_network->total_flits_injected);
		}
	}
}

// 定时收集网络拥塞信息
void update_weight_matrix(Network* cur_network,int cur_cycle,SimParams* params,NoC_congestion_matrix* congestion_matrix) {
	// 每十个周期收集一次网络拥塞信息
	// 收集完成后应当立刻计算最短路，找到输出端口，后面十个周期都按照这个表来处理
	if (cur_cycle % params->global_sample_cycles != 0 || params->routing_algorithm!=3) return;
	initialize_congestion_matrix(congestion_matrix,params);
	// 获取router网络头指针
	Router** router = cur_network->routers;

	// 下面的i代表横轴上第i个Router，j代表纵轴上第j个Router
	// 分别收集向右/向左/向上/向下转发的链路拥塞
	// congestion_matrix->right_link[i][j]代表Router[i][j]向右转发的链路
	//                    left_link                      向左
	//                    up_link                        向上
	//                    down_link                      向下
	for (int i = 0 ; i < params->size_x ; i++) {
		for (int j = 0 ; j < params->size_y ; j++) {
			// 处理所有向右转发的链路（只有非最右侧的Router才会有）
			// right_link[i][j]代表router[i][j]右侧链路的拥塞情况
			if (i <= params->size_x - 2) {
				for (int k = 0 ; k < params->num_vcs ; k++) {
					// congestion_matrix
					congestion_matrix->right_link[i][j] += router[i+1][j].vcs[WEST][k].count;
				}
			}
			// 处理所有向左转发的链路（只有非最左侧的Router才会有）
			if (i >=1 ) {
				for (int k = 0 ; k < params->num_vcs ; k++) {
					congestion_matrix->left_link[i][j] += router[i-1][j].vcs[EAST][k].count;
				}
			}
			// 处理所有向上转发的链路（只有非最上侧的Router才会有）
			if (j <= params->size_y - 2) {
				for (int k = 0 ; k < params->num_vcs ; k++) {
					congestion_matrix->up_link[i][j] += router[i][j+1].vcs[SOUTH][k].count;
				}
			}
			// 处理所有向下转发的链路（只有非最下侧的Router才会有）
			if (j >= 1) {
				for (int k = 0 ; k < params->num_vcs ; k++) {
					congestion_matrix->down_link[i][j] += router[i][j-1].vcs[NORTH][k].count;
				}
			}
		}
	}
	// printf("congestion_matrix[2][2].left_link is full load!\n",congestion_matrix->left_link[2][2]==4);
}