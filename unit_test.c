// #define TEST
#include "data_structure.h"
#include "func_declaration.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 测试 direction_to_string 函数
void test_direction_to_string() {
    fprintf(file,"Testing direction_to_string...\n");
    const char* result;

    // 测试样例：LOCAL
    fprintf(file,"Testing LOCAL: Input = LOCAL, Expected = 'LOCAL'\n");
    result = direction_to_string(LOCAL);
    fprintf(file,"Result = '%s'\n", result);
    if (strcmp(result, "LOCAL") != 0) {
        fprintf(file,"Test failed for LOCAL.\n");
    }

    // 测试样例：EAST
    fprintf(file,"Testing EAST: Input = EAST, Expected = 'EAST'\n");
    result = direction_to_string(EAST);
    fprintf(file,"Result = '%s'\n", result);
    if (strcmp(result, "EAST") != 0) {
        fprintf(file,"Test failed for EAST.\n");
    }

    // 测试样例：SOUTH
    fprintf(file,"Testing SOUTH: Input = SOUTH, Expected = 'SOUTH'\n");
    result = direction_to_string(SOUTH);
    fprintf(file,"Result = '%s'\n", result);
    if (strcmp(result, "SOUTH") != 0) {
        fprintf(file,"Test failed for SOUTH.\n");
    }

    // 测试样例：WEST
    fprintf(file,"Testing WEST: Input = WEST, Expected = 'WEST'\n");
    result = direction_to_string(WEST);
    fprintf(file,"Result = '%s'\n", result);
    if (strcmp(result, "WEST") != 0) {
        fprintf(file,"Test failed for WEST.\n");
    }

    // 测试样例：NORTH
    fprintf(file,"Testing NORTH: Input = NORTH, Expected = 'NORTH'\n");
    result = direction_to_string(NORTH);
    fprintf(file,"Result = '%s'\n", result);
    if (strcmp(result, "NORTH") != 0) {
        fprintf(file,"Test failed for NORTH.\n");
    }

    // 测试样例：未知方向
    fprintf(file,"Testing unknown direction: Input = -1, Expected = 'UNKNOWN'\n");
    result = direction_to_string(-1);
    fprintf(file,"Result = '%s'\n", result);
    if (strcmp(result, "UNKNOWN") != 0) {
        fprintf(file,"Test failed for unknown direction.\n");
    }

    fprintf(file,"direction_to_string test completed.\n");
}

// 测试 is_output_port_available 函数
void test_is_output_port_available() {
    fprintf(file,"Testing is_output_port_available...\n");
    Router router;
    router.x = 1;
    router.y = 1;
    int size_x = 3;
    int size_y = 3;
    int result;

    // 测试样例：EAST
    fprintf(file,"Testing EAST: Router(x = 1, y = 1), Input = EAST, Expected = 1\n");
    result = is_output_port_available(&router, EAST, size_x, size_y);
    fprintf(file,"Result = %d\n", result);
    if (result != 1) {
        fprintf(file,"Test failed for EAST.\n");
    }

    // 测试样例：SOUTH
    fprintf(file,"Testing SOUTH: Router(x = 1, y = 1), Input = SOUTH, Expected = 1\n");
    result = is_output_port_available(&router, SOUTH, size_x, size_y);
    fprintf(file,"Result = %d\n", result);
    if (result != 1) {
        fprintf(file,"Test failed for SOUTH.\n");
    }

    // 测试样例：WEST
    fprintf(file,"Testing WEST: Router(x = 1, y = 1), Input = WEST, Expected = 1\n");
    result = is_output_port_available(&router, WEST, size_x, size_y);
    fprintf(file,"Result = %d\n", result);
    if (result != 1) {
        fprintf(file,"Test failed for WEST.\n");
    }

    // 测试样例：NORTH
    fprintf(file,"Testing NORTH: Router(x = 1, y = 1), Input = NORTH, Expected = 1\n");
    result = is_output_port_available(&router, NORTH, size_x, size_y);
    fprintf(file,"Result = %d\n", result);
    if (result != 1) {
        fprintf(file,"Test failed for NORTH.\n");
    }

    // 测试样例：LOCAL
    fprintf(file,"Testing LOCAL: Router(x = 1, y = 1), Input = LOCAL, Expected = 1\n");
    result = is_output_port_available(&router, LOCAL, size_x, size_y);
    fprintf(file,"Result = %d\n", result);
    if (result != 1) {
        fprintf(file,"Test failed for LOCAL.\n");
    }

    // 测试样例：越界的 WEST
    router.x = 0;
    fprintf(file,"Testing out-of-bounds WEST: Router(x = 0, y = 1), Input = WEST, Expected = 0\n");
    result = is_output_port_available(&router, WEST, size_x, size_y);
    fprintf(file,"Result = %d\n", result);
    if (result != 0) {
        fprintf(file,"Test failed for out-of-bounds WEST.\n");
    }

    fprintf(file,"is_output_port_available test completed.\n");
}

