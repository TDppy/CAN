#ifndef FUNC
#define FUNC
// function declaration

// 功能:将方向的值转化为字符串
// 输入: int direction
// 输出: const char* 例如"LCOAL"的地址 
const char* direction_to_string(int direction);

// 功能:初始化虚通道，将head,tail,count赋值为0 
// 输入:VirtualChannel* vc 
void initialize_virtual_channel(VirtualChannel* vc);

// 初始化路由器
// 输入:路由器指针、id、横纵坐标 
void initialize_router(Router* router, int router_id, int x, int y,int num_vcs);

// 初始化网络
// 输入:Network* network,横轴长度，纵轴长度 
void initialize_network(Network* network, SimParams* params);

// 计算路由方向
// 输入:Router* cur_router,要计算输出端口的flit,纵轴长度 
int calculate_routing(Router* cur_router, Flit* flit, int size_y,SimParams* params,
                      Network* last_network,int cur_cycle,NoC_congestion_matrix* matrix);

// 获取相反方向
// 输入:当前路由器的目标输出端口值 
// 返回值：下一跳路由器的输入端口值 
int opposite_dir(int outdir);

// 虚通道分配 
// 输入:目标router,申请虚通道的方向  
// 分配原则：按顺序查看该方向的几个虚通道是否有空间，有就返回vc_id 
// 返回值：被分配的vc_id 
int allocate_virtual_channel(Router* router_ptr , int in_dir,SimParams* params);

// 在vc中增加一个flit
// 输入:VirtualChannel* vc_ptr,Flit* flit_ptr 
// 返回值:成功返回1，失败返回-1 
// 注意：由于vc的内存是网络初始化时候就分配好的，这里只需要把flit的值复制到vc的队尾
//       因此，要记得释放flit_ptr的内存 
int vc_push_flit(VirtualChannel* vc_ptr , Flit* flit_ptr , int buffer_size);

// 在vc中删除第一个flit
// 输入:VirtualChannel* vc_ptr
// 返回值:成功返回1，失败返回-1 
int vc_pop_flit(VirtualChannel* vc_ptr , int buffer_size);

// 深拷贝网络
// 输入:const Network* src
// 返回值:Network* copied_network 
// const Network* 这个指针所指向的内容不能被修改,即拷贝过程中源内容是只读的 
// 注意：这里分配了copied network的内存，在使用完成后，应当释放该内存 
Network* deep_copy_network(const Network* src);

// 传入当前router坐标，获取下一跳router坐标 
void get_next_router_position(int* x,int* y,int out_dir);

SelectedFlit* compare_by_qos(SelectedFlit* flit_list, int num);

SelectedFlit* compare_by_time(SelectedFlit* flit_list, int num);

SelectedFlit* find_flit_for_out_dir(Router* router,SimParams* params,int out_dir,Network* last_network,NoC_congestion_matrix* matrix,int cur_cycle);

void router_transfer(Network* last_network,Network* cur_network,Router* last_router,SimParams* params,NoC_congestion_matrix* matrix);

int traffic_pattern(int source,SimParams* params,int pattern);

int traffic_injection(Network* network , int cur_cycle , double inj_rate , SimParams* params);

//free_network只能free由malloc分配的，因为free函数只适用于堆上分配的内存
int free_network(Network* network,int size_x,int size_y);

void simulate(Network* cur_network , SimParams* params , double inj_rate , NoC_congestion_matrix* free_matrix);

void print_metrics(Network network);

SimParams* setup_sim_params(const char* filename,SimParams* params);

void print_sim_params(SimParams* params);

int is_output_port_available(Router* router, int output_direction, int size_x, int size_y);

/**
 * 功能：打印整个网络中所有路由器的虚通道（VC）占用情况
 * 输入：
 *   - network：指向当前网络的指针（包含所有路由器）
 *   - params：指向仿真参数的指针（需获取 buffer_size/num_vcs 等配置）
 *   - cur_cycle：当前仿真周期（用于标注日志上下文）
 */
void print_vc_occupancy(Network* network, SimParams* params, int cur_cycle);

int getBitCount(unsigned int size);

unsigned int reverseBinary(unsigned int src, int bitCount, unsigned int size);

void initialize_congestion_matrix(NoC_congestion_matrix* matrix , SimParams* params);

void initialize_scheduler_table(SimParams* params ,InterConnectSchedulerTable* scheduler_table);

int calculate_min_conges_port(int cur_cycle,SimParams* params,NoC_congestion_matrix* congestion_matrix,Coordinate cur,Coordinate dest);

void south_west_dp(SimParams* params, NoC_congestion_matrix* congestion_matrix, InterConnectSchedulerTable* scheduler_table);

void update_weight_matrix(Network* cur_network,int cur_cycle,SimParams* params,NoC_congestion_matrix* congestion_matrix);

int greedy_routing(Router** router,Coordinate cur,Coordinate dest);

int DoR_routing(Coordinate cur,Coordinate dest);
#endif