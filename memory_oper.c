#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "data_structure.h"
#include "func_declaration.h"

// 深拷贝网络
// 输入:const Network* src
// 返回值:Network* copied_network
// const Network* 这个指针所指向的内容不能被修改,即拷贝过程中源内容是只读的
// 注意：这里分配了copied network的内存，在使用完成后，应当释放该内存
Network* deep_copy_network(const Network* src) {
    //处理空指针异常:源指针为空
    if (!src) {
        fprintf(file,"null pointer exception\n");
        printf("null pointer exception\n");
        return NULL;
    }
    Network* dest = (Network*)malloc(sizeof(Network));

    //处理空指针异常:分配Network内存失败
    if (!dest) {
        fprintf(file,"null pointer exception\n");
        printf("null pointer exception\n");
        return NULL;
    }

    // 拷贝基本字段
    *dest = *src;

    // 分配并拷贝路由器数组
    dest->routers = (Router**)malloc(src->size_x * sizeof(Router*));

    // 处理空指针异常:routers**二级指针分配内存失败
    if (!dest->routers) {
        fprintf(file,"null pointer exception\n");
        printf("null pointer exception\n");
        free(dest);
        return NULL;
    }

    for (int i = 0; i < src->size_x; i++) {
        dest->routers[i] = (Router*)malloc(src->size_y * sizeof(Router));
        if (!dest->routers[i]) {
            fprintf(file,"null pointer exception\n");
            printf("null pointer exception\n");
            // 如果分配失败，释放已分配的内存
            for (int j = 0; j < i; j++) {
                free(dest->routers[j]);
            }
            free(dest->routers);
            free(dest);
            return NULL;
        }

        for (int j = 0; j < src->size_y; j++) {
            // 深拷贝每个路由器
            memcpy(&dest->routers[i][j], &src->routers[i][j], sizeof(Router));
        }
    }
    return dest;
}

int free_network(Network* network,int size_x,int size_y){
    for(int i = 0;i < size_x ; i++){
        if(network->routers[i]==NULL) return -1;
        free(network->routers[i]);
    }
    if(network->routers==NULL) return -1;
    free(network);
    return 0;
}