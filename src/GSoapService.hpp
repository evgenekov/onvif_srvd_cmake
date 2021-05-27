#ifndef GSOAPSERVICE_H
#define GSOAPSERVICE_H

// ---- project includes ----
#include "ServiceContext.h"

// ---- gsoap ----
#include "soapDeviceBindingService.h"
#include "soapMediaBindingService.h"
#include "soapPTZBindingService.h"

// ---- armoury ----
#include "armoury/files.hpp"
#include "armoury/logger.hpp"

#define FOREACH_SERVICE(APPLY, soap)                                                                                     \
    APPLY(DeviceBindingService, soap)                                                                                    \
    APPLY(MediaBindingService, soap)                                                                                     \
    APPLY(PTZBindingService, soap)                                                                                       \
    /*                                                                                                                   \
     * If you need support for other services,                                                                           \
     * add the desired option to the macro FOREACH_SERVICE.                                                              \
     *                                                                                                                   \
     * Note: Do not forget to add the gsoap binding class for the service,                                               \
     * and the implementation methods for it, like for DeviceBindingService                                              \
                                                                                                                       \ \
                                                                                                                       \ \
                                                                                                                       \ \
            APPLY(ImagingBindingService, soap)                                                                           \
            APPLY(PTZBindingService, soap)                                                                               \
            APPLY(RecordingBindingService, soap)                                                                         \
            APPLY(ReplayBindingService, soap)                                                                            \
            APPLY(SearchBindingService, soap)                                                                            \
            APPLY(ReceiverBindingService, soap)                                                                          \
            APPLY(DisplayBindingService, soap)                                                                           \
            APPLY(EventBindingService, soap)                                                                             \
            APPLY(PullPointSubscriptionBindingService, soap)                                                             \
            APPLY(NotificationProducerBindingService, soap)                                                              \
            APPLY(SubscriptionManagerBindingService, soap)                                                               \
    */



#define DECLARE_SERVICE(service, soap) service service##_inst(soap);

#define DISPATCH_SERVICE(service, soap)                                                                                \
    else if (service##_inst.dispatch() != SOAP_NO_METHOD)                                                              \
    {                                                                                                                  \
        soap_send_fault(soap);                                                                                         \
        soap_stream_fault(soap, std::cerr);                                                                            \
    }


/*******************************************************************************
 * Wrapper structure to hold gSoap instance
 *
 * This structure contains the required confrol functions to instantiate and
 * safely destruct the gSoap pointer.
 *
 * This class will place the gSoap pointer within a unique pointer in order
 * to ensure that the memory is freed when gSoap has been closed down
 ******************************************************************************/
struct GSoapWrapper
{
    GSoapWrapper() : ptr(soap_new(), &soap_free)
    {
    }

    soap const *getSoapPtr() const
    {
        return ptr.get();
    }

    soap *getSoapPtr()
    {
        return ptr.get();
    }

    ~GSoapWrapper()
    {
        soap_destroy(ptr.get()); // delete managed C++ objects
        soap_end(ptr.get());     // delete managed memory
    }

  private:
    std::unique_ptr<soap, decltype(&soap_free)> ptr;
};


/*******************************************************************************
 * Instance class to namage gSoap
 *
 * This class is used to manage gSoap via ThreadWarden
 ******************************************************************************/
class GSoapInstance
{
  public:
    static constexpr char const *g_workerName{"gsoap thread"};
    static constexpr bool g_copyDataOnce{true};
    struct Input
    {
    } dataIn;
    struct Output
    {
    } dataOut;

    int work();

    GSoapInstance(ServiceContext service_ctx);
    ~GSoapInstance();

    void checkServiceCtx();

  private:
    ServiceContext serviceCtx;
    GSoapWrapper gSoap;
    DeviceBindingService DeviceBindingService_inst;
    MediaBindingService MediaBindingService_inst;
    PTZBindingService PTZBindingService_inst;
};

#endif // GSOAPSERVICE_H
