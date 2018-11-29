// concurrency_in_action.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#define _HAS_CXX17
// boost 1.68 msvc14.1
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

#include "pch.h"
#include <iostream>

#include "tests.h"

int main()
{
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
    dns.find_entry( "" );
    dns.update_or_add_entry( "", "" );
  }

  return 0;
}
