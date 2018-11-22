// concurrency_in_action.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <mutex>

int main()
{
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

  return 0;
}
