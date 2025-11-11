#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../header/data_structure.h"
#include "../header/func_declaration.h"


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

int shuffle_traffic(int src_x, int src_y, int size_x, int size_y) {
    // 1. 计算总节点数与所需二进制位数（确保覆盖所有路由器ID）
    int total_nodes = size_x * size_y;
    int total_bit_cnt = getBitCount(total_nodes);  // 复用现有工具函数（计算容纳0~total_nodes-1的最小位数）

    // 2. 源坐标转换为唯一ID（src_id = src_x * size_y + src_y）
    unsigned int src_id = (unsigned int)(src_x * size_y + src_y);

    // 3. 二进制循环左移1位（洗牌核心逻辑）
    if (total_bit_cnt == 0) {  // 异常处理：拓扑大小为0（理论不会触发）
        printf( "Error: Invalid total_bit_cnt = 0 in shuffle_traffic!\n");
        return src_id;  //  fallback到源ID，避免崩溃
    }
    unsigned int msb = (src_id >> (total_bit_cnt - 1)) & 1;  // 提取最高位（如4位ID 1010的最高位是1）
    unsigned int shifted_id = (src_id << 1) | msb;            // 左移1位 + 最高位补到最低位（1010→0101）

    // 4. 边界保护：确保目标ID在0~total_nodes-1范围内
    unsigned int dest_id = shifted_id % total_nodes;

    // 5. 调试信息（可选开启，验证计算过程）
    // fprintf(file, "[Shuffle Debug] src(%d,%d) → src_id=%u → shifted_id=%u → dest_id=%u\n",
    //         src_x, src_y, src_id, shifted_id, dest_id);

    return (int)dest_id;
}