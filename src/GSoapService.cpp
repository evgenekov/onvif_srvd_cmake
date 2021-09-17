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

    gSoap.getSoapPtr()->fget = http_get;

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
        sleep(1);
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


/*******************************************************************************
 * HTTP GET Callback function
 *
 * This function will be called if the gSoap instance recieves a GET request
 * over HTTP, this will initially be used to serve a snapshot image but may
 * eventually be used to help configure the deamon from a web interface
 *
 * @return HTTP Response codes (404 for error)
 ******************************************************************************/
int GSoapInstance::http_get(struct soap* soap)
{
    if (strchr(soap->path + 1, '/') || strchr(soap->path + 1, '\\'))
        return 403;
    if (!soap_tag_cmp(soap->path, "*.html"))
        return copy_file(soap, soap->path + 1, "text/html");
    if (!soap_tag_cmp(soap->path, "*.xml") || !soap_tag_cmp(soap->path, "*.xsd")
    || !soap_tag_cmp(soap->path, "*.wsdl"))
        return copy_file(soap, soap->path + 1, "text/xml");
    if (!soap_tag_cmp(soap->path, "*.jpg"))
        return copy_file(soap, soap->path + 1, "image/jpeg");
    if (!soap_tag_cmp(soap->path, "*.gif"))
        return copy_file(soap, soap->path + 1, "image/gif");
    if (!soap_tag_cmp(soap->path, "*.png"))
        return copy_file(soap, soap->path + 1, "image/png");
    if (!soap_tag_cmp(soap->path, "*.ico"))
        return copy_file(soap, soap->path + 1, "image/ico");
    return 404; /* HTTP not found */
}


/*******************************************************************************
 * Copy File
 *
 * This function will take a file from the device the daemon is running from,
 * open it up and serve it to the device over HTTO, images and http files are
 * supported at the moment
 *
 * @return SOAP Status
 ******************************************************************************/
int GSoapInstance::copy_file(struct soap *soap, const char *name, const char *type)
{
    printf("NAME: %s", name);
    FILE *fd;
    size_t r;
    fd = fopen(name, "rb"); /* open file to copy */
    if (!fd)
        return 404; /* return HTTP not found */
    soap->http_content = type;
    if (soap_response(soap, SOAP_FILE)) /* OK HTTP response header */
    {
        soap_end_send(soap);
        fclose(fd);
        return soap->error;
    }
    for (;;)
    {
        r = fread(soap->tmpbuf, 1, sizeof(soap->tmpbuf), fd);
        if (!r)
            break;
        if (soap_send_raw(soap, soap->tmpbuf, r))
        {
            soap_end_send(soap);
            fclose(fd);
            return soap->error;
        }
    }
    fclose(fd);
    return soap_end_send(soap);
}
