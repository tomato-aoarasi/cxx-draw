#pragma once

#include "crow.h"
#include <filesystem>
#include "string"
#include <configuration/config.hpp>
#include <common/utils.hpp>
#include <service/other/other_service.hpp>

namespace self {
	class RouteUtils final {
	private:
		crow::SimpleApp& m_app;
	public:
		RouteUtils() = delete;
		~RouteUtils() = default;
		RouteUtils(const RouteUtils&) = delete;
		RouteUtils(RouteUtils&&) = delete;
		RouteUtils& operator=(const RouteUtils&) = delete;
		RouteUtils& operator=(RouteUtils&&) = delete;

		explicit RouteUtils(crow::SimpleApp& app);

		void gaussianBlur(void);
	};
};