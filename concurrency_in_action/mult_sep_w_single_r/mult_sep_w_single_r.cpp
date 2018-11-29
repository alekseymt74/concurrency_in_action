#include "mult_sep_w_single_r.h"

sub_ptr os_writer::push_elem( uint32_t _sz )
{
  std::unique_lock< boost::shared_mutex > lock( mtx );
  uint32_t ind = ( uint32_t ) _os.tellp() + sizeof( uint32_t ) + sizeof( _sz );
  _os << ind << _sz;
  elem_w_ptr elem_w = std::make_unique< elem_writer >( shared_from_this(), ind, _sz );
  sub_ptr sub = std::make_unique< subscriber >( std::move( elem_w ) );
  for( uint32_t i = 0; i < _sz; ++i )
    _os << uint8_t( 0 );

  return std::move( sub );
}

void os_writer::update( uint32_t ind, const void *_ptr, uint32_t _sz )
{
  assert( ind + _sz <= _os.length() );
  uint8_t *pos = _os.data() + ind;
  boost::shared_lock< boost::shared_mutex > lock( mtx );
  memcpy( ( void * ) pos, _ptr, _sz );
}

void os_writer::process()
{
  std::unique_lock< boost::shared_mutex > lock( mtx );
  /* do smth*/
}

elem_writer::elem_writer( os_w_ptr _writer, uint32_t _ind, uint32_t _sz )
  : writer( _writer )
  , ind( _ind )
  , sz( _sz )
{}

void elem_writer::update( const void *_ptr, uint32_t _sz )
{
  assert( sz == _sz );
  writer->update( ind, _ptr, _sz );
}

uint32_t elem_writer::get_id() { return ind; }

uint32_t elem_writer::get_size() { return sz; }

subscriber::subscriber( elem_w_ptr &&_writer )
  : writer( std::move( _writer ) )
{}

void subscriber::update( const void *_ptr, uint32_t _sz )
{
  writer->update( _ptr, _sz );
}

uint32_t subscriber::get_id()
{
  return writer->get_id();
}

uint32_t subscriber::get_size()
{
  return writer->get_size();
}

bool subscriber::operator<( const subscriber &rsh )
{
  return ( this->writer->get_id() < rsh.writer->get_id() );
}

bool operator<( const sub_ptr &lhs, const sub_ptr &rsh )
{
  return ( lhs->operator<( *rsh.get() ) );
}
