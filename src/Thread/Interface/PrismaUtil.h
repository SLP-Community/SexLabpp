#pragma once
#include "PrismaUI_API.h"

namespace Thread::PrismaUI
{
	inline PRISMA_UI_API::IVPrismaUI2* PrismaAPI{ nullptr };
	inline PRISMA_UI_API::IVPrismaUI2* GetAPI() { return PrismaAPI; }
	inline bool IsAvailable() { return PrismaAPI != nullptr; }

	inline bool Initialize()
	{
		PrismaAPI = static_cast<PRISMA_UI_API::IVPrismaUI2*>(PRISMA_UI_API::RequestPluginAPI(PRISMA_UI_API::InterfaceVersion::V2));
		return (PrismaAPI ? true : false);
	}

	inline PrismaView CreateView(const char* htmlPath)
	{
		if (!IsAvailable()) return 0;
		const auto view = PrismaAPI->CreateView(htmlPath);
		if (view) PrismaAPI->Hide(view);
		return view;
	}

	inline std::string JsonEscape(const std::string& s)
	{
		std::string out;
		out.reserve(s.size());
		for (char c : s) {
			if (c == '"') out += "\\\"";
			else if (c == '\\') out += "\\\\";
			else out += c;
		}
		return out;
	}
}