#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "data_structure.h"
#include "func_declaration.h"
//#define VIS
// 功能:将方向的值转化为字符串
// 输入: int direction
// 输出: const char* 例如"LCOAL"的地址
const char* direction_to_string(int direction) {
    switch (direction) {
        case LOCAL: return "LOCAL";
        case EAST: return "EAST";
        case SOUTH: return "SOUTH";
        case WEST: return "WEST";
        case NORTH: return "NORTH";
        default: return "UNKNOWN"; // 如果输入无效的方向值
    }
}

// 判断输出端口是否可用
// 输入:Router* router,要检测的输出方向,横纵长度,纵轴长度
int is_output_port_available(Router* router, int output_direction, int size_x, int size_y) {
    int next_x = router->x;
    int next_y = router->y;

	// 根据输出方向计算下一跳Router的坐标
    switch (output_direction) {
        case EAST: {
        	next_x++;
        	break;
        }
        case SOUTH: {
        	next_y--;
        	break;
        }
        case WEST: {
        	next_x--;
        	break;
        }
        case NORTH: {
        	next_y++;
        	break;
        }
        case LOCAL: return 1; // Local方向始终可用
        default: return 0;
    }

	// 判断下一跳在不在网络内
    return (next_x >= 0 && next_x < size_x && next_y >= 0 && next_y < size_y);
}

// 计算路由方向
// 输入:Router* cur_router,要计算输出端口的flit,纵轴长度
int calculate_routing(Router* cur_router, Flit* flit, int size_y,SimParams* params,Network* last_network) {
	int dest_x		= flit->destination / size_y;
	int dest_y		= flit->destination % size_y;
	int cur_x		= cur_router->x;
	int cur_y		= cur_router->y;
	Router** router = last_network->routers;
	switch (params->routing_algorithm) {
		case 1 :
			// 比较目标路由器和当前路由器的坐标
			if (dest_x > cur_x) return EAST;
			if (dest_x < cur_x) return WEST;
			if (dest_y > cur_y) return NORTH;
			if (dest_y < cur_y) return SOUTH;
			return LOCAL;
		case 2 :
			// 目标在右边
			if (dest_x > cur_x) {
				if (dest_y > cur_y) {
					// 东边和北边
					int east_cnt = router[cur_x+1][cur_y].vcs[WEST][0].count + router[cur_x+1][cur_y].vcs[WEST][1].count;
					int north_cnt = router[cur_x][cur_y+1].vcs[SOUTH][0].count +router[cur_x][cur_y+1].vcs[SOUTH][1].count;
					return east_cnt < north_cnt ? EAST : NORTH;
				}
				else if (dest_y < cur_y) {
					// 东边和南边
					int east_cnt = router[cur_x+1][cur_y].vcs[WEST][0].count + router[cur_x+1][cur_y].vcs[WEST][1].count;
					int south_cnt = router[cur_x][cur_y-1].vcs[NORTH][0].count + router[cur_x][cur_y-1].vcs[NORTH][1].count;
					return east_cnt < south_cnt ? EAST : SOUTH;
				}
				else {
					// 东边
					return EAST;
				}
			}
            // 目标在左边
		    else if (dest_x < cur_x) {
				if (dest_y > cur_y) {
					// 西边和北边
					int west_cnt  = router[cur_x-1][cur_y].vcs[EAST][0].count + router[cur_x-1][cur_y].vcs[EAST][1].count;
					int north_cnt = router[cur_x][cur_y+1].vcs[SOUTH][0].count + router[cur_x][cur_y+1].vcs[SOUTH][1].count;
					return west_cnt < north_cnt ? WEST : NORTH;
				}
				else if (dest_y < cur_y) {
					// 西边和南边
					int west_cnt  = router[cur_x-1][cur_y].vcs[EAST][0].count + router[cur_x-1][cur_y].vcs[EAST][1].count;
					int south_cnt = router[cur_x][cur_y-1].vcs[NORTH][0].count + router[cur_x][cur_y-1].vcs[NORTH][1].count;
					return west_cnt < south_cnt ? WEST : SOUTH;
				}
		    	else {
		    		// 西边
		    		return WEST;
		    	}
			}
		    // 目标在当前x坐标
			else {
				if (dest_y > cur_y) {
					// 北边
					return NORTH;
				}
				else if (dest_y < cur_y) {
					// 南边
					return SOUTH;
				}
				else {
					// 当前
					return LOCAL;
				}
			}
			break;
		default:
			fprintf(file,"routing algorithm params error!\n");
	}

    return LOCAL;
}

// 获取相反方向
// 输入:当前路由器的目标输出端口值
// 返回值：下一跳路由器的输入端口值
int opposite_dir(int outdir) {
    switch (outdir) {
        case EAST: return WEST;
        case WEST: return EAST;
        case SOUTH: return NORTH;
        case NORTH: return SOUTH;
        case LOCAL: return LOCAL;
        default: return -1;
    }
}

