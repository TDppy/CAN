#include <stdio.h>
#include <stdbool.h>

// ���src�Ƿ�����Ч��Χ�ڣ�0��size-1��
bool isValid(unsigned int src, unsigned int size) {
    return size > 0 && src < size;
}

// ������Ҫ�Ķ�����λ����������0��size-1����Сλ����
int getBitCount(unsigned int size) {
    if (size <= 1) return 1; // size=1ʱ����Χ0~0����Ҫ1λ

    int bits = 0;
    unsigned int current = 1; // 2^0 = 1
    while (current < size) {
        current <<= 1; // �ȼ���current *= 2
        bits++;
    }
    return bits;
}

// ����ָ��λ����ת������λ������ǰ��0������ȷ�������0~size-1��Χ��
unsigned int reverseBinary(unsigned int src, int bitCount, unsigned int size) {
    unsigned int result = 0;
    for (int i = 0; i < bitCount; i++) {
        // ȡ��src�����λ���ŵ�result�ĵ�ǰ���λ
        result = (result << 1) | (src & 1);
        // ����src����һλ
        src >>= 1;
    }
    // �ں����ڲ�����ȡģ������ȷ���������Ч��Χ
    return result % size;
}

int main() {
    unsigned int size, src;

    // printf("����������size: ");
    // scanf("%u", &size);
    //
    // printf("���������ת��ֵsrc: ");
    // scanf("%u", &src);
    int sizes[4] = {4,6,8};
    for (int i = 0 ; i < 3 ; i++) {
        size = sizes[i];
        for (int j = 0 ; j < size ; j++) {
            src = j;
            // ��֤����Ϸ���
            if (!isValid(src, size)) {
                printf("����: src������0��%d֮��\n", size - 1);
                return 1;
            }

            // ������Ҫ�Ķ�����λ��
            int bitCount = getBitCount(size);
            printf("��Ӧ������λ��: %dλ\n", bitCount);

            // ֱ�ӵ��ú������������ⲿ����ȡģ
            unsigned int reversed = reverseBinary(src, bitCount, size);

            printf("��ת���������: %u\n", reversed);
        }
        printf("\n");
    }
    return 0;
}
