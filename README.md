仿真器使用纯C语言描述。 
拓扑目前仅支持2D mesh，默认1 packet = 1 flit，每周期每个router输出端口最多可以输出1个flit。
流量模式支持常见的合成流量，如uniform、transpose、tornado。
数据从core发送到NI打包，然后转发到Router，再由Router网络经过若干跳转发到目标Router，最后经过目标NI解包到目标core。
在仿真器中，主要仿真Router网络的行为，NI和Router在仿真器中不做实现。流量由仿真器根据注入率生成，注入到Router的本地虚通道中。注入率指的是每周期每个Router注入一个Flit的概率。
关于指标计算：
flit注入到网络中后，每经过一跳，链路总使用次数就会被累加，用于后续计算链路利用率，例如20%。经过若干跳转发，达到目标Router，当目标router检测该flit目标输出端口是LOCAL时，就计算该flit的端到端时延=当前周期-flit的数据注入周期，例如20cycles。此外，吞吐量的单位是flits/cycles，计算方式是网络在一次仿真期间总接收的flits数量除以总的周期数，例如设定一次仿真500cycles，一共接收了200flits，则吞吐量是200flits/500cycles=0.4flit/cycle。

NoC仿真器目录:
	cmake-build-debug/   存放配置参数、统计的指标文件
		config.txt
		noc_results_qos.txt
			
	src/       存放源码，包括主函数、工具类、头文件等
	data_structrue.h
	fuc_declaration.h
	initial_utils.cpp
	io_utils.cpp
	main.cpp
	memory_oper.cpp
	router.cpp
	traffic_oper.cpp

	exe/       存放可执行文件
	config/    存放配置信息
	config.txt
	results/    存放仿真结果，包含各类指标和日志文件
	metrics.txt
	log.txt
