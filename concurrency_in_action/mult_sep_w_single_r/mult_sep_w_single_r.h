#ifndef MULT_SEP_W_SINGLE_R_H
#define MULT_SEP_W_SINGLE_R_H

#include <memory>
#include <shared_mutex>

#include "../bin_streams/bin_streams.h"

class subscriber;
typedef std::unique_ptr< subscriber > sub_ptr;

class os_writer : public std::enable_shared_from_this< os_writer >
{
  /*mutable*/ std::shared_timed_mutex mtx;
  bin_streams::bin_ostream _os;
public:
  sub_ptr push_elem( uint32_t _sz );
  void update( uint32_t ind, const void *_ptr, uint32_t _sz );
  void process();
};
typedef std::shared_ptr< os_writer > os_w_ptr;

class elem_writer
{
  os_w_ptr writer;
  uint32_t ind;
  uint32_t sz;
public:
  elem_writer( os_w_ptr _writer, uint32_t _ind, uint32_t _sz );
  void update( const void *_ptr, uint32_t _sz );
  uint32_t get_id();
  uint32_t get_size();
};
typedef std::unique_ptr< elem_writer > elem_w_ptr;

class subscriber
{
  elem_w_ptr writer;
public:
  subscriber( elem_w_ptr &&_writer );
  void update( const void *_ptr, uint32_t _sz );
  uint32_t get_id();
  uint32_t get_size();
  bool operator<( const subscriber &rsh );
};
typedef std::unique_ptr< subscriber > sub_ptr;

bool operator<( const sub_ptr &lhs, const sub_ptr &rsh );

#endif // MULT_SEP_W_SINGLE_R_H