// 测试 opposite_dir 函数
void test_opposite_dir() {
    fprintf(file,"Testing opposite_dir...\n");
    int result;

    // 测试样例：EAST
    fprintf(file,"Testing EAST: Input = EAST, Expected = WEST (%d)\n", WEST);
    result = opposite_dir(EAST);
    fprintf(file,"Result = %d\n", result);
    if (result != WEST) {
        fprintf(file,"Test failed for EAST.\n");
    }

    // 测试样例：WEST
    fprintf(file,"Testing WEST: Input = WEST, Expected = EAST (%d)\n", EAST);
    result = opposite_dir(WEST);
    fprintf(file,"Result = %d\n", result);
    if (result != EAST) {
        fprintf(file,"Test failed for WEST.\n");
    }

    // 测试样例：SOUTH
    fprintf(file,"Testing SOUTH: Input = SOUTH, Expected = NORTH (%d)\n", NORTH);
    result = opposite_dir(SOUTH);
    fprintf(file,"Result = %d\n", result);
    if (result != NORTH) {
        fprintf(file,"Test failed for SOUTH.\n");
    }

    // 测试样例：NORTH
    fprintf(file,"Testing NORTH: Input = NORTH, Expected = SOUTH (%d)\n", SOUTH);
    result = opposite_dir(NORTH);
    fprintf(file,"Result = %d\n", result);
    if (result != SOUTH) {
        fprintf(file,"Test failed for NORTH.\n");
    }

    // 测试样例：LOCAL
    fprintf(file,"Testing LOCAL: Input = LOCAL, Expected = LOCAL (%d)\n", LOCAL);
    result = opposite_dir(LOCAL);
    fprintf(file,"Result = %d\n", result);
    if (result != LOCAL) {
        fprintf(file,"Test failed for LOCAL.\n");
    }

    // 测试样例：未知方向
    fprintf(file,"Testing unknown direction: Input = -1, Expected = -1\n");
    result = opposite_dir(-1);
    fprintf(file,"Result = %d\n", result);
    if (result != -1) {
        fprintf(file,"Test failed for unknown direction.\n");
    }

    fprintf(file,"opposite_dir test completed.\n");
}

