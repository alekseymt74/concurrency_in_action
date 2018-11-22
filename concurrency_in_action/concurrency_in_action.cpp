// concurrency_in_action.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#define _HAS_CXX17
// boost 1.68 msvc14.1
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

#include "pch.h"
#include <iostream>

#include <map>
#include <string>
#include <mutex>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

int main()
{
  {
    class some_big_object
    {};

//    void swap( some_big_object& lhs, some_big_object& rhs )
//    {}

    class X
    {
    private:
      some_big_object some_detail;
      mutable std::mutex m;
    public:
      X( some_big_object const& sd ) :some_detail( sd ) {}

      /*friend*/ void swap_adopt( X& lhs, X& rhs )
      {
        if( &lhs == &rhs )
          return;
        std::lock( lhs.m, rhs.m );
        std::lock_guard<std::mutex> lock_a( lhs.m, std::adopt_lock );
        std::lock_guard<std::mutex> lock_b( rhs.m, std::adopt_lock );
//        swap( lhs.some_detail, rhs.some_detail );
      }

      /*friend*/ void swap_defer( X& lhs, X& rhs )
      {
        if( &lhs == &rhs )
          return;
        std::unique_lock<std::mutex> lock_a( lhs.m, std::defer_lock );
        std::unique_lock<std::mutex> lock_b( rhs.m, std::defer_lock );
        std::lock( lock_a, lock_b );
//        swap( lhs.some_detail, rhs.some_detail );
      }

      void swap_scoped( X& lhs, X& rhs )
      {
        if( &lhs == &rhs )
          return;
        std::scoped_lock lock_ab( lhs.m, rhs.m );
//        swap( lhs.some_detail, rhs.some_detail );
      }
    };

  }

  {
    class call_once_test
    {
    private:
      std::once_flag connection_init_flag;
      void open_connection()
      {
        /*init*/
      }
    public:
      call_once_test() {}
      void send_data( const void *data )
      {
        std::call_once( connection_init_flag, &call_once_test::open_connection, this );
        /*send*/
      }
      const void *receive_data()
      {
        std::call_once( connection_init_flag, &call_once_test::open_connection, this );
        return nullptr;
      }
    };

    call_once_test _call_once;
    _call_once.send_data( nullptr );
    const void *data = _call_once.receive_data();
  }

  {
    typedef std::string dns_entry;
    class dns_cache
    {
      std::map<std::string, dns_entry> entries;
      mutable boost::shared_mutex entry_mutex;
    public:
      dns_entry find_entry( std::string const& domain ) const
      {
        boost::shared_lock<boost::shared_mutex> lk( entry_mutex );
        std::map<std::string, dns_entry>::const_iterator const it = entries.find( domain );

        return ( it == entries.end() ) ? dns_entry() : it->second;
      }
      void update_or_add_entry( std::string const& domain,
        dns_entry const& dns_details )
      {
        std::lock_guard<boost::shared_mutex> lk( entry_mutex );
        entries[ domain ] = dns_details;
      }
    };

    dns_cache dns;
    dns.find_entry( "" );
    dns.update_or_add_entry( "", "" );
  }

  return 0;
}
