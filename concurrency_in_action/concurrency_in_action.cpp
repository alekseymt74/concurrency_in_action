// concurrency_in_action.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <set>
#include <thread>
#include <atomic>
#include <numeric>

#include "tests.h"
#include "mult_sep_w_single_r\mult_sep_w_single_r.h"
#include "memory_ordering.h"

int main()
{
  //memory ordering
  {
    if (false)
      modif_order::w_w_coherence();
    for (int i = 0; i < 5; ++i)
    {
      modif_order::reoders_tm rep = modif_order::reordering_test< std::memory_order_relaxed, std::memory_order_relaxed
        , std::memory_order_relaxed, std::memory_order_relaxed
        , std::memory_order_relaxed, std::memory_order_relaxed
        , std::memory_order_relaxed, std::memory_order_relaxed >();
      std::cout << "num reorders: " << rep.first << "\ttm: " << rep.second << std::endl;
      rep = modif_order::reordering_test< std::memory_order_release, std::memory_order_acquire
        , std::memory_order_release, std::memory_order_acquire
        , std::memory_order_seq_cst, std::memory_order_seq_cst
        , std::memory_order_seq_cst, std::memory_order_seq_cst >();
      std::cout << "num reorders: " << rep.first << "\ttm: " << rep.second << std::endl;
      rep = modif_order::reordering_test< std::memory_order_seq_cst, std::memory_order_seq_cst
        , std::memory_order_seq_cst, std::memory_order_seq_cst
        , std::memory_order_seq_cst, std::memory_order_seq_cst
        , std::memory_order_seq_cst, std::memory_order_seq_cst >();
      std::cout << "num reorders: " << rep.first << "\ttm: " << rep.second << std::endl;
      assert(rep.first == 0);
      rep = modif_order::reordering_test< std::memory_order_seq_cst, std::memory_order_seq_cst
        , std::memory_order_seq_cst, std::memory_order_seq_cst
        , std::memory_order_relaxed, std::memory_order_relaxed
        , std::memory_order_relaxed, std::memory_order_relaxed >();
      std::cout << "num reorders: " << rep.first << "\ttm: " << rep.second << std::endl;
      assert(rep.first == 0);
      std::cout << "#####################" << std::endl;
    }
  }

  {
    some_big_object sbo1, sbo2;
    X x1( sbo1 ), x2( sbo2 );
    swap_adopt( x1, x2 );
    swap_defer( x1, x2 );
    swap_scoped( x1, x2 );
  }

  {

    call_once_test _call_once;
    _call_once.send_data( nullptr );
    const void *data = _call_once.receive_data();
  }

  {
    dns_cache dns;
    dns_entry entry = dns.find_entry( "" );
    dns.update_or_add_entry( "", "" );
  }

  // multiple separated (non-conflict) writers / single reader(processor)
  {
    typedef std::set< sub_ptr > subs_cont;
    typedef subs_cont::iterator it_sub;
    subs_cont subs;
    os_w_ptr os_w = std::make_shared< os_writer >();
    for( int i = 0; i < 33; ++i )
    {
      uint32_t sz = 4;
      sub_ptr sub = os_w->push_elem( sz );
      subs.emplace( std::move( sub ) );
    }
    uint8_t buf[ 8 ];
    std::atomic_bool active = true;
    std::thread thr_update_1( [ & ]()
    {
      while( active )
      {
        for( auto &sub : subs )
        {
          sub->update(&buf, sub->get_size());
          std::this_thread::sleep_for( std::chrono::microseconds( 50 ) );
        }
      }
    });
    std::thread thr_update_2( [ & ]()
    {
      while( active )
      {
        for (auto &sub : subs)
        {
          sub->update( &buf, sub->get_size() );
          std::this_thread::sleep_for( std::chrono::microseconds( 75 ) );
        }
      }
    } );
    std::thread thr_process( [ & ]()
    {
      while( active )
      {
        os_w->process();
        std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
      }
    } );
    std::this_thread::sleep_for( std::chrono::seconds( 5 ) );
    active = false;
    thr_update_1.join();
    thr_update_2.join();
    thr_process.join();
  }

  return 0;
}
