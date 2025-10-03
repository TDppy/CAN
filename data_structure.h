#ifndef STUC
#define STUC
#define NUM_DIRECTIONS 5
#define MAX_BUFFER_SIZE 16
#define MAX_NUM_VCS 8
#define MAX_PATH_LENGTH 100
#define EAST 1
#define SOUTH 2
#define WEST 3
#define NORTH 4
#define LOCAL 0
#define QOS_TYPE_NUM 5
// Flit
typedef struct {
    int source;//src router , router_x_id = source / y_width router_y_id = source % y_width
    int destination;
    int flit_id;
    int cycle_injected;
    int qos;
    int type;
    int path[105];//存储经过的所有router id
    int hops; // 存储经过了多少跳
} Flit;

// SelectedFlits是当前路由器中选中的Flit

typedef struct{
    Flit flit;
    int vc_id;
    int in_dir;
} SelectedFlit;

// VirtualChannel
typedef struct {
    Flit buffer[MAX_BUFFER_SIZE];
    int head;
    int tail;
    int count;
} VirtualChannel;

// Router
typedef struct {
    int router_id;
    VirtualChannel vcs[NUM_DIRECTIONS][MAX_NUM_VCS];
    int x, y;
} Router;

// Network
typedef struct {
    Router** routers;
    int size_x;
    int size_y;
    int cur_cycles;
    int total_flits_injected;
    int total_flits_received;
    int total_latency;
    int total_link_utilization;
    int total_latency_with_type[QOS_TYPE_NUM];
    int total_flits_received_with_type[QOS_TYPE_NUM];
    // 达到max_num时的周期
    int max_num_cycles;
} Network;

// Simulation Params
typedef struct{
    double inj_rate_start ;
    double inj_rate_end   ;
    double inj_rate_step  ;
    int size_x         	  ;
    int size_y            ;
    int buffer_size       ;
    int total_cycles      ;
    int max_flits_num     ;
    int num_vcs           ;
    int traffic_pattern   ;
    int prior_strategy    ;
    int routing_algorithm ;
}SimParams;

extern FILE *file;
#endif
