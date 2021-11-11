#include "StdAfx.h"
#include "Session.h"

#include <set>

const std::set<std::string> c_supportedContentTypes = {
    "application/json",
    "application/x-www-form-urlencoded"
};


void parseJSONBody(std::map<std::string, std::string> &map, const std::string body)
{
    // parse as json
    JSON jBody = JSON::parse(body);
    for (auto j : jBody.items())
    {
        JSON value = j.value();
        std::string valStr;
        if (value.is_string())
        {
            valStr = value;
        }
        else if (value.is_number_float())
        {
            valStr = std::to_string(value.get<double>());
        }
        else if (value.is_number_integer())
        {
            valStr = std::to_string(value.get<int>());
        }
        else if (value.is_boolean())
        {
            valStr = value.get<bool>() ? "true" : "false";
        }

        if (valStr.empty()) return;

        map.insert_or_assign(j.key(), valStr);
    }
}

void parse_x_www_form_urlencoded(std::map<std::string, std::string>& map, const std::string body)
{
    // TODO : figure out how to parse this
    
    const char c_eqChar = '=';
    const char c_delimChar = '&';

    int offset = 0u;

    do
    {
        int keyStartIndex = offset;
        int keyEndIndex   = body.find_first_of(c_eqChar, offset);
        int valStartIndex = keyEndIndex + 1;
        int valEndIndex   = body.find_first_of(c_delimChar, valStartIndex);

        auto keyLen = keyEndIndex - keyStartIndex;
        auto valLen = valEndIndex - valStartIndex;

        auto keyString = body.substr(keyStartIndex, keyLen);
        auto valString = body.substr(valStartIndex, valLen);

        map.insert_or_assign(keyString, valString);

        offset = valEndIndex + 1;
    }     
    while (offset > 0 && offset < body.length());
}

std::map<std::string, std::string> SessionBase::parseBody(std::string contentType, const std::string body)
{
    
    std::map<std::string, std::string> bodyData;

    if (c_supportedContentTypes.count(contentType) == 0)
    {
        // unsupported content type
        return bodyData;
    }

    if (contentType == "application/json")
    {
        // parse as json
        parseJSONBody(bodyData, body);
    }
    else if (contentType == "application/x-www-form-urlencoded")
    {
        parse_x_www_form_urlencoded(bodyData, body);
    }
    else
    {

    }

    return bodyData;
}

std::map<std::string, std::string> SessionBase::extractQueries(beast::string_view target)
{
    // find '?' to identify a key and '=' to identify values, all deliminated by '&'
    // find first query key
    std::map<std::string, std::string> queries{};

    size_t searchOffset = 0;
    size_t keyIndex = 0;
    size_t valueIndex = 0;
    size_t nextKey = 0;

    std::string str(target.data(), target.length());

    keyIndex = str.find_first_of('?', keyIndex);
    while (keyIndex != beast::string_view::npos && valueIndex != beast::string_view::npos && nextKey != beast::string_view::npos)
    {
        std::string key = "";
        std::string val = "";
        if (keyIndex == beast::string_view::npos) return queries;
        // look for the ampersand pos
        nextKey = str.find_first_of('&', keyIndex + 1);

        // look for a value for that key
        valueIndex = str.find_first_of('=', keyIndex);

        // there is no value for this key but maybe one for a future key
        if (valueIndex >= nextKey)
        {
            key = str.substr(keyIndex + 1, nextKey - keyIndex - 1).data();
        }
        else
        {
            key = str.substr(keyIndex + 1, valueIndex - keyIndex - 1).data();
            val = str.substr(valueIndex + 1, nextKey - valueIndex - 1).data();
        }

        keyIndex = nextKey;
        queries.insert_or_assign(key, val);
    }
    return queries;
}

std::string SessionBase::extractEndpoint(beast::string_view target)
{
    target.length();
    std::string ep(target.data(), target.length());
    auto index = 0;// ep.find_last_of("/api/");
    auto queryIndex = ep.find_first_of('?', index);
    if (index < ep.length())
    {
        ep = ep.substr(index, queryIndex - index).data();
        return ep;
    }
    return "";
}