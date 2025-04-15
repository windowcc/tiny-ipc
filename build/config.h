// 版本号
#define Tinyipc_VERSION_MAJOR 
#define Tinyipc_VERSION_MINOR 

// 共享内存段最大连接数
// 如果是 @UNICAST   模式, writer 数量为 MAX_CONNECTIONS - 1, reaer 数量为 1.
// 如果是 @BROADCAST 模式, writer / reader 数量均为 MAX_CONNECTIONS / 2.
// MAX_CONNECTIONS 必须8的倍数
#define MAX_CONNECTIONS 64
