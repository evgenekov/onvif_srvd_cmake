#include "GSoapService.hpp"


// GSoapInstance Functions
/*******************************************************************************
 * Constructor for GSoapInstance Class
 *
 * @param service_ctx Service context holding gSoap configuration.
 ******************************************************************************/
GSoapInstance::GSoapInstance(ServiceContext service_ctx)
    : serviceCtx(std::move(service_ctx)), gSoap{}, DeviceBindingService_inst{gSoap.getSoapPtr()},
      MediaBindingService_inst{gSoap.getSoapPtr()}, PTZBindingService_inst{gSoap.getSoapPtr()}
{
    if (!gSoap.getSoapPtr())
        throw std::out_of_range("soap context is empty");

    gSoap.getSoapPtr()->bind_flags = SO_REUSEADDR;

    if (!soap_valid_socket(soap_bind(gSoap.getSoapPtr(), NULL, serviceCtx.port, 10)))
    {
        soap_stream_fault(gSoap.getSoapPtr(), std::cerr);
        exit(EXIT_FAILURE);
    }

    gSoap.getSoapPtr()->send_timeout = 3; // timeout in sec
    gSoap.getSoapPtr()->recv_timeout = 3; // timeout in sec

    // save pointer of service_ctx in soap
    gSoap.getSoapPtr()->user = (void *)&serviceCtx;

    // verify serviceCtx has been stored in the class.
    checkServiceCtx();

    FOREACH_SERVICE(DECLARE_SERVICE, gSoap.getSoapPtr())
}


/*******************************************************************************
 * Destructor for GSoapInstance
 ******************************************************************************/
GSoapInstance::~GSoapInstance()
{
    return;
}


/*******************************************************************************
 * Main work function for gSoap
 *
 * This function is used by ThreadWarden to manage the gSoap connection, this
 * function essentially runs like a while(1) loop until ThreadWarden tells
 * gSoap to stop runing.
 *
 * @return 1 if error, 0 if okay
 ******************************************************************************/
int GSoapInstance::work()
{
    // wait new client
    if (!soap_valid_socket(soap_accept(gSoap.getSoapPtr())))
        sleep(20);
    {
        arms::log<arms::LOG_INFO>("SOAP Invalid Socket");
        soap_stream_fault(gSoap.getSoapPtr(), std::cerr);
    }

    // process service
    if (soap_begin_serve(gSoap.getSoapPtr()))
    {
        arms::log<arms::LOG_INFO>("Process Service");
        soap_stream_fault(gSoap.getSoapPtr(), std::cerr);
    }
    FOREACH_SERVICE(DISPATCH_SERVICE, gSoap.getSoapPtr())
    else
    {
        arms::log<arms::LOG_INFO>("Unknown service");
        return 1;
    }

    arms::log<arms::LOG_INFO>("Soap Destroy");
    soap_destroy(gSoap.getSoapPtr()); // delete managed C++ objects
    soap_end(gSoap.getSoapPtr());     // delete managed memory
    return 0;
}


/*******************************************************************************
 * Veritication function to ensure that service_ctx is holding the correct
 * information.
 ******************************************************************************/
void GSoapInstance::checkServiceCtx(void)
{
    if (serviceCtx.eth_ifs.empty())
        throw std::runtime_error("Error: not set no one ehternet interface more details see opt --ifs\n");

    if (serviceCtx.scopes.empty())
        throw std::runtime_error("Error: not set scopes more details see opt --scope\n");

    if (serviceCtx.get_profiles().empty())
        throw std::runtime_error("Error: not set no one profile more details see --help\n");
}
