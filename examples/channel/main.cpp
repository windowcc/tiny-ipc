
#include <signal.h>
#include <string.h>
#include <iostream>
#include <atomic>
#include <thread>
#include <ipc/ipc.h>

namespace {

std::atomic<bool> IsQuit { false };
ipc::Channel *Ipc = nullptr;
uint16_t Count = 0;

void do_send()
{
    ipc::Channel ipc {"channel", ipc::SENDER};
    Ipc = &ipc;
    const std::string buffer1 {"Hello,World!!!"};
    const std::string buffer2 {"nice to meet you."};
    while (!IsQuit.load(std::memory_order_acquire))
    {
        if(Count++ >= 1000)
        {
            break;
        }
        std::cout << "Write " << Count << " : " << (Count%2 == 1 ? buffer1 : buffer2) << "\n";
        ipc.write(std::to_string(Count) + " : " + (Count%2 == 1 ? buffer1 : buffer2));
        // std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

class SendCallback : public ipc::Callback
{
public:
    virtual void message_arrived(const ipc::Buffer *buf /*msg*/, const ipc::ErrorCode &cause) final{
        std::cout << "Read " << std::string((char*)buf->data(),buf->size()) << "\n";
    }
private:
    uint32_t cnt_ {0};
};

void do_recv()
{
    std::cout << "Please Enter Ctrl + C to End." << std::endl;
    ipc::Channel ipc {"channel", ipc::RECEIVER};
    ipc.set_callback(
        std::dynamic_pointer_cast<ipc::Callback>(std::make_shared<SendCallback>()));
    Ipc = &ipc;
    ipc.read();
}

} // namespace

int main(int argc, char ** argv)
{
    if (argc < 2)
    {
        return -1;
    }
    auto exit = [](int)
    {
        IsQuit.store(true, std::memory_order_release);
        if (Ipc)
        {
            Ipc->disconnect();
        }
    };
    ::signal(SIGINT  , exit);
    ::signal(SIGABRT , exit);
    ::signal(SIGSEGV , exit);
    ::signal(SIGTERM , exit);
#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || \
    defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || \
    defined(WINCE) || defined(_WIN32_WCE)
    ::signal(SIGBREAK, exit);
#else
    ::signal(SIGHUP  , exit);
#endif

    std::string mode {argv[1]};
    if (mode == "send")
    {
        do_send();
    }
    else if (mode == "recv")
    {
        do_recv();
    }
    return 0;
}