// 虚通道分配
// 输入:目标router,申请虚通道的方向
// 分配原则：按顺序查看该方向的几个虚通道是否有空间，有就返回vc_id
// 返回值：被分配的vc_id
int allocate_virtual_channel(Router* router_ptr , int in_dir,SimParams* params) {
	int num_vcs = params->num_vcs;
	int buffer_size = params->buffer_size;
    for (int vc_id = 0; vc_id < num_vcs; vc_id++) {
        if (router_ptr->vcs[in_dir][vc_id].count < buffer_size) {
            return vc_id;
        }
    }
    return -1;
}


// 在vc中增加一个flit
// 输入:VirtualChannel* vc_ptr,Flit* flit_ptr
// 返回值:成功返回1，失败返回0
// 注意：由于vc的内存是网络初始化时候就分配好的，这里只需要把flit的值复制到vc的队尾
//       因此，要记得释放flit_ptr的内存
int vc_push_flit(VirtualChannel* vc_ptr , Flit* flit_ptr , int buffer_size){
	if(vc_ptr->count == buffer_size){
		return 0;
	}
	vc_ptr->buffer[vc_ptr->tail] = *flit_ptr;
    vc_ptr->tail = (vc_ptr->tail + 1) % buffer_size;
    vc_ptr->count++;
	return 1;
}

// 在vc中删除第一个flit
// 输入:VirtualChannel* vc_ptr
// 返回值:成功返回1，失败返回-1
int vc_pop_flit(VirtualChannel* vc_ptr , int buffer_size){
	if(vc_ptr->count == 0){
		return -1;
	}
	Flit zero_flit = {0};
	vc_ptr->buffer[vc_ptr->head] = zero_flit;
    vc_ptr->head = (vc_ptr->head + 1) % buffer_size;
    vc_ptr->count--;
	return 1;
}



// 传入当前router坐标，获取下一跳router坐标
void get_next_router_position(int* x,int* y,int out_dir){
    switch (out_dir) {
        case EAST: *x = *x + 1; break;
        case SOUTH: *y = *y - 1; break;
        case WEST: *x = *x - 1; break;
        case NORTH: *y = *y + 1; break;
    }
}

SelectedFlit* compare_by_qos(SelectedFlit* flit_list, int num) {
	int index = 0;
	for(int i = 0 ; i < num ; i++){
		if(flit_list[i].flit.qos < flit_list[index].flit.qos){
			index = i;
		}
	}
    // 返回值用于确定排序顺序，qos值越小，优先级越高
    return &flit_list[index];
}

SelectedFlit* compare_by_time(SelectedFlit* flit_list, int num) {
	int index = 0;
	for(int i = 0 ; i < num ; i++){
		if(flit_list[i].flit.cycle_injected < flit_list[index].flit.cycle_injected){
			index = i;
		}
	}
    return &flit_list[index];
}

SelectedFlit* find_flit_for_out_dir(Router* router,SimParams* params,int out_dir,Network* last_network){
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
			if(calculate_routing(router,flit,size_y,params,last_network) != out_dir){
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
void router_transfer(Network* last_network,Network* cur_network,Router* last_router,SimParams* params){
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
		selected_flit = find_flit_for_out_dir(last_router,params,out_dir,last_network);

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
			if (cur_network->total_flits_received % 25 == 0 || params->max_flits_num - cur_network->total_flits_received < 10){
				//printf("cur_network has received %d flits\n",cur_network->total_flits_received);
				//fprintf(file,"cur_network has received %d flits\n",cur_network->total_flits_received);
				if (params->max_flits_num - cur_network->total_flits_received < 10) {
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

void simulate(Network* cur_network , SimParams* params , double inj_rate){
	int size_x				= params->size_x;
	int size_y				= params->size_y;
	double total_cycles		= params->total_cycles;
	Network* last_network	= deep_copy_network(cur_network);
	// 如果收不到最终的数据，可能会陷入无限循环
	for (int cur_cycle = 1; cur_network->max_num_cycles == 0; cur_cycle++) {
		cur_network->cur_cycles = cur_cycle;
		traffic_injection(cur_network, cur_cycle , inj_rate,params);
		for (int i = 0; i < size_x; i++) {
			for (int j = 0; j < size_y; j++) {
				router_transfer(last_network, cur_network,&last_network->routers[i][j] , params);
			}
		}
		// if (cur_network->max_num_cycles == cur_cycle) break;
		free_network(last_network,size_x,size_y);
		last_network = deep_copy_network(cur_network);
		if (params->max_flits_num - cur_network->total_flits_injected <= 10 && cur_cycle < 50) {
			//printf("cur_cycles = %d ,",cur_cycle);
			//printf("inj_rate = %.2f has inject %d flits\n",inj_rate,cur_network->total_flits_injected);
			//fprintf(file,"inj_rate = %.2f has inject %d flits\n",inj_rate,cur_network->total_flits_injected);
		}
	}

}
