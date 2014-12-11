#include <iostream>

#include "zeroconf/NetService.h"

using namespace ZeroConf;

class ServiceListener : public ZeroConf::NetServiceListener
{
public:
  virtual void willPublish(NetService *pNetService)
  {
    std::cout<< "willPublish" << std::endl;
  }
  virtual void didNotPublish(NetService *pNetService)
  {
    std::cout<< "didNotPublish" << std::endl;
  }
  virtual void didPublish(NetService *pNetService)
  {
    std::cout<< "didPublish " 
    << "name: " << pNetService->getName()
    << " port: " << pNetService->getPort()
    << std::endl;
  }
  virtual void willResolve(NetService *pNetService)
  {
    std::cout<< "willResolve" << std::endl;
  }
  virtual void didNotResolve(NetService *NetService)
  {
    std::cout<< "didNotResolve" << std::endl;
  }
  virtual void didResolveAddress(NetService *pNetService)
  {
    std::cout<< "didResolveAddress" 
    << pNetService->getName() << " " << pNetService->getPort()
    << std::endl;
  }
  virtual void didUpdateTXTRecordData(NetService *pNetService)
  {
    std::cout<< "didUpdateTXTRecordData" << std::endl;
  }   
  virtual void didStop(NetService *pNetService)
  {
    std::cout<< "didStop" << std::endl;
  }
};

int main (int argc, char * const argv[]) 
{
  ServiceListener listener;
  
  ZeroConf::NetService serviceA("local", "_osc._udp", "serviceA", 12345);
  serviceA.setListener(&listener);
  serviceA.publish();
  
  ZeroConf::NetService serviceAbis(serviceA.getDomain(), serviceA.getType(), serviceA.getName());
  serviceAbis.setListener(&listener);
  serviceAbis.resolveWithTimeout(10);
  
  return 0;
}