// 测试 allocate_virtual_channel 函数
void test_allocate_virtual_channel() {
    fprintf(file,"Testing allocate_virtual_channel...\n");
    Router router;
    SimParams params;
    params.num_vcs = 2;
    params.buffer_size = 2;
    int result;

    // 初始化虚通道
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        for (int j = 0; j < params.num_vcs; j++) {
            initialize_virtual_channel(&router.vcs[i][j]);
        }
    }

    // 测试样例：有可用虚通道
    fprintf(file,"Testing available VC: Input = LOCAL, Expected = 0\n");
    result = allocate_virtual_channel(&router, LOCAL, &params);
    fprintf(file,"Result = %d\n", result);
    if (result != 0) {
        fprintf(file,"Test failed for available VC.\n");
    }

    // 填满所有虚通道
    for (int j = 0; j < params.num_vcs; j++) {
        VirtualChannel* vc = &router.vcs[LOCAL][j];
        for (int k = 0; k < params.buffer_size; k++) {
            Flit flit;
            vc_push_flit(vc, &flit, params.buffer_size);
        }
    }

    // 测试样例：无可用虚通道
    fprintf(file,"Testing no available VC: Input = LOCAL, Expected = -1\n");
    result = allocate_virtual_channel(&router, LOCAL, &params);
    fprintf(file,"Result = %d\n", result);
    if (result != -1) {
        fprintf(file,"Test failed for no available VC.\n");
    }

    fprintf(file,"allocate_virtual_channel test completed.\n");
}

// 测试 vc_push_flit 函数
void test_vc_push_flit() {
    fprintf(file,"Testing vc_push_flit...\n");
    VirtualChannel vc;
    initialize_virtual_channel(&vc);
    int buffer_size = 2;
    Flit flit;
    int result;

    // 测试样例：向可用虚通道推送 flit
    fprintf(file,"Testing pushing flit to available VC: Buffer size = 2, Expected = 1\n");
    result = vc_push_flit(&vc, &flit, buffer_size);
    fprintf(file,"Result = %d\n", result);
    if (result != 1) {
        fprintf(file,"Test failed for pushing flit to available VC.\n");
    }

    // 填满 VC
    for (int i = 0; i < buffer_size - 1; i++) {
        vc_push_flit(&vc, &flit, buffer_size);
    }

    // 测试样例：向满的虚通道推送 flit
    fprintf(file,"Testing pushing flit to full VC: Buffer size = 2, Expected = -1\n");
    result = vc_push_flit(&vc, &flit, buffer_size);
    fprintf(file,"Result = %d\n", result);
    if (result != -1) {
        fprintf(file,"Test failed for pushing flit to full VC.\n");
    }

    fprintf(file,"vc_push_flit test completed.\n");
}

// 测试 vc_pop_flit 函数
void test_vc_pop_flit() {
    fprintf(file,"Testing vc_pop_flit...\n");
    VirtualChannel vc;
    initialize_virtual_channel(&vc);
    int buffer_size = 2;
    Flit flit;
    int result;

    // 测试样例：从空的虚通道弹出 flit
    fprintf(file,"Testing popping from empty VC: Buffer size = 2, Expected = -1\n");
    result = vc_pop_flit(&vc, buffer_size);
    fprintf(file,"Result = %d\n", result);
    if (result != -1) {
        fprintf(file,"Test failed for popping from empty VC.\n");
    }

    vc_push_flit(&vc, &flit, buffer_size);

    // 测试样例：从非空的虚通道弹出 flit
    fprintf(file,"Testing popping from non-empty VC: Buffer size = 2, Expected = 1\n");
    result = vc_pop_flit(&vc, buffer_size);
    fprintf(file,"Result = %d\n", result);
    if (result != 1) {
        fprintf(file,"Test failed for popping from non-empty VC.\n");
    }

    fprintf(file,"vc_pop_flit test completed.\n");
}

