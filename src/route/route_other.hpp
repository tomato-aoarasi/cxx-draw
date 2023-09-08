#pragma once

#include "crow.h"
#include <filesystem>
#include "string"
#include <configuration/config.hpp>
#include <common/utils.hpp>
#include <service/other/other_service.hpp>
#include "lua5.4/lua.hpp"
#include <Aspose.Words.Cpp/Document.h>
#include <Aspose.Words.Cpp/DocumentBuilder.h>
#include <Aspose.Words.Cpp/Drawing/Shape.h>
#include <Aspose.Words.Cpp/Drawing/ImageData.h>
#include <Aspose.Words.Cpp/SaveFormat.h>
#include <Aspose.Words.Cpp/Saving/IResourceSavingCallback.h>
#include <Aspose.Words.Cpp/Saving/ResourceSavingArgs.h>
#include <Aspose.Words.Cpp/Saving/SaveOutputParameters.h>
#include <Aspose.Words.Cpp/Saving/SvgSaveOptions.h>
#include <Aspose.Words.Cpp/Saving/SvgTextOutputMode.h>

namespace self {
	class RouteOther final {
	private:
		crow::SimpleApp& m_app;
	public:
		RouteOther() = delete;
		~RouteOther() = default;
		RouteOther(const RouteOther&) = delete;
		RouteOther(RouteOther&&) = delete;
		RouteOther& operator=(const RouteOther&) = delete;
		RouteOther& operator=(RouteOther&&) = delete;

		explicit RouteOther(crow::SimpleApp& app);

		std::string luaHandle(std::string_view luaFilePath, std::string_view luaFunction, json data);
		void draw(void);
        void favicon(void);
	};
}