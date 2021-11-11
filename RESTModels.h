#ifndef RESTMODELS_H
#define RESTMODELS_H
#include "nlohmann/json.hpp"
#include <set>
#include <map>

using JSON = nlohmann::json;

namespace jsonUtils {

	JSON loadFile(const std::string path);
	bool saveFile(const std::string path, JSON j);

	template <typename Type, typename SrcType = Type>
	Type extractValue(JSON& j, const std::string key, Type defaultValue)
	{
		try
		{
			return static_cast<Type>(j[key].get<SrcType>());
		}
		catch (...)
		{
			return defaultValue;
		}
	}
}

class JSONInfoBody {
public:
	enum class RequestTypeTag
	{
		ResourceRequest,
		APIRequest,
		UnknownRequest,
	};


	enum class BodyType
	{
		FirstBody = 0,

		Body_URIInfo = FirstBody,
		Body_ErrorInfo,
		Body_ReponseInfo,
		Body_ServerInfo,
		Body_ConnectionInfo,
		Body_TokenInfo,
		
		Body_Custom,

		NumBodies,

		Body_Invalid = -1,
	};

	JSONInfoBody(BodyType ID = BodyType::Body_Invalid);
	JSONInfoBody(std::string name);
	virtual ~JSONInfoBody() {}

	virtual JSON toJSON() const = 0;
	virtual void  loadJSON(JSON) = 0;
	
	const std::string c_name;

	static std::shared_ptr<JSONInfoBody> createBody(BodyType ID);
	static std::shared_ptr<JSONInfoBody> createBody(std::string name);

	RequestTypeTag getReqTypeTag() const { return m_reqTag; }
	void setReqTypeTag(RequestTypeTag tag) { m_reqTag = tag; }

private:
	RequestTypeTag m_reqTag;
};
typedef std::shared_ptr<JSONInfoBody> BodyInfoPtr;

typedef std::map<std::string,std::shared_ptr<JSONInfoBody>> BodyMap;

class ResourceInfoBody : public JSONInfoBody
{
public:
	ResourceInfoBody()
		: JSONInfoBody("Resource_Info")
		, m_resourcePath("")
	{
	}

	virtual JSON toJSON() const override
	{
		JSON info;
		info["Path"] = m_resourcePath;
		return info;
	}
	virtual void loadJSON(JSON info) override
	{
		m_resourcePath = jsonUtils::extractValue(info, "Path", std::string());
	}

	std::string		getPath()  const { return m_resourcePath; }
	void setPath(std::string val)     { m_resourcePath = val; }

protected:
	std::string		m_resourcePath;
};


#endif // !RESTMODELS_H