// 测试 get_next_router_position 函数
void test_get_next_router_position() {
    fprintf(file,"Testing get_next_router_position...\n");
    int x = 1;
    int y = 1;

    // 测试样例：EAST
    fprintf(file,"Testing EAST: Input(x = 1, y = 1), Expected(x = 2, y = 1)\n");
    get_next_router_position(&x, &y, EAST);
    fprintf(file,"Result(x = %d, y = %d)\n", x, y);
    if (x != 2 || y != 1) {
        fprintf(file,"Test failed for EAST.\n");
    }

    x = 1;
    y = 1;
    // 测试样例：SOUTH
    fprintf(file,"Testing SOUTH: Input(x = 1, y = 1), Expected(x = 1, y = 0)\n");
    get_next_router_position(&x, &y, SOUTH);
    fprintf(file,"Result(x = %d, y = %d)\n", x, y);
    if (x != 1 || y != 0) {
        fprintf(file,"Test failed for SOUTH.\n");
    }

    x = 1;
    y = 1;
    // 测试样例：WEST
    fprintf(file,"Testing WEST: Input(x = 1, y = 1), Expected(x = 0, y = 1)\n");
    get_next_router_position(&x, &y, WEST);
    fprintf(file,"Result(x = %d, y = %d)\n", x, y);
    if (x != 0 || y != 1) {
        fprintf(file,"Test failed for WEST.\n");
    }

    x = 1;
    y = 1;
    // 测试样例：NORTH
    fprintf(file,"Testing NORTH: Input(x = 1, y = 1), Expected(x = 1, y = 2)\n");
    get_next_router_position(&x, &y, NORTH);
    fprintf(file,"Result(x = %d, y = %d)\n", x, y);
    if (x != 1 || y != 2) {
        fprintf(file,"Test failed for NORTH.\n");
    }

    fprintf(file,"get_next_router_position test completed.\n");
}

// 测试 compare_by_qos 函数
void test_compare_by_qos() {
    fprintf(file,"Testing compare_by_qos...\n");
    SelectedFlit flit_list[3];
    flit_list[0].flit.qos = 3;
    flit_list[1].flit.qos = 1;
    flit_list[2].flit.qos = 2;

    // 测试样例
    fprintf(file,"Testing compare_by_qos: Input QoS values = [3, 1, 2], Expected QoS = 1\n");
    SelectedFlit* result = compare_by_qos(flit_list, 3);
    fprintf(file,"Result QoS = %d\n", result->flit.qos);
    if (result->flit.qos != 1) {
        fprintf(file,"Test failed for compare_by_qos.\n");
    }
    fprintf(file,"compare_by_qos test completed.\n");
}

// 测试 compare_by_time 函数
void test_compare_by_time() {
    fprintf(file,"Testing compare_by_time...\n");
    SelectedFlit flit_list[3];
    flit_list[0].flit.cycle_injected = 3;
    flit_list[1].flit.cycle_injected = 1;
    flit_list[2].flit.cycle_injected = 2;

    // 测试样例
    fprintf(file,"Testing compare_by_time: Input cycle_injected values = [3, 1, 2], Expected cycle_injected = 1\n");
    SelectedFlit* result = compare_by_time(flit_list, 3);
    fprintf(file,"Result cycle_injected = %d\n", result->flit.cycle_injected);
    if (result->flit.cycle_injected != 1) {
        fprintf(file,"Test failed for compare_by_time.\n");
    }
    fprintf(file,"compare_by_time test completed.\n");
}

