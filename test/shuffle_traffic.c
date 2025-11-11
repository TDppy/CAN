#include <stdio.h>
#include <stdlib.h>
// ---------------------- 依赖工具函数（若未定义需补充，已有则忽略） ----------------------
// 计算容纳0~size-1的最小二进制位数（如size=16→4位，size=64→6位）
int getBitCount(unsigned int size) {
    if (size <= 1) return 1;
    int bits = 0;
    unsigned int current = 1;
    while (current < size) {
        current <<= 1;  // 等价于current *= 2
        bits++;
    }
    return bits;
}

/**
 * 独立Shuffle流量模式函数：计算源路由器对应的目标路由器ID
 * @param src_x：源路由器X坐标（0~size_x-1）
 * @param src_y：源路由器Y坐标（0~size_y-1）
 * @param size_x：Mesh拓扑X轴路由器数量（如4、8）
 * @param size_y：Mesh拓扑Y轴路由器数量（如4、8）
 * @return 目标路由器ID（dest_id = dest_x × size_y + dest_y）
 */
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


// 4×4 Mesh Shuffle模式测试函数
void test_shuffle_4x4() {
    int size_x = 4, size_y = 4;
    printf("=== 4×4 Mesh Shuffle Traffic Test ===\n");
    printf("src_x\tsrc_y\tsrc_id\tdest_id\tdest_x\tdest_y\n");
    printf("----------------------------------------\n");

    for (int src_x = 0; src_x < size_x; src_x++) {
        for (int src_y = 0; src_y < size_y; src_y++) {
            int src_id = src_x * size_y + src_y;
            int dest_id = shuffle_traffic(src_x, src_y, size_x, size_y);
            int dest_x = dest_id / size_y;
            int dest_y = dest_id % size_y;

            // 打印结果（对比预期表验证）
            printf("%d\t%d\t%d\t%d\t%d\t%d\n",
                   src_x, src_y, src_id, dest_id, dest_x, dest_y);
        }
    }
    printf("========================================\n\n");
}

// 8×8 Mesh 全节点测试函数（遍历所有64个路由器）
void test_shuffle_8x8_full() {
    int size_x = 8, size_y = 8;
    printf("=== 8×8 Mesh Shuffle Traffic (Full Nodes Test) ===\n");
    printf("src_x\\src_y | 0    1    2    3    4    5    6    7  \n");
    printf("----------------------------------------------------\n");

    // 遍历所有src_x（0~7），每行对应1个src_x，列对应src_y（0~7）
    for (int src_x = 0; src_x < size_x; src_x++) {
        printf("src_x=%-2d    | ", src_x);

        // 遍历当前src_x下的所有src_y（0~7）
        for (int src_y = 0; src_y < size_y; src_y++) {
            int src_id = src_x * size_y + src_y;
            int dest_id = shuffle_traffic(src_x, src_y, size_x, size_y);
            // 格式化输出dest_id（占4位，右对齐，确保列对齐）
            printf("%-4d", dest_id);
        }
        printf("\n");  // 每行结束换行
    }
    printf("====================================================\n");
}

int main() {
    test_shuffle_4x4();   // 先测试4×4拓扑
    test_shuffle_8x8_full();   // 再测试8×8拓扑
    return 0;
}