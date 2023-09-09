#pragma once

#include "crow.h"
#include <filesystem>
#include "string"
#include <configuration/config.hpp>
#include <common/utils.hpp>
#include <service/other/other_service.hpp>
#include "lua5.4/lua.hpp"

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

self::RouteOther::RouteOther(crow::SimpleApp& app) : m_app{ app } {}
std::string self::RouteOther::luaHandle(std::string_view luaFilePath, std::string_view luaFunction, json data)
{
	std::string result;
	lua_State* L{ luaL_newstate() };
	luaL_openlibs(L);

	if (luaL_dofile(L, luaFilePath.data()) == LUA_OK) {
		// ���� getSvgCode ����
		lua_getglobal(L, luaFunction.data());
		lua_pushstring(L, data.dump().c_str());
		if (lua_pcall(L, 1, 1, 0) == LUA_OK) {
			// ��鷵��ֵ����
			if (lua_isstring(L, -1)) {
				// ��ȡ����ֵ
				result = lua_tostring(L, -1);
			}
		}
	}
	else {
		lua_close(L);
		throw self::HTTPException(500, "lua script load failure."s);
	}
	lua_close(L);
	return result;
}
void self::RouteOther::draw(void) {
	// ͼ��
	CROW_ROUTE(this->m_app, "/draw/<string>/<string>").methods(crow::HTTPMethod::Post)([&](const crow::request req, const std::string& lua_file_name, const std::string& lua_function_name) {
		json data{ json::parse(req.body.empty() ? "{}" : req.body) }, json_data{};
		std::exchange(data, data[0]);

		std::string
			image_output_type{ "jpg" };

		if (data.count("imageType"))image_output_type = data.at("imageType").get<std::string>();
		if (data.count("data"))json_data = data.at("data");

		return self::HandleResponseBody([&]() -> std::string {
			if (Global::is_auth) {
				std::string auth{ req.get_header_value("Authorization") };
				if (auth != ("Bearer "s + Global::authorization_code)) throw HTTPException(401);
			}

			// �ϴ��ļ�
			std::vector<std::string> temp_filename_vec{};
			bool is_upload_file{ false };
			if (data.count("uploadImage")) {
				for (const auto& element : data.at("uploadImage"))
				{
					std::string temp_filename{ self::Tools::UUIDGenerator() + ".png" }, path{ Global::temp_path / temp_filename };
					self::Tools::saveBase64Image(element.get<std::string>(), path);
					temp_filename_vec.emplace_back(path);
				}
				json_data["__imagePath"] = temp_filename_vec;
				is_upload_file = true;
			}

			// ����SVG�����ַ���
			std::string lua_path{ Global::lua_directory / (lua_file_name + ".lua"s) };

			std::string return_data{ luaHandle(lua_path, lua_function_name, json_data) };

			json jsondata{ json::parse(std::move(return_data)) };

			std::exchange(jsondata, jsondata.at(0));

			if (jsondata.at("status").get<int>() != 0) throw HTTPException(jsondata.at("data").at("msg").get<std::string>(), jsondata.at("data").at("code").get<uint16_t>(), jsondata.at("data").at("extra"), jsondata.at("status").get<int>());

			try {
				// ����һ��Image����
				Magick::Image image;

				std::string temp_file_name{ self::Tools::UUIDGenerator() + ".svg" }, path{ Global::temp_path / temp_file_name };
				std::ofstream temp_file(path);
				if (temp_file.is_open()) {
					temp_file << jsondata.at("code").get<std::string>();
				}
				else {
					temp_file.close();
					throw std::runtime_error("unable to create temporary file.");
				}
				temp_file.close();

				std::string result{};

				if (Global::drawMode == 0) {
					Magick::Blob blob;
					// ��SVG�ļ��ж�ȡͼ��
					image.read(path);

					// ��������ļ���ʽΪPNG
					image.magick(image_output_type);

					// ��ͼ��д�뵽PNG�ļ���
					//image.write("output.png");

					image.write(&blob);

					result = std::string(reinterpret_cast<const char*>(blob.data()), blob.length());
				}
				else if (Global::drawMode == 1) {
					auto doc{ System::MakeObject<Aspose::Words::Document>() };

					if (Global::isSetFontFolder) {
						doc->set_FontSettings(Global::fontSettings);
					}

					// ����ȱʧ����
					auto substitutionWarningHandler = System::MakeObject<LogSystem::HandleDocumentSubstitutionWarnings>();
					doc->set_WarningCallback(substitutionWarningHandler);

					auto builder{ System::MakeObject<Aspose::Words::DocumentBuilder>(doc) };

					std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
					std::u16string u16Path = convert.from_bytes(path);

					auto shape{ builder->InsertImage(System::String(u16Path)) };

					auto bytes{ shape->get_ImageData()->get_ImageBytes()->data() };
					shape.reset();
					builder.reset();
					doc.reset();

					result = std::string(bytes.begin(), bytes.end());
				}
				else { throw HTTPException("This drawing type doesn't exist"s, 500); }

				// ɾ����ʱ�ļ�
				{
					std::filesystem::remove(path);
					for (const auto& imgpath : temp_filename_vec) {
						std::filesystem::remove(imgpath);
					}
				}

				// �����lua����
				if (jsondata.at("extraControl").get<bool>()) {
					luaHandle(jsondata.at("data").at("luaFile").get<std::string>(), jsondata.at("data").at("luaFunction").get<std::string>(), jsondata.at("data").at("content"));
				}

				return result;
			}
			catch (const std::exception& ex) {
				throw HTTPException(500, -1, std::string(ex.what()));
			}
			}, "image/" + image_output_type);
		});
}
void self::RouteOther::favicon(void)
{
	// ͼ��
	CROW_ROUTE(this->m_app, "/favicon.ico").methods(crow::HTTPMethod::Get)([&]() {
		return self::OtherService::favicon();
		});
};