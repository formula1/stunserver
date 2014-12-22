
#include "commonincludes.hpp"
#include <node.h>
#include "stdio.h"
#include "stuncore.h"
#include "server.h"
#include "socketaddress.h"
#include "adapters.h"
#include "resolvehostname.h"
#include "nodestun_args.h"
#include "nodestun_generic.h"


using namespace v8;


bool NodeStun_Args::OneArgs(const Arguments& args, Local<Object> &option_map){

  if(args[0]->IsUndefined())
  {
    // its already been created
  }
  else if(args[0]->IsObject())
  {
    option_map = args[0]->ToObject();
  }
  else
  {
    return NS_G::throwErr("The StunServer Configuration must be an Object");
  }
  return false;
}

bool NodeStun_Args::ThreeArgs(const Arguments& args, Local<Object> &option_map){
  int n;
  int argslen = args.Length();
  Local<Value> temp;

  Local<String> port_options [3] = {
    String::NewSymbol("port"),
    String::NewSymbol("interface"),
    String::NewSymbol("advertised")
  };
  Local<String> port_keys [6] = {
    String::NewSymbol("primary_port"),
    String::NewSymbol("primary_interface"),
    String::NewSymbol("primary_advertised"),
    String::NewSymbol("alternate_port"),
    String::NewSymbol("alternate_interface"),
    String::NewSymbol("alternate_advertised")
  };

  Local<String> option_names [5] = {
    String::NewSymbol("protocol"),
    String::NewSymbol("mode"),
    String::NewSymbol("family"),
    String::NewSymbol("max_connections"),
    String::NewSymbol("verbosity")
  };

  if(argslen > 3)
  {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
    return true;
  }

  for(n=0;n<2;n++)
  {
    int nn;
    if(!(argslen > n) || args[n]->IsUndefined())
    {
      Logging::LogMsg(LL_DEBUG,"Stun[%d] will be defaults", n);
    }else if(args[n]->IsObject())
    {
      Logging::LogMsg(LL_DEBUG,"Stun[%d] is detailed", n);
      Handle<Object> opt = args[n]->ToObject();
      for (nn=0; nn<3; ++nn)
      {
        temp = opt->Get(port_options[nn]);
        if(!temp->IsUndefined())
        {
          Logging::LogMsg(LL_DEBUG,
          "Stun[%d].detail[%s] will be set",
          n,
          NS_G::v8str2stdstr(port_options[nn]).c_str()
          );
          option_map->Set(port_keys[n*3+nn], temp);
        }else{
          Logging::LogMsg(LL_DEBUG,
          "Stun[%d].detail[%s] will be default",
          n,
          NS_G::v8str2stdstr(port_options[nn]).c_str()
          );
        }
      }
    }
    else if(args[n]->IsNumber())
    {
      Logging::LogMsg(LL_DEBUG,"Only setting port for Stun[%d]", n);
      option_map->Set(port_keys[n*3], args[n]);
    }
    else
    {
      ThrowException(Exception::TypeError(
      String::New("Primary can only be an object, an integer or undefined")
      ));
      return true;
    }
    if(option_map->Get(port_keys[n*3])->IsUndefined())
    {
      if(n==0)
      {
        option_map->Set(port_keys[n*3], Number::New(3478));
      }
      else
      {
        nn = option_map->Get(port_keys[(n-1)*3])->Int32Value();
        option_map->Set(port_keys[n*3], Number::New((nn+1)%65536));
      }
    }
    else if(!option_map->Get(port_keys[n*3])->IsNumber())
    {
      ThrowException(Exception::TypeError(String::New("Port can only be a integer")));
      return true;
    }
  }
  if(args.Length() < 3 || args[2]->IsUndefined())
  {
    Logging::LogMsg(LL_DEBUG,"Extra Options will be defaults");
  }
  else if(args[2]->IsObject())
  {
    Handle<Object> opt = args[2]->ToObject();
    Logging::LogMsg(LL_DEBUG,"Extra Options is detailed");
    for (n=0; n<5; ++n)
    {
      temp = opt->Get(option_names[n]);
      if(!temp->IsUndefined())
      {
        Logging::LogMsg(LL_DEBUG,
        "Extra.detail[%s]",
        NS_G::v8str2stdstr(option_names[n]).c_str()
        );
        option_map->Set(option_names[n], temp);
      }
    }
  }
  else if(args[2]->IsString())
  {
    Logging::LogMsg(LL_DEBUG,"Only setting protocol in Extras");
    option_map->Set(option_names[0], args[2]);
  }
  else
  {
    NS_G::throwErr("Extra Options can only be an object or a string");
    return true;
  }
  return false;
}

