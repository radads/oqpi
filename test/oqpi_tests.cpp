#include <iostream>
#include <queue>
#include <mutex>

#pragma warning (disable : 4127)
#pragma warning (disable : 4459)

#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"
 
 #include "oqpi.hpp"

#include "cqueue.hpp"
#include "timer_contexts.hpp"

using thread = oqpi::thread_interface;
using semaphore = oqpi::semaphore_interface;


template<typename T>
using cqueue = qqueue<T, std::mutex>;
volatile bool a = true;
using namespace std::chrono_literals;

using scheduler_type = oqpi::scheduler<cqueue>;

using tc = oqpi::task_context_container<timer_task_context>;
using gc = oqpi::group_context_container<timer_group_context>;

using oqpi_tk = oqpi::helpers<scheduler_type, gc, tc>;


using server = websocketpp::server<websocketpp::config::asio>;

void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg)
{
    std::cout << msg->get_payload() << std::endl;
}

static std::mutex coutMutex;
void SleepFor(int duration)
{
    duration = 10;
    {
        std::lock_guard<std::mutex> l(coutMutex);
        std::cout << "Sleeping for " << duration << "ms" << std::endl;
    }
    //oqpi::this_thread::sleep_for(std::chrono::milliseconds(duration));
    thread_local volatile int a = 10;
    a = 10;
    while (a--);
}

int main()
{

//     const auto attr = oqpi::thread_attributes{ "MyThread", (std::numeric_limits<uint32_t>::min)(), oqpi::core_affinity::all_cores, oqpi::thread_priority::highest };
//     thread th{ attr, []()
//     {
//         server print_server;
// 
//         print_server.set_message_handler(&on_message);
// 
//         print_server.init_asio();
//         print_server.listen(9002);
//         print_server.start_accept();
// 
//         print_server.run();
// 
//     }};



    const int STACK_SIZE = 0;

    const oqpi::worker_config workersConfig[] =
    {
    { oqpi::thread_attributes("WT(0) - ", STACK_SIZE, oqpi::core_affinity::core0, oqpi::thread_priority::above_normal), oqpi::worker_priority::wprio_any, 1},
    { oqpi::thread_attributes("WT(1) - ", STACK_SIZE, oqpi::core_affinity::core1, oqpi::thread_priority::above_normal), oqpi::worker_priority::wprio_any, 1},
    { oqpi::thread_attributes("WT(2) - ", STACK_SIZE, oqpi::core_affinity::core2, oqpi::thread_priority::above_normal), oqpi::worker_priority::wprio_any, 1},
    { oqpi::thread_attributes("WT(3) - ", STACK_SIZE, oqpi::core_affinity::core3, oqpi::thread_priority::above_normal), oqpi::worker_priority::wprio_any, 1},
    };
    

    oqpi_tk::scheduler_.registerWorkers<thread, semaphore>(workersConfig);
    oqpi_tk::scheduler_.start();


//     auto t1 = oqpi::make_task<tc>("MyWaitableTask", oqpi::task_priority::high, SleepFor, 500);
//     auto t2 = oqpi::make_task_item<tc>("MyFireAndForgetTask", oqpi::task_priority::high, SleepFor, 200);
// 
//     oqpi_tk::schedule_task(oqpi::task_handle(t1));
//     oqpi_tk::schedule_task(oqpi::task_handle(t2));
//     
//     auto tg = oqpi_tk::make_parallel_group<oqpi::task_type::waitable>("MyFork", oqpi::task_priority::normal, 5);
//     int32_t t = 0;
//     tg->addTask(oqpi_tk::make_task_item("MyFT1", oqpi::task_priority::normal, SleepFor, t += 10));
//     tg->addTask(oqpi_tk::make_task_item("MyFT2", oqpi::task_priority::normal, SleepFor, t += 10));
//     tg->addTask(oqpi_tk::make_task_item("MyFT3", oqpi::task_priority::normal, SleepFor, t += 10));
//     tg->addTask(oqpi_tk::make_task_item("MyFT4", oqpi::task_priority::normal, SleepFor, t += 10));
// 
//     oqpi_tk::schedule_task(oqpi::task_handle(tg));
// 

    const auto part = oqpi::simple_partitioner(40, 4);
    oqpi::parallel_for<gc, tc>(oqpi_tk::scheduler_, "ParallelFor", part, oqpi::task_priority::normal, SleepFor);

//     th.join();

    //tg->wait();

    oqpi::this_thread::sleep_for(200ms);
    oqpi_tk::scheduler_.stop();

    return 0;
}