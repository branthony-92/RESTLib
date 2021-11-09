#ifndef RESTENDPOINT_H
#define RESTENDPOINT_H

#include "RESTModels.h"
#include "RESTServerContext.h"
#include "MdlResponseInfo.h"


typedef std::map<std::string, std::string> Params;
typedef std::map<std::string, std::string> ParsedBody;

class ParameterMap
{
	Params m_map;
public:
    ParameterMap(Params data) : m_map(data) {}
	virtual ~ParameterMap() {}

	bool contains(const std::string key)
	{
		return m_map.find(key) != m_map.end();
	}
	std::string extract(std::string key) 
	{
		if (!contains(key)) return "";
		return m_map.at(key);
	}
	int extract(std::string key, int def)
	{
		if (!contains(key)) return def;
		auto val = m_map.at(key);
		try
		{
			auto result = std::stoi(val);
			return result;
		}
		catch (...)
		{
			return def;
		}
	}
	long extract(std::string key, long def)
	{
		if (!contains(key)) return def;
		auto val = m_map.at(key);
		try
		{
			auto result = std::stol(val);
			return result;
		}
		catch (...)
		{
			return def;
		}
	}
	float  extract(std::string key, float  def)
	{
		if (!contains(key)) return def;
		auto val = m_map.at(key);
		try
		{
			auto result = std::stof(val);
			return result;
		}
		catch (...)
		{
			return def;
		}
	}
	double extract(std::string key, double def)
	{
		if (!contains(key)) return def;
		auto val = m_map.at(key);
		try
		{
			auto result = std::stod(val);
			return result;
		}
		catch (...)
		{
			return def;
		}
	}
	bool extract(std::string key, bool def)
	{
		if (!contains(key))      return def;
		auto val = m_map.at(key);
		for (auto& c : val) c = std::tolower(c);
		if (val == "true")		 return true;
		else if (val == "false") return false;
		return def;
	}

	bool empty() const { return m_map.empty(); }
};

class HTTPRequestHandler
{
public:

    enum class RequestType
    {
        ResourceRequest,
        APIRequest,
        UnknownRequest,
    };

    HTTPRequestHandler(std::string name, RequestType type)
        : m_name(name)
        , m_dataType(type)
        , m_pResponseBody(nullptr)
		, m_IDTag("")
    {}

    virtual ~HTTPRequestHandler() {};

    // virtual HTTP method handlers 
    // These will be responsible for parsing the target string for queries
    virtual std::shared_ptr<JSONInfoBody> handleRequest_Get(std::string target,    ParameterMap queries, ParameterMap body, TRESTCtxPtr pCtx) { return nullptr; }
    virtual std::shared_ptr<JSONInfoBody> handleRequest_Put(std::string target,    ParameterMap queries, ParameterMap body, TRESTCtxPtr pCtx) { return nullptr; }
    virtual std::shared_ptr<JSONInfoBody> handleRequest_Post(std::string target,   ParameterMap queries, ParameterMap body, TRESTCtxPtr pCtx) { return nullptr; }
    virtual std::shared_ptr<JSONInfoBody> handleRequest_Delete(std::string target, ParameterMap queries, ParameterMap body, TRESTCtxPtr pCtx) { return nullptr; }
    virtual std::shared_ptr<JSONInfoBody> handleRequest_Head(std::string target,   ParameterMap queries, ParameterMap body, TRESTCtxPtr pCtx) { return nullptr; }

    // base property accessors
    std::string getName() const { return m_name; }
    RequestType getType() const { return m_dataType; }
    ReponseBodyPtr getResponseBody() const { return m_pResponseBody; }
    void resetResponseBody() { m_pResponseBody = std::make_shared<ResponseInfoBody>(); }

	std::string getIDTag() const { return m_IDTag; }
	void setIDTag(std::string id) { m_IDTag = id; }

protected:
    std::string m_name;
	RequestType    m_dataType;
    ReponseBodyPtr m_pResponseBody;

	std::string m_IDTag;
};
typedef std::shared_ptr<HTTPRequestHandler>  ReqHandlerPtr;
typedef std::map<std::string, ReqHandlerPtr> ReqHandlerMap;

class ResourceRequestHandler : public HTTPRequestHandler
{

public:
    ResourceRequestHandler() : HTTPRequestHandler("Resource", HTTPRequestHandler::RequestType::ResourceRequest) {}

    // virtual HTTP method handlers
    virtual std::shared_ptr<JSONInfoBody> handleRequest_Get(std::string target, ParameterMap queries, ParameterMap body, TRESTCtxPtr pCtx) override;
};

#endif // !RESTENDPOINT_H