#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
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
int calculate_routing(Router* cur_router, Flit* flit, int size_y,SimParams* params,Network* last_network,int cur_cycle,NoC_congestion_matrix* matrix) {
	int dest_x		= flit->destination / size_y;
	int dest_y		= flit->destination % size_y;
	int cur_x		= cur_router->x;
	int cur_y		= cur_router->y;
	Coordinate cur,dest;
	cur.x = cur_x,cur.y = cur_y;
	dest.x = dest_x,dest.y = dest_y;
	Router** router = last_network->routers;
	int forward_port;
	switch (params->routing_algorithm) {
		case 1 :
			forward_port = DoR_routing(cur,dest);
			return forward_port;
		case 2 :
			forward_port = greedy_routing(router,cur,dest);
			return forward_port;
		case 3:
			forward_port = calculate_min_conges_port(cur_cycle,params,matrix,cur,dest);
			return forward_port;
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
		return 0;
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



/**
 * 功能：基于全局拥塞矩阵计算最优路由端口（routing_algorithm=3时调用）
 * 核心逻辑：在最少跳数路径约束下，选择拥塞代价最低的方向
 * 输入：
 *   cur_cycle：当前仿真周期（用于判断是否满足采样间隔）
 *   params：仿真参数（含网络大小、全局采样周期等）
 *   congestion_matrix：全局拥塞矩阵（存储各链路拥塞值）
 *   cur：当前路由器坐标（x/y）
 *   dest：目标路由器坐标（x/y）
 * 返回：
 *   最优端口（EAST/WEST/NORTH/SOUTH/LOCAL），-1表示当前周期不满足采样间隔
 */
int calculate_min_conges_port(int cur_cycle, SimParams* params,
                              NoC_congestion_matrix* congestion_matrix,
                              Coordinate cur, Coordinate dest) {
    // 1. 提取核心参数
    int curX = cur.x;          // 当前路由器X坐标
    int curY = cur.y;          // 当前路由器Y坐标
    int destX = dest.x;        // 目标路由器X坐标
    int destY = dest.y;        // 目标路由器Y坐标

    // 2. 边界条件：当前已到达目标，直接返回LOCAL（跳数为0）
    if (curX == destX && curY == destY) {
        return LOCAL;
    }

    // 3. 初始化DP矩阵：dp[i][j] = 从(i,j)到目标(destX,destY)的最小拥塞代价
    int dp[MAX_MESH_SIZE_X][MAX_MESH_SIZE_Y];
    for (int i = 0; i < MAX_MESH_SIZE_X; i++) {
        for (int j = 0; j < MAX_MESH_SIZE_Y; j++) {
            dp[i][j] = INT_MAX;  // 初始设为极大值（表示未计算）
        }
    }
    dp[destX][destY] = 0;  // 目标节点的代价为0（终点无拥塞）

    // 4. 计算最少跳数路径的精确范围（核心修正：以当前坐标为锚点，剔除冗余节点）
    // X轴范围：[min(curX, destX), max(curX, destX)]（仅遍历当前到目标的X区间）
    // int x_min = (curX < destX) ? curX : destX;
    // int x_max = (curX > destX) ? curX : destX;
    int x_start = destX;          // DP遍历起始点：目标X（从目标反向推导）
    int x_end = curX;             // DP遍历终止点：当前X（无需遍历到0）
    int x_step = (destX > curX) ? -1 : 1;  // 步长：目标在东→递减，在西→递增

    // Y轴范围：[min(curY, destY), max(curY, destY)]（仅遍历当前到目标的Y区间）
    // int y_min = (curY < destY) ? curY : destY;
    // int y_max = (curY > destY) ? curY : destY;
    int y_start = destY;          // DP遍历起始点：目标Y
    int y_end = curY;             // DP遍历终止点：当前Y
    int y_step = (destY > curY) ? -1 : 1;  // 步长：目标在北→递减，在南→递增

    // 5. 确定最少跳数的可选方向（仅2个方向，避免绕路）
    int dx = destX - curX;  // X轴方向差：正=东，负=西
    int dy = destY - curY;  // Y轴方向差：正=北，负=南
    int allowed_dirs[2] = {-1, -1};  // 最少跳数的可选方向（如EAST+NORTH）
    int dir_count = 0;                // 可选方向数量（只可能是1或2）

    // 根据目标相对于当前的位置，锁定最少跳数方向
    if (dx > 0 && dy > 0) {         // 目标在东北方→允许EAST/NORTH
        allowed_dirs[0] = EAST;
        allowed_dirs[1] = NORTH;
        dir_count = 2;
    } else if (dx > 0 && dy < 0) {  // 目标在东南方→允许EAST/SOUTH
        allowed_dirs[0] = EAST;
        allowed_dirs[1] = SOUTH;
        dir_count = 2;
    } else if (dx < 0 && dy > 0) {  // 目标在西北方→允许WEST/NORTH
        allowed_dirs[0] = WEST;
        allowed_dirs[1] = NORTH;
        dir_count = 2;
    } else if (dx < 0 && dy < 0) {  // 目标在西南方→允许WEST/SOUTH
        allowed_dirs[0] = WEST;
        allowed_dirs[1] = SOUTH;
        dir_count = 2;
    } else {                        // 理论上calculate_routing已处理同一轴，防异常
        allowed_dirs[0] = (dx != 0) ? ((dx > 0) ? EAST : WEST) : ((dy > 0) ? NORTH : SOUTH);
        dir_count = 1;
    }
	int best_port = LOCAL;
	// 6. DP核心计算：从目标反向遍历，计算每个节点的最小拥塞代价（无neiX/neiY）
	for (int i = x_start;
	     (destX > curX) ? (i >= x_end) : (i <= x_end);
	     i += x_step) {

	    for (int j = y_start;
	         (destY > curY) ? (j >= y_end) : (j <= y_end);
	         j += y_step) {

	        if (i == destX && j == destY) continue;  // 跳过目标（代价已知为0）

	        int min_cost = INT_MAX;

	        // 遍历所有允许方向（最多就2个方向），读取链路拥塞信息并更新min_cost，该循环后面会把min_cost赋值给dp[i][j]
	        for (int d = 0; d < dir_count; d++) {
	            int dir = allowed_dirs[d];
	            int link_cost = 0;  // 仅需当前节点的链路拥塞值，无邻居坐标

	            switch (dir) {
	                case EAST:
	                    // 东向链路拥塞 = 当前节点(i,j)的right_link（无需知道东邻居是(i+1,j)）
	                    link_cost = congestion_matrix->right_link[i][j];
	                    // 目标方向节点的DP值 = 东向节点(i+1,j)的dp值（直接推导，无需存储neiX）
	                    if ( dp[i + 1][j] != INT_MAX) {
	                        int total_cost = dp[i + 1][j] + link_cost;
	                    	if (total_cost < min_cost) {
	                    		min_cost = total_cost;
	                    		best_port = dir;
	                    	}
	                    }
	                    break;
	                case WEST:
	                    // 西向链路拥塞 = 当前节点(i,j)的left_link
	                    link_cost = congestion_matrix->left_link[i][j];
	                    // 目标方向节点的DP值 = 西向节点(i-1,j)的dp值
	                    if (dp[i - 1][j] != INT_MAX) {
	                        int total_cost = dp[i - 1][j] + link_cost;
	                    	if (total_cost < min_cost) {
	                    		min_cost = total_cost;
	                    		best_port = dir;
	                    	}
	                    }
	                    break;
	                case NORTH:
	                    // 北向链路拥塞 = 当前节点(i,j)的up_link（你的核心结论：对应北邻居南侧缓存）
	                    link_cost = congestion_matrix->up_link[i][j];
	                    // 目标方向节点的DP值 = 北向节点(i,j+1)的dp值
	                    if (dp[i][j + 1] != INT_MAX) {
	                        int total_cost = dp[i][j + 1] + link_cost;
	                    	if (total_cost < min_cost) {
	                    		min_cost = total_cost;
	                    		best_port = dir;
	                    	}
	                    }
	                    break;
	                case SOUTH:
	                    // 南向链路拥塞 = 当前节点(i,j)的down_link
	                    link_cost = congestion_matrix->down_link[i][j];
	                    // 目标方向节点的DP值 = 南向节点(i,j-1)的dp值
	                    if (dp[i][j - 1] != INT_MAX) {
	                        int total_cost = dp[i][j - 1] + link_cost;
	                    	if (total_cost < min_cost) {
	                    		min_cost = total_cost;
	                    		best_port = dir;
	                    	}
	                    }
	                    break;
	                default:
	                    continue;
	            }
	        }

	        // 更新当前节点的DP值
	        if (min_cost != INT_MAX) {
	            dp[i][j] = min_cost;
	        }
	    }
	}
	// if (cur_cycle > 432) printf("cur_x = %d, cur_y = %d, dest_x = %d, dest_y = %d,best_port = %s\n",
	// 								   cur.x,cur.y,dest.x,dest.y,direction_to_string(best_port));
    // 返回最终的最优方向（最少跳数+最低拥塞）
	if ((cur.x!=dest.x || cur.y!=dest.y) && best_port == LOCAL) {
		printf("error!");
		exit(0);
	}
    return best_port;
}

void south_west_dp(SimParams* params, NoC_congestion_matrix* congestion_matrix, InterConnectSchedulerTable* scheduler_table) {
	int (*south_west_dp)[MAX_MESH_SIZE_Y] = scheduler_table->south_west_dp;
	int (*south_west_port_choose)[MAX_MESH_SIZE_Y] = scheduler_table->south_west_port;
	south_west_dp[0][0] = 0;
	south_west_port_choose[0][0] = LOCAL;
	// 1. 边界条件：第一行（y=0，所有x）
	for (int i = 1; i < params->size_x; i++) {
		south_west_dp[i][0] = south_west_dp[i-1][0] + congestion_matrix->left_link[i][0];
		south_west_port_choose[i][0] = WEST;
	}
	// 边界条件：第一列（x=0，所有y）
	for (int j = 1; j < params->size_y; j++) {
		south_west_dp[0][j] = south_west_dp[0][j-1] + congestion_matrix->down_link[0][j];
		south_west_port_choose[0][j] = SOUTH;
	}

	// 2. 扩散计算：覆盖所有非边界路由器（i从1开始，j从1开始）
	for (int i = 1; i < params->size_x; i++) { // 原i=2 → 改为i=1
		for (int j = 1; j < params->size_y; j++) { // 原j=2 → 改为j=1
			int left_cost = south_west_dp[i-1][j] + congestion_matrix->left_link[i][j];
			int down_cost = south_west_dp[i][j-1] + congestion_matrix->down_link[i][j];
			// 选择成本更低的路径
			if (left_cost < down_cost) {
				south_west_dp[i][j] = left_cost;
				south_west_port_choose[i][j] = WEST;
			} else {
				south_west_dp[i][j] = down_cost;
				south_west_port_choose[i][j] = SOUTH;
			}
		}
	}
}

int greedy_routing(Router** router,Coordinate cur,Coordinate dest) {
	int cur_x = cur.x, cur_y = cur.y;
	int dest_x = dest.x, dest_y = dest.y;
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
}
int DoR_routing(Coordinate cur,Coordinate dest) {
	int cur_x = cur.x, cur_y = cur.y;
	int dest_x = dest.x, dest_y = dest.y;
	// 比较目标路由器和当前路由器的坐标
	if (dest_x > cur_x) return EAST;
	if (dest_x < cur_x) return WEST;
	if (dest_y > cur_y) return NORTH;
	if (dest_y < cur_y) return SOUTH;
	return LOCAL;
}