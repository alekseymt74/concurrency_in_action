#ifndef BIN_STREAMS_H
#define BIN_STREAMS_H

#include <sstream>
#include <stdint.h>

namespace bin_streams
{

class bin_streambuf : public std::basic_stringbuf < uint8_t, std::char_traits< uint8_t > >
{
  typedef std::basic_stringbuf< uint8_t, std::char_traits< uint8_t > > base_buffer;
  uint8_t *_Pendsave;	// the saved end pointer during freeze
  bool frozen;
public:
  bin_streambuf( uint8_t *_Getptr, std::streamsize _Count )
    : base_buffer()
    , frozen( false )
  {
#ifdef WINDOWS
# if ( _MSC_VER == 1900 )
    _Init( _Getptr, ( std::size_t ) _Count, ( _Strstate ) 0 );
# elif  ( _MSC_VER > 1900 )
    _Init( _Getptr, ( std::size_t ) _Count, ( int ) 0 );
# elif
    ;
#endif // _MSC_VER
#else // UNIX
    _M_string = base_buffer::__string_type( _Getptr, ( std::size_t ) _Count );
    _M_stringbuf_init( std::ios_base::in | std::ios_base::out );
#endif // WINDOWS
  }
  ~bin_streambuf()
  {
#ifdef WINDOWS
    if( frozen )//!!!!!!!!!!!!!!!!!!!!!!!!
      freeze( false );
#endif // WINDOWS
  }
  // returns count from current to begin for input stream
  std::streamsize /*__CLR_OR_THIS_CALL*/ gcount() const
  {
    return ( gptr() == 0 ? 0 : ( std::streamsize )( gptr() - eback() ) );
  }
  // returns count from end to begin for input stream
  std::streamsize /*__CLR_OR_THIS_CALL*/ egcount() const
  {
    return ( egptr() == 0 ? 0 : ( std::streamsize )( egptr() - eback() ) );
  }
  // returns count from current to begin for output stream
  std::streamsize /*__CLR_OR_THIS_CALL*/ pcount() const
  {
    return ( pptr() == 0 ? 0 : ( std::streamsize )( pptr() - pbase() ) );
  }
  // returns count from end to begin for output stream
  std::streamsize /*__CLR_OR_THIS_CALL*/ epcount() const
  {
    return ( epptr() == 0 ? 0 : ( std::streamsize )( epptr() - pbase() ) );
  }
  uint8_t /*__CLR_OR_THIS_CALL*/ *gdata()
  {	// freeze and return pointer to character array
#ifdef WINDOWS
    freeze();
#endif // WINDOWS
    return ( eback() );
  }
  uint8_t /*__CLR_OR_THIS_CALL*/ *gdata() const
  {
    return ( eback() );
  }
  uint8_t /*__CLR_OR_THIS_CALL*/ *pdata()
  {	// freeze and return pointer to character array
#ifdef WINDOWS
    freeze();
#endif // WINDOWS
    return ( pbase() );
  }
  uint8_t /*__CLR_OR_THIS_CALL*/ *pdata() const
  {
    return ( pbase() );
  }
#ifdef WINDOWS
  void /*__CLR_OR_THIS_CALL*/ freeze( bool _Freezeit = true )
  {	// freeze or unfreeze writing
    if( _Freezeit && !frozen )
    {	// disable writing
      frozen = true;
      _Pendsave = epptr();
      setp( pbase(), pptr(), eback() );
    }
    else if( !_Freezeit && frozen )
    {	// re-enable writing
      frozen = false;
      setp( pbase(), pptr(), _Pendsave );
    }
  }
#ifdef WINDOWS
  virtual int_type __CLR_OR_THIS_CALL overflow( int_type _Meta = std::char_traits< uint8_t >::eof() )
#else // UNIX
  virtual int_type overflow( int_type _Meta = std::char_traits< uint8_t >::eof() )
#endif // WINDOWS
  {
    if( frozen )
      return ( EOF );
    return base_buffer::overflow( _Meta );
  }
#endif // WINDOWS
};

class bin_istream : public std::basic_istream<uint8_t, std::char_traits<uint8_t> >//std::istream
{
public:
  typedef std::basic_istream<uint8_t, std::char_traits<uint8_t> >::pos_type pos_type;
  typedef std::basic_istream<uint8_t, std::char_traits<uint8_t> >::off_type off_type;
private:
  bin_streambuf buf;
public:
  bin_istream( uint8_t *_Getptr, std::streamsize _Count )
#ifdef WINDOWS
    : std::basic_istream<uint8_t, std::char_traits<uint8_t> >( ( std::basic_streambuf< uint8_t, std::char_traits< uint8_t > > * ) 0, false )
#else // UNIX
    : std::basic_istream<uint8_t, std::char_traits<uint8_t> >( ( std::basic_streambuf< uint8_t, std::char_traits< uint8_t > > * ) 0 )
#endif // WINDOWS
    , buf( _Getptr, _Count )
  {
#ifdef WINDOWS
    _Myios::init( &buf, false );
#else // UNIX
    init( &buf );
#endif // WINDOWS
  }
  template< typename T >
  bin_istream &/*__CLR_OR_THIS_CALL*/ operator>>( T &_Val )
  {
    read( ( uint8_t * ) &_Val, sizeof( _Val ) );
    return ( *this );
  }
  const uint8_t *data() const
  {
    return buf.gdata();
  }
  std::streamsize length()
  {
    return buf.gcount();
  }
  std::streamsize res_count()
  {
    return buf.egcount();
  }
};

class bin_ostream : public std::basic_ostream<uint8_t, std::char_traits<uint8_t> >//std::ostream
{
public:
  typedef std::basic_ostream<uint8_t, std::char_traits<uint8_t> >::pos_type pos_type;
  typedef std::basic_ostream<uint8_t, std::char_traits<uint8_t> >::off_type off_type;
private:
  uint8_t *initial_raw_data;
  bin_streambuf buf;
public:
  bin_ostream()
#ifdef WINDOWS
    : std::basic_ostream<uint8_t, std::char_traits<uint8_t> >( std::_Noinit, false )
#else // UNIX
    : std::basic_ostream<uint8_t, std::char_traits<uint8_t> >( /*( __streambuf_type * ) 0*/ )
#endif // WINDOWS
    , initial_raw_data( 0 )
    , buf( 0, 0 )
  {
#ifdef WINDOWS
    _Myios::init( &buf, false );
#else // UNIX
    init( &buf );
#endif // WINDOWS
  }
  bin_ostream( std::streamsize initial_size )
#ifdef WINDOWS
    : std::basic_ostream<uint8_t, std::char_traits<uint8_t> >( std::_Noinit, false )
#else // UNIX
    : std::basic_ostream<uint8_t, std::char_traits<uint8_t> >( /*( __streambuf_type * ) 0*/ )
#endif // WINDOWS
    , initial_raw_data( new uint8_t[ initial_size ] )
    , buf( initial_raw_data, initial_size )
  {
#ifdef WINDOWS
    _Myios::init( &buf, false );
#else // UNIX
    init( &buf );
#endif // WINDOWS
  }
  ~bin_ostream()
  {
    if( initial_raw_data )
      delete[] initial_raw_data;
  }
  template< typename T >
  bin_ostream &/*__CLR_OR_THIS_CALL*/ operator<<( T &_Val )
  {
    write( ( uint8_t * ) &_Val, sizeof( _Val ) );
    return ( *this );
  }
  template< typename T >
  bin_ostream &/*__CLR_OR_THIS_CALL*/ operator<<( const T &_Val )
  {
    write( ( uint8_t * ) &_Val, sizeof( _Val ) );
    return ( *this );
  }
  uint8_t *data()
  {
    return buf.pdata();
  }
  std::streamsize length()
  {
    return buf.pcount();
  }
  std::streamsize res_count()
  {
    return buf.epcount();
  }
  void /*__CLR_OR_THIS_CALL*/ freeze( bool _Freezeit = true )
  {	// freeze or unfreeze writing
#ifdef WINDOWS
    buf.freeze( _Freezeit );
    if( !_Freezeit )
      clear();
#endif // WINDOWS
  }
};

} // namespace bin_streams

#endif // BIN_STREAMS_H

//using namespace bin_streams;

//int _tmain( int argc, _TCHAR* argv[] )
//{
//  bin_ostream bos( 3 );
//  std::streamsize res_count_in = bos.res_count();
//  std::streamsize count_in = bos.length();
//  int i_in = 33;
//  double d_in = 77.77;
//  bos << i_in;
//  count_in = bos.length();
//  uint8_t *str_in = bos.data();
//  bos << d_in;
//  count_in = bos.length();
//  //  bos.freeze( false );
//  bos << d_in;
//  count_in = bos.length();
//  str_in = bos.data();

//  bin_istream bis( bos.data(), bos.length() );
//  std::streamsize res_count_out = bis.res_count();
//  std::streamsize count_out = bis.length();
//  int i_out = 0;
//  double d_out = 0.0;
//  bis >> i_out >> d_out;
//  count_out = bis.length();
//  bis.seekg( 0 );
//  count_out = bis.length();
//  int i_out_1 = 0;
//  bis >> i_out_1;
//  count_out = bis.length();

//  return 0;
//}
