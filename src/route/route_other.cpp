#include "route_other.hpp"

self::RouteOther::RouteOther(crow::SimpleApp& app) : m_app{ app } {}
void self::RouteOther::test(void){
    // 图标
    CROW_ROUTE(this->m_app, "/draw/<string>/<string>").methods(crow::HTTPMethod::Post)([&](const crow::request req, const std::string& lua_file_name, const std::string& lua_function_name) {
        json data{ json::parse(req.body.empty() ? "{}" : req.body)};
        std::exchange(data, data[0]);

        std::string 
            image_output_type{ "jpg" },
            json_data{ "null" };

        if (data.count("imageType"))image_output_type = data.at("imageType").get<std::string>();
        if (data.count("data"))json_data = data.at("data").dump();

        return self::HandleResponseBody([&]() -> std::string {
            if (Global::is_auth) {
                std::string auth{ req.get_header_value("Authorization") };
                if (auth != ("Bearer "s + Global::authorization_code)) throw HTTPException(401);
            }

            // 定义SVG代码字符串
            std::string svg_code{ }, lua_path{ Global::lua_directory / (lua_file_name + ".lua"s)};

            lua_State* L{ luaL_newstate() };
            luaL_openlibs(L);

            if (luaL_dofile(L, lua_path.c_str()) == LUA_OK) {
                // 调用 getSvgCode 函数
                lua_getglobal(L, lua_function_name.c_str());
                lua_pushstring(L, json_data.c_str());
                if (lua_pcall(L, 1, 1, 0) == LUA_OK) {
                    // 检查返回值类型
                    if (lua_isstring(L, -1)) {
                        // 读取返回值
                        svg_code = lua_tostring(L, -1);
                    }
                }
            }
            else {
                lua_close(L);
                throw self::HTTPException(500, "lua script load failure."s);
            }
            lua_close(L);

            try {
                // 创建一个Image对象
                Magick::Image image;

                std::string temp_file_name{ self::Tools::UUIDGenerator() + ".svg" }, path{ Global::svg_temp_path / temp_file_name };
                std::ofstream temp_file(path);
                if (temp_file.is_open()) {
                    temp_file << svg_code;
                }
                else {
                    temp_file.close();
                    throw std::runtime_error("unable to create temporary file.");
                }
                temp_file.close();

                // 从SVG文件中读取图像
                image.read(path);

                std::filesystem::remove(path);

                // 设置输出文件格式为PNG
                image.magick(image_output_type);

                // 将图像写入到PNG文件中
                Magick::Blob blob;
                //image.write("output.png");

                image.write(&blob);
                return std::string(reinterpret_cast<const char*>(blob.data()), blob.length());
            }
            catch (const std::exception& ex) {
                throw HTTPException(500, -1, std::string(ex.what()));
            }
            }, "image/" + image_output_type);
        });
}
void self::RouteOther::favicon(void)
{
    // 图标
    CROW_ROUTE(this->m_app, "/favicon.ico").methods(crow::HTTPMethod::Get)([&]() {
        return self::OtherService::favicon();
        });
};