// 测试 find_flit_for_out_dir 函数
void test_find_flit_for_out_dir() {
    fprintf(file,"Testing find_flit_for_out_dir...\n");
    Router router;
    SimParams params;
    params.size_x = 3;
    params.size_y = 3;
    params.num_vcs = 2;
    params.buffer_size = 4;
    int out_dir = EAST;

    // 初始化路由器和虚通道
    initialize_router(&router, 0, 1, 1, params.num_vcs);

    // 添加多个 flit 到虚通道，这些 flit 目标方向都为 EAST
    Flit flit1, flit2, flit3;
    flit1.source = 0;
    flit1.destination = 2 * params.size_y + 1; // 目标在 EAST 方向
    flit1.cycle_injected = 1;
    flit1.qos = 3;

    flit2.source = 0;
    flit2.destination = 2 * params.size_y + 1; // 目标在 EAST 方向
    flit2.cycle_injected = 2;
    flit2.qos = 1;

    flit3.source = 0;
    flit3.destination = 2 * params.size_y + 1; // 目标在 EAST 方向
    flit3.cycle_injected = 3;
    flit3.qos = 2;

    VirtualChannel* vc1 = &router.vcs[LOCAL][0];
    VirtualChannel* vc2 = &router.vcs[LOCAL][1];
    VirtualChannel* vc3 = &router.vcs[WEST][1];
    vc_push_flit(vc1, &flit1, params.buffer_size);
    vc_push_flit(vc2, &flit2, params.buffer_size);
    vc_push_flit(vc3, &flit3, params.buffer_size);

    // 测试按照注入时间选取
    params.prior_strategy = 0;
    SelectedFlit* result_time = find_flit_for_out_dir(&router, &params, out_dir);
    if (result_time == NULL || result_time->flit.cycle_injected != 1) {
        fprintf(file,"Test failed for find_flit_for_out_dir with time priority.\n");
    } else {
        fprintf(file,"flit1 is selected by injected time compare function!\n");
        //free(result_time);
    }

    // 测试按照 QoS 选取
    params.prior_strategy = 1;
    SelectedFlit* result_qos = find_flit_for_out_dir(&router, &params, out_dir);
    if (result_qos == NULL || result_qos->flit.qos != 1) {
        fprintf(file,"Test failed for find_flit_for_out_dir with QoS priority.\n");
    } else {
        fprintf(file,"flit1 is selected by qos compare function!\n");
        //free(result_qos);
    }

    fprintf(file,"find_flit_for_out_dir test completed.\n");
}

// 测试 router_transfer 函数
void test_router_transfer() {
    fprintf(file,"Testing router_transfer...\n");
    Network last_network;
    Network cur_network;
    SimParams params;
    params.size_x = 3;
    params.size_y = 3;
    params.num_vcs = 2;
    params.buffer_size = 2;
    params.total_cycles = 10;
    params.max_flits_num = 100;
    params.traffic_pattern = 1;
    params.prior_strategy = 0;

    initialize_network(&last_network, &params);
    initialize_network(&cur_network, &params);

    Router* last_router = &last_network.routers[1][1];

    // 添加一些 flit 到虚通道
    Flit flit;
    flit.source = 0;
    flit.destination = 2 * params.size_y + 1; // 目标在 EAST 方向
    flit.cycle_injected = 1;
    flit.qos = 1;
    VirtualChannel* vc = &last_router->vcs[LOCAL][0];
    vc_push_flit(vc, &flit, params.buffer_size);

    router_transfer(&last_network, &cur_network, last_router, &params);

    //free_network(&last_network, params.size_x, params.size_y);
    //free_network(&cur_network, params.size_x, params.size_y);

    fprintf(file,"router_transfer test completed.\n");
}

// 测试 simulate 函数
void test_simulate() {
    fprintf(file,"Testing simulate...\n");
    Network cur_network;
    SimParams params;
    params.size_x = 3;
    params.size_y = 3;
    params.num_vcs = 2;
    params.buffer_size = 2;
    params.total_cycles = 200;
    params.max_flits_num = 500;
    params.traffic_pattern = 1;
    params.prior_strategy = 0;
    params.inj_rate_start = 0.1;
    params.inj_rate_end = 0.5;
    params.inj_rate_step = 0.1;

    initialize_network(&cur_network, &params);

    double inj_rate = 0.5;
    simulate(&cur_network, &params, inj_rate);

    //free_network(&cur_network, params.size_x, params.size_y);
    print_metrics(cur_network);
    fprintf(file,"simulate test completed.\n");
}

int main() {
    test_direction_to_string();
    test_is_output_port_available();
    test_opposite_dir();
    test_allocate_virtual_channel();
    test_vc_push_flit();
    test_vc_pop_flit();
    test_get_next_router_position();
    test_compare_by_qos();
    test_compare_by_time();
    test_find_flit_for_out_dir();
    test_router_transfer();
    test_simulate();

    return 0;
}