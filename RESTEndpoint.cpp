#include "StdAfx.h"
#include "RESTEndpoint.h"


std::shared_ptr<JSONInfoBody> ResourceRequestHandler::handleRequest_Get(std::string target, ParameterMap queries, ParameterMap body, TRESTCtxPtr pCtx)
{
	if (!pCtx) return nullptr;
	auto pWebPageBody = std::make_shared<ResourceInfoBody>();
	pWebPageBody->setPath(pCtx->findResource(target));
	return pWebPageBody;
}


