//
// XmlRpc++ Copyright (c) 2002-2008 by Chris Morley
//

#include "XmlRpcServerMethod.h"
#include "XmlRpcServer.h"

namespace XmlRpc {


  XmlRpcServerMethod::XmlRpcServerMethod(std::string const& name, XmlRpcServer* server)
  {
    _name = name;
    _server = server;
    if (_server) _server->addMethod(this);
  }

  XmlRpcServerMethod::~XmlRpcServerMethod()
  {
    if (_server) _server->removeMethod(this);
  }


} // namespace XmlRpc
