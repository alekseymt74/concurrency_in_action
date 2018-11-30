#ifndef I_RWS_H
#define I_RWS_H

#include <memory>

namespace rws
{

  class i_writer : public std::enable_shared_from_this< i_writer >
  {
  public:
    virual ~i_writer() {}
  };


} // namespace rws

#endif // I_RWS_H