bool NodeStun_Args::Object2Config(Local<Object> &option_map, CStunServerConfig& configOut){
  //Temporary values
  std::string tempstr;
  int tempint;
  Handle<Value> tempv8;
  HRESULT hr;

  CStunServerConfig config;


  // default values;
  int family = AF_INET;
  // bool fIsUdp = true;
  int nPrimaryPort = DEFAULT_STUN_PORT;
  int nAltPort = DEFAULT_STUN_PORT + 1;
  bool fHasAtLeastTwoAdapters = false;

  enum ServerMode
  {
    Basic,
    Full
  };
  ServerMode mode=Basic;

  // ---- MODE ----------------------------------------------------------
  tempv8 = option_map->Get(String::NewSymbol("mode"));
  if (!tempv8->IsUndefined())
  {
    if(!tempv8->IsString())
    {
      return NS_G::throwErr("Mode can only be an Integer or Undefined");
    }

    tempstr = NS_G::v8str2stdstr(tempv8->ToString());
    if (tempstr == "basic")
    {
      mode = Basic;
    }
    else if (tempstr == "full")
    {
      mode = Full;
    }
    else
    {
      return NS_G::throwErr("Mode must be \"full\" or \"basic\".");
    }
    Logging::LogMsg(LL_DEBUG,"Setting mode to: %s",tempstr.c_str());
  }
  else
  {
    Logging::LogMsg(LL_DEBUG,"mode will be default");
  }


  // ---- FAMILY --------------------------------------------------------
  family = AF_INET;
  tempv8 = option_map->Get(String::NewSymbol("family"));
  if (!tempv8->IsUndefined())
  {
    if(!tempv8->IsNumber())
    {
      return NS_G::throwErr("IP Family can only be a String or Undefined");
    }
    tempint = tempv8->Int32Value();
    if (tempint == 4)
    {
      family = AF_INET;
    }
    else if (tempint == 6)
    {
      family = AF_INET6;
    }
    else
    {
      return NS_G::throwErr("IP Family argument must be 4 or 6");
    }
    Logging::LogMsg(LL_DEBUG,"Setting IP Family to: %d",tempint);
  }
  else
  {
    Logging::LogMsg(LL_DEBUG,"IP Family will be default");
  }

  // ---- PROTOCOL --------------------------------------------------------
  tempv8 = option_map->Get(String::NewSymbol("protocol"));
  if (!tempv8->IsUndefined())
  {
    if(!tempv8->IsString())
    {
      return NS_G::throwErr("protocol can only be a String or Undefined");
    }
    tempstr = NS_G::v8str2stdstr(tempv8->ToString());
    if ((tempstr != "udp") && (tempstr != "tcp"))
    {
      return NS_G::throwErr("Protocol argument must be 'udp' or 'tcp'. 'tls' is not supported yet");
    }
    config.fTCP = (tempstr == "tcp");
    Logging::LogMsg(LL_DEBUG,"Setting Protocol to: %s",tempstr.c_str());
  }
  else
  {
    Logging::LogMsg(LL_DEBUG,"Protocol will be default");
  }


  // ---- MAX Connections -----------------------------------------------------
  tempv8 = option_map->Get(String::NewSymbol("max_connections"));
  if (!tempv8->IsUndefined())
  {
    if(!tempv8->IsNumber())
    {
      return NS_G::throwErr("Maximum connections can only be an Integer or Undefined");
    }
    tempint = tempv8->Int32Value();
    if (config.fTCP == false)
    {
      return NS_G::throwErr("Maximum connections parameter has no meaning in UDP mode. Did you mean to specify {\"protocol\":\"tcp\"} ?");
    }
    else
    {
      if(tempint < 1 || tempint > 100000)
      {
        return NS_G::throwErr("Maximum connections must be between 1-100000");
      }
    }
    config.nMaxConnections = tempint;
    Logging::LogMsg(LL_DEBUG,"Setting Maximum connections to: %d",tempint);
  }
  else
  {
    Logging::LogMsg(LL_DEBUG,"Maximum connections will be default");
  }


  // ---- PRIMARY PORT --------------------------------------------------------
  nPrimaryPort = DEFAULT_STUN_PORT;
  tempv8 = option_map->Get(String::NewSymbol("primary_port"));
  if (!tempv8->IsUndefined())
  {
    if(!tempv8->IsNumber())
    {
      return NS_G::throwErr("Primary Port can only be an Integer or Undefined");
    }
    tempint = tempv8->Int32Value();
    if((tempint < 0x0001) || (tempint > 0xffff))
    {
      return NS_G::throwErr("Primary port value is invalid.  Value must be between 1-65535");
    }
    nPrimaryPort = tempint;
    Logging::LogMsg(LL_DEBUG,"Setting Primary Port to: %d",tempint);
  }
  else
  {
    Logging::LogMsg(LL_DEBUG,"Primary Port will be default");
  }

  // ---- ALT PORT --------------------------------------------------------
  nAltPort = DEFAULT_STUN_PORT + 1;
  tempv8 = option_map->Get(String::NewSymbol("alternate_port"));
  if (!tempv8->IsUndefined())
  {
    if(!tempv8->IsNumber())
    {
      return NS_G::throwErr("Alternative Port can only be an Integer or Undefined");
    }
    tempint = tempv8->Int32Value();
    if((tempint < 0x0001) || (tempint > 0xffff))
    {
      return NS_G::throwErr("Alternative port value is invalid.  Value must be between 1-65535");
    }
    nAltPort = tempint;
    Logging::LogMsg(LL_DEBUG,"Setting Alternate Port to: %d",tempint);
  }
  else
  {
    Logging::LogMsg(LL_DEBUG,"Alternate Port will be default");
  }

  if (nPrimaryPort == nAltPort)
  {
    return NS_G::throwErr("Primary port and alternate port must be different values");
  }


  // ---- Adapters and mode adjustment

  fHasAtLeastTwoAdapters = ::HasAtLeastTwoAdapters(family);

  if (mode == Basic)
  {
    uint16_t port = (uint16_t)((int16_t)nPrimaryPort);
    // in basic mode, if no adapter is specified, bind to all of them
    tempv8 = option_map->Get(String::NewSymbol("primary_interface"));
    if (tempv8->IsUndefined())
    {
      if (family == AF_INET)
      {
        config.addrPP = CSocketAddress(0, port);
        config.fHasPP = true;
      }
      else if (family == AF_INET6)
      {
        sockaddr_in6 addr6 = {}; // zero-init
        addr6.sin6_family = AF_INET6;
        config.addrPP = CSocketAddress(addr6);
        config.addrPP.SetPort(port);
        config.fHasPP = true;
      }
      Logging::LogMsg(LL_DEBUG,"Primary Interface will be default");
    }
    else
    {
      if(!tempv8->IsString())
      {
        return NS_G::throwErr("Mode can only be an String or Undefined");
      }

      tempstr = NS_G::v8str2stdstr(tempv8->ToString());

      CSocketAddress addr;
      hr = ::GetSocketAddressForAdapter(family, tempstr.c_str(), port, &addr);
      if (FAILED (hr))
      {
        return NS_G::throwErr("No matching primary adapter found");
      }
      config.addrPP = addr;
      config.fHasPP = true;
      Logging::LogMsg(LL_DEBUG,"Setting Primary Interface to: %s",tempstr.c_str());
    }
  }
  else  // Full mode
  {
    CSocketAddress addrPrimary;
    CSocketAddress addrAlternate;
    uint16_t portPrimary = (uint16_t)((int16_t)nPrimaryPort);
    uint16_t portAlternate = (uint16_t)((int16_t)nAltPort);

    // in full mode, we can't bind to all adapters
    // so if one isn't specified, it's best guess - just avoid duplicates
    if (fHasAtLeastTwoAdapters == false)
    {
      return NS_G::throwErr("There does not appear to be two or more unique IP addresses to run in full mode");
    }
    tempv8 = option_map->Get(String::NewSymbol("primary_interface"));
    if (!tempv8->IsUndefined())
    {
      if(!tempv8->IsString())
      {
        return NS_G::throwErr("Primary Interface can only be an String or Undefined");
      }
      tempstr = NS_G::v8str2stdstr(tempv8->ToString());
      hr = GetSocketAddressForAdapter(family, tempstr.c_str(), 0, &addrPrimary);
      if(FAILED(hr))
      {
        return NS_G::throwErr("Failed to get Socket address for Primary Interface");
      }
      Logging::LogMsg(LL_DEBUG,"Setting Primary Interface to: %s",tempstr.c_str());
    }
    else
    {
      hr = GetBestAddressForSocketBind(true, family, 0, &addrPrimary);
      std::string refstr1;
      if (SUCCEEDED(hr))
      {
        addrPrimary.ToString(&refstr1);
        // strip off the port suffix
        size_t x = refstr1.find_last_of(':');
        if (x != std::string::npos)
        {
          refstr1 = refstr1.substr(0, x);
        }
      }
      else
      {
        return NS_G::throwErr("Failed to find a Socket for Primary Interface");
      }
      Logging::LogMsg(LL_DEBUG,"Primary Interface will be default");
    }

    tempv8 = option_map->Get(String::NewSymbol("alternate_interface"));
    if (!tempv8->IsUndefined())
    {
      if(!tempv8->IsString())
      {
        return NS_G::throwErr("Alternate Interface can only be an String or Undefined");
      }
      tempstr = NS_G::v8str2stdstr(tempv8->ToString());
      hr = GetSocketAddressForAdapter(family, tempstr.c_str(), 0, &addrAlternate);
      if(FAILED(hr))
      {
        return NS_G::throwErr("Failed to get Socket address for Alternate Interface");
      }
      Logging::LogMsg(LL_DEBUG,"Setting Alternate Interface to: %s",tempstr.c_str());
    }
    else
    {
      hr = GetBestAddressForSocketBind(false, family, 0, &addrAlternate);
      std::string refstr2;
      if (SUCCEEDED(hr))
      {
        addrAlternate.ToString(&refstr2);
        // strip off the port suffix
        size_t x = refstr2.find_last_of(':');
        if (x != std::string::npos)
        {
          refstr2 = refstr2.substr(0, x);
        }
        return NS_G::throwErr("Failed to find a Socket for Alternate Interface");
      }
      Logging::LogMsg(LL_DEBUG,"Alternate Interface will be default");
    }

    if  (addrPrimary.IsSameIP(addrAlternate))
    {
      return NS_G::throwErr("Primary interface and Alternate Interface appear to have the same IP address. Full mode requires two IP addresses that are unique");
    }


    config.addrPP = addrPrimary;
    config.addrPP.SetPort(portPrimary);
    config.fHasPP = true;

    config.addrPA = addrPrimary;
    config.addrPA.SetPort(portAlternate);
    config.fHasPA = true;

    config.addrAP = addrAlternate;
    config.addrAP.SetPort(portPrimary);
    config.fHasAP = true;

    config.addrAA = addrAlternate;
    config.addrAA.SetPort(portAlternate);
    config.fHasAA = true;

  }


  // ---- Address advertisement --------------------------------------------------------
  // handle the advertised address parameters and make sure they are valid IP address strings

  tempv8 = option_map->Get(String::NewSymbol("primary_advertised"));
  if (!tempv8->IsUndefined())
  {
    if(!tempv8->IsString())
    {
      return NS_G::throwErr("Primary Advertised can only be an String or Undefined");
    }
    tempstr = NS_G::v8str2stdstr(tempv8->ToString());
    hr = ::NumericIPToAddress(family, tempstr.c_str(), &config.addrPrimaryAdvertised);
    if (FAILED(hr))
    {
      return NS_G::throwErr("Primary Advertised is not a valid IP address string");
    }
    Logging::LogMsg(LL_DEBUG,"Setting Primary Advertised to: %s",tempstr.c_str());
  }
  else
  {
    Logging::LogMsg(LL_DEBUG,"Primary Advertised will be default");
  }

  tempv8 = option_map->Get(String::NewSymbol("alternate_advertised"));
  if (!tempv8->IsUndefined())
  {
    if(!tempv8->IsString())
    {
      return NS_G::throwErr("Alternate Advertised can only be an String or Undefined");
    }
    tempstr = NS_G::v8str2stdstr(tempv8->ToString());
    if (mode != Full)
    {
      return NS_G::throwErr("Cannot set Alternate Advertised unless {mode:\"full\"}");
    }
    hr = ::NumericIPToAddress(family, tempstr.c_str(), &config.addrAlternateAdvertised);
    if (FAILED(hr))
    {
      return NS_G::throwErr("Alternate Advertised is not a valid IP address string");
    }
    Logging::LogMsg(LL_DEBUG,"Setting Alternate Advertised to: %d",tempint);
  }
  else
  {
    Logging::LogMsg(LL_DEBUG,"Alternate Advertised will be default");
  }
  configOut = config;


  return false;
}
