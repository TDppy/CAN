#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "data_structure.h"
#include "func_declaration.h"

int traffic_pattern(int source,SimParams* params,int pattern){
	int size_x		 = params->size_x;
	int size_y		 = params->size_y;
	int src_x		 = source / size_y;
	int src_y		 = source % size_y;
	int comple_src_x = size_x - 1 - src_x;
	int comple_src_y = size_y - 1 - src_y;
	int destination;
	switch(pattern){
		// uniform random
		case 1:destination = rand()%(size_x * size_y);break;
		// bit-complement
		case 2:destination = comple_src_x * size_y + comple_src_y;break;
		// transpose
		case 3:destination = src_y * size_x + src_x;break;
		// bit-reverse
		case 4: {
			int xBitCount = getBitCount(size_x);
			int yBitCount = getBitCount(size_y);
			int totalBitCnt = xBitCount + yBitCount;
			int srcTotalBits = src_x << yBitCount + src_y;
			destination = reverseBinary(srcTotalBits, totalBitCnt, 1 << (xBitCount + yBitCount));
		}
	}
	return destination;
}

int traffic_injection(Network* network , int cur_cycle , double inj_rate , SimParams* params){
	int size_x				= params->size_x;
	int size_y				= params->size_y;
	int num_vcs				= params->num_vcs;
	int buffer_size			= params->buffer_size;
	int traffic_pattern_num = params->traffic_pattern;
	for(int axis_x = 0;axis_x < size_x ; axis_x++){
		for(int axis_y = 0; axis_y < size_y ;axis_y++){
			if((double)rand() / RAND_MAX >= inj_rate  ) continue;
			if (network->total_flits_injected == params->max_flits_num) {
				return 1;
			}
			int qos_level = rand() % 2; //默认只有2种qos类型
			Flit* flit = (Flit*)malloc(sizeof(Flit));
			flit->flit_id = network->total_flits_injected;
			flit->qos = qos_level;
			flit->source = axis_x * size_y + axis_y;
			flit->destination = traffic_pattern(flit->source,params,traffic_pattern_num);
			flit->cycle_injected = cur_cycle;
			flit->path[1] = flit->source;
			flit->hops = 1;
			for(int vc_id = 0 ; vc_id < num_vcs ; vc_id++){
				VirtualChannel* vc = &network->routers[axis_x][axis_y].vcs[LOCAL][vc_id];
				if(vc_push_flit(vc, flit , buffer_size)){
					network->total_flits_injected++;
					break;
				}
			}
			free(flit);
		}
	}
	return 0;
}

// 计算需要的二进制位数（能容纳0到size-1的最小位数）
int getBitCount(unsigned int size) {
	if (size <= 1) return 1; // size=1时，范围0~0，需要1位

	int bits = 0;
	unsigned int current = 1; // 2^0 = 1
	while (current < size) {
		current <<= 1; // 等价于current *= 2
		bits++;
	}
	return bits;
}

// 按照指定位数翻转二进制位（包含前导0），并确保结果在0~size-1范围内
unsigned int reverseBinary(unsigned int src, int bitCount, unsigned int size) {
	unsigned int result = 0;
	for (int i = 0; i < bitCount; i++) {
		// 取出src的最低位，放到result的当前最低位
		result = (result << 1) | (src & 1);
		// 处理src的下一位
		src >>= 1;
	}
	// 在函数内部进行取模操作，确保结果在有效范围
	return result % size;
}