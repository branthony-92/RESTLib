#ifndef RESTENDPOINT_H
#define RESTENDPOINT_H

#include "RESTModels.h"
#include "MdlResponseInfo.h"

typedef std::map<std::string, std::string> Params;
typedef std::map<std::string, std::string> ParsedBody;

class ParameterMap
{
	Params m_map;
public:
    ParameterMap(Params data) : m_map(data) {}
	ParameterMap() : m_map() {}
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

struct RequestData {
	std::string  target;
	ParameterMap queries;
	ParameterMap body;
};

typedef  std::function<BodyInfoPtr(RequestData&)> RequestCallback;
typedef  std::map<std::string,RequestCallback> CallbackMap;

#endif // !RESTENDPOINT_H