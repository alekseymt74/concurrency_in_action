#ifndef MEMORY_ORDERING_H
#define MEMORY_ORDERING_H

#include <atomic>
#include <assert.h>
#include <thread>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <time.h>
#include <chrono>

namespace modif_order
{

  void w_w_coherence()
  {
    bool A = true;
    std::atomic< bool > M = A;
    assert( M == true );
    bool B = false;
    M = B;
    assert( M == false );
  }

  typedef std::pair< int, std::chrono::system_clock::rep > reoders_tm;

  template< std::memory_order A_Store_Order = std::memory_order_seq_cst
    , std::memory_order A_Load_Order = std::memory_order_seq_cst
    , std::memory_order B_Store_Order = std::memory_order_seq_cst
    , std::memory_order B_Load_Order = std::memory_order_seq_cst
    , std::memory_order A1_Store_Order = std::memory_order_seq_cst
    , std::memory_order A1_Load_Order = std::memory_order_seq_cst
    , std::memory_order B1_Store_Order = std::memory_order_seq_cst
    , std::memory_order B1_Load_Order = std::memory_order_seq_cst >
    reoders_tm reordering_test()
  {
    std::chrono::system_clock::time_point tm_start = std::chrono::system_clock::now();
    std::atomic< int > A = 0;
    std::atomic< int > B = 0;
    std::atomic< int > A1 = 0;
    std::atomic< int > B1 = 0;
    std::atomic< bool > barrier_A = false;
    std::atomic< bool > barrier_B = false;
    constexpr int num_threads = 2;
    //std::atomic< bool > start_1 = false;
    //std::atomic< bool > start_2 = false;
    //std::atomic< int > num_done = 0;
    bool start_1 = false;
    bool start_2 = false;
    int num_done = 0;
    std::condition_variable cond_start_1;
    std::condition_variable cond_start_2;
    std::mutex mtx_start_1;
    std::mutex mtx_start_2;
    std::condition_variable cond_end;
    std::mutex mtx_end;
    std::atomic_bool active = true;
    std::thread thr_A([&]()
    {
      while (active.load(std::memory_order_relaxed))
      {
        {
          std::unique_lock< std::mutex > lock_start(mtx_start_1);
          cond_start_1.wait(lock_start, [&]()->bool { return start_1; });
          start_1 = false;
        }
        //for (int i = 0; i < rand() % 100; ++i)
        //  ;
        A.store(1, A_Store_Order);

        barrier_A.store(true, std::memory_order_seq_cst);
        bool _b = barrier_B.load(std::memory_order_seq_cst);

        B1.store(B.load(B_Load_Order), B1_Store_Order);
        std::unique_lock< std::mutex > lock_end(mtx_end);
        ++num_done;
        cond_end.notify_one();
      }
    });
    std::thread thr_B([&]()
    {
      while (active.load(std::memory_order_relaxed))
      {
        {
          std::unique_lock< std::mutex > lock_start(mtx_start_2);
          cond_start_2.wait(lock_start, [&]()->bool { return start_2; });
          start_2 = false;
        }
        //for (int i = 0; i < rand() % 100; ++i)
        //  ;
        B.store(1, B_Store_Order);

        barrier_B.store(true, std::memory_order_seq_cst);
        bool _b = barrier_A.load(std::memory_order_seq_cst);

        A1.store(A.load(A_Load_Order), A1_Store_Order);
        std::unique_lock< std::mutex > lock_end(mtx_end);
        ++num_done;
        cond_end.notify_one();
      }
    });
    //srand(static_cast<unsigned int>(time(NULL)));
    constexpr int num_iters = 100000;
    int num_reorders = 0;
    for (int i = 0; i < num_iters; ++i)
    {
      if (i == num_iters - 1)
        active.store(false, std::memory_order_relaxed);;
      A.store(0, A_Store_Order);
      B.store(0, B_Store_Order);
      {
        std::unique_lock< std::mutex > lock_start(mtx_start_1);
        start_1 = true;
        cond_start_1.notify_one();
      }
      {
        std::unique_lock< std::mutex > lock_start(mtx_start_2);
        start_2 = true;
        cond_start_2.notify_one();
      }
      std::unique_lock< std::mutex > lock_end(mtx_end);
      cond_end.wait(lock_end, [&]() { return (num_done == num_threads); });
      num_done = 0;
      if (A1.load(A1_Load_Order) == 0 && B1.load(B1_Load_Order) == 0)
        ++num_reorders;// std::cout << "reorder detected in iteration #" << i << std::endl;
    }
    thr_A.join();
    thr_B.join();

    std::chrono::system_clock::rep tm_ellapsed = std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::system_clock::now() - tm_start ).count();

    return std::make_pair( num_reorders, tm_ellapsed );
  }

} // modification_order

#endif // MEMORY_ORDERING_H
