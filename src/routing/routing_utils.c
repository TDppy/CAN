#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "../header/data_structure.h"
#include "../header/func_declaration.h"

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