#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "../header/data_structure.h"
#include "../header/func_declaration.h"
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