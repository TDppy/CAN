#include <stdio.h>
#include <stdbool.h>

// 检查src是否在有效范围内（0到size-1）
bool isValid(unsigned int src, unsigned int size) {
    return size > 0 && src < size;
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

int main() {
    unsigned int size, src;

    // printf("请输入上限size: ");
    // scanf("%u", &size);
    //
    // printf("请输入待翻转数值src: ");
    // scanf("%u", &src);
    int sizes[4] = {4,6,8};
    for (int i = 0 ; i < 3 ; i++) {
        size = sizes[i];
        for (int j = 0 ; j < size ; j++) {
            src = j;
            // 验证输入合法性
            if (!isValid(src, size)) {
                printf("错误: src必须在0到%d之间\n", size - 1);
                return 1;
            }

            // 计算需要的二进制位数
            int bitCount = getBitCount(size);
            printf("对应二进制位数: %d位\n", bitCount);

            // 直接调用函数，无需在外部处理取模
            unsigned int reversed = reverseBinary(src, bitCount, size);

            printf("翻转后的整数是: %u\n", reversed);
        }
        printf("\n");
    }
    return 0;
}
