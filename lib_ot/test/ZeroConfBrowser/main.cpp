#include <iostream>
#include "zeroconf/NetServiceBrowser.h"
#include "zeroconf/NetService.h"

using namespace ZeroConf;

class BrowserListener : public ZeroConf::NetServiceBrowserListener
{
public:
  virtual void didFindDomain(NetServiceBrowser *pNetServiceBrowser, const std::string &domainName, bool moreDomainsComing) {}
  virtual void didRemoveDomain(NetServiceBrowser *pNetServiceBrowser, const std::string &domainName, bool moreDomainsComing) {}
  
  virtual void didFindService(NetServiceBrowser* pNetServiceBrowser, NetService *pNetService, bool moreServicesComing) 
  {
    std::cout << "Service found: " << pNetService->getName() << " "<< pNetService->getType() << std::endl;
  }
  virtual void didRemoveService(NetServiceBrowser *pNetServiceBrowser, NetService *pNetService, bool moreServicesComing) 
  {
    std::cout << "Service removed: " << pNetService->getName() << " "<< pNetService->getType() << std::endl;
  }
  
  virtual void willSearch(NetServiceBrowser *pNetServiceBrowser) {}
  virtual void didNotSearch(NetServiceBrowser *pNetServiceBrowser) {}
  
  virtual void didStopSearch(NetServiceBrowser *pNetServiceBrowser) {}
};

int main (int argc, char * const argv[]) 
{
  NetServiceBrowser browser;
  BrowserListener listener;
  browser.setListener(&listener);
  browser.searchForServicesOfType("_http._tcp", "local");
    
  return getchar();
}
