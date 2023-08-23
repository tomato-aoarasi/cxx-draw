#pragma once

#include <iostream>
#include <fmt/core.h>
#include <fstream>
#include <memory>
#include <exception>
#include <stdexcept>
#include <utility>
#include <filesystem>
#include <locale>
#include <codecvt>
#include <string>
#include <span>
#include <jwt-cpp/jwt.h>
#include <sqlite_modern_cpp.h>
#include "crow.h"
#include "tl/generator.hpp"

#if ENABLED_ASYNCIO
#include <asyncio/task.h>
#include <asyncio/runner.h>
#include <asyncio/sleep.h>
#include <asyncio/schedule_task.h>
#include <asyncio/callstack.h>
#endif

#include <Poco/Net/MediaType.h>
#include <Poco/Net/MailMessage.h>
#include <Poco/Net/SecureSMTPClientSession.h>
#include <Poco/Net/NetException.h>
#include <snowflake.hpp>
#include "Poco/UUID.h"
#include "Poco/UUIDGenerator.h"
#include "Poco/DigestEngine.h"
#include "Poco/SHA2Engine.h"
#include "Poco/SHA1Engine.h"
#include "lua5.4/lua.hpp"

#ifndef TEST_PROJECT_HPP
#define TEST_PROJECT_HPP  



//#define WARNING_CONTENT  
using _uint64 = unsigned long long int;

tl::generator<int> generateInts()
{
	for (int i = 0; i < 10; ++i)
	{
		co_yield i;
	}
};

#if ENABLED_ASYNCIO
asyncio::Task<std::string_view> hello() {
	co_return "hello";
}

asyncio::Task<std::string_view> world() {
	co_return "world";
}

asyncio::Task<std::string> hello_world() {
	co_return fmt::format("{} {}", co_await hello(), co_await world());
	fmt::print("{}", asyncio::run(world()));
}
#endif

namespace fs = std::filesystem;

class TestProject final {
public:
#if ENABLED_ASYNCIO
	inline static void coroutine_test() {
		StartWatch();
		auto completes_synchronously = [](int args) -> asyncio::Task<int> {
			co_return args;
		};

		auto main = [&](const int N) ->  asyncio::Task<int> {
			int sum = 0;
			for (int i = 0; i < N; ++i) {
				sum += co_await completes_synchronously(3);
			}
			co_return sum;
		};

		fmt::print("{}\n", asyncio::run(main(10)));
		StopWatch();
	};
#endif

	inline static void uuid_test() {
		using snowflake_t = snowflake<-1L, std::mutex>;
		snowflake_t::twepoch = 1690029136L * 1000L;
		snowflake_t snowflake_uuid;
		snowflake_uuid.init(1, 1);
		auto id = snowflake_uuid.nextid();

		Poco::UUIDGenerator uuid_generator{};

		std::cout << Poco::UUID("00000000-0000-0000-0000-000000000000").toString() << std::endl;

		//Poco::DigestEngine* engine{ new Poco::MD5Engine() };
		std::unique_ptr<Poco::DigestEngine> engine{ new Poco::SHA1Engine() };
		Poco::UUID uuid{ uuid_generator.createFromName(Poco::UUID("00000000-0000-0000-0000-000000000000"), std::to_string(id), *engine, Poco::UUID::UUID_NAME_BASED_SHA1) };

		engine.reset();

		std::cout << uuid.toString() << std::endl;
		std::cout << id << "\n";
	}

#define _CODE1 
#define CODE2 
#define _CODE3 

	inline static int LuaTest() {
        lua_State* L = luaL_newstate();
        luaL_openlibs(L);
        int retLoad = luaL_loadfile(L, "lua/main.lua");
        if (retLoad == 0)
        {
            printf("load file success retLoad:%d\n", retLoad);
        }
        if (retLoad || lua_pcall(L, 0, 0, 0))
        {
            printf("error %s\n", lua_tostring(L, -1));
            return -1;
        }

#ifdef CODE1
        //printf("调用lua中的变量\n");
        lua_getglobal(L, "name"); //lua获取全局变量name的值并且返回到栈顶
        lua_getglobal(L, "age");  //lua获取全局变量age的值并且返回到栈顶,这个时候length对应的值将代替width的值成为新栈顶
        //注意读取顺序
        int age = lua_tointeger(L, -1);
        const char* name = lua_tostring(L, -2);
        printf("name = %s\n", name); //栈顶
        printf("age = %d\n", age);   //次栈顶

#endif

#ifdef CODE2
        // printf("调用lua中的函数\n");

        //调用函数,依次压入参数
        lua_getglobal(L, "add");
        lua_pushnumber(L, 10);
        lua_pushnumber(L, 20);
        //查看压入栈的元素
        for (int i = 1; i < 3; i++)
        {
            printf("number:%f\n", lua_tonumber(L, -i));
        }
        //lua_pcall(L,2,1,0):传入两个参数 期望得到一个返回值，0表示错误处理函数在栈中的索引值，压入结果前会弹出函数和参数
        int pcallRet = lua_pcall(L, 2, 1, 0); //lua_pcall将计算好的值压入栈顶,并返回状态值
        if (pcallRet != 0)
        {
            printf("error %s\n", lua_tostring(L, -1));
            return -1;
        }

        printf("pcallRet:%d\n", pcallRet);
        int val = lua_tonumber(L, -1); //在栈顶取出数据
        printf("val:%d\n", val);
        lua_pop(L, -1); //弹出栈顶
        //再次查看栈内元素，发现什么都没有，因为lua在返回函数计算值后会清空栈,只保留返回值 
        for (int i = 1; i < 3; i++)
        {
            printf("number:%f\n", lua_tonumber(L, -i));
        }
#endif

#ifdef CODE3
        // printf("读取lua table中对应的值\n");
        //将全局变量mytable压入栈
        lua_getglobal(L, "mytable");
        //压入表中的key
        lua_pushstring(L, "name");

        //lua_gettable会在栈顶取出一个元素并且返回把查找到的值压入栈顶
        lua_gettable(L, 1);
        const char* name = lua_tostring(L, -1); //在栈顶取出数据
        printf("name:%s\n", name);

        lua_pushstring(L, "id");//压入id
        lua_gettable(L, 1);//在lua mytable表中取值返回到栈顶
        int id = lua_tonumber(L, -1); //在栈顶取出数据
        printf("id:%d\n", id);
#endif

        lua_close(L);
		return 0;
	}

    inline static int svg_test() {
        // 定义SVG代码字符串
        std::string svg_code{ R"(<svg version="1.1"
baseProfile = "full"
width = "100" height = "100"
xmlns = "http://www.w3.org/2000/svg">
    <rect x="0" y="0" width="100" height="100" fill="red" />
    <image x = "0" y = "0" width = "100" height = "100" href = "./public/0_score.png" />
</svg>)" };

        try {
            // 创建一个Image对象
            Magick::Image image;

            std::string temp_file_name{ self::Tools::UUIDGenerator() + ".svg" }, path{ Global::svg_temp_path / temp_file_name };
            std::cout << path << std::endl;
            std::ofstream temp_file(path);
            if (temp_file.is_open()) {
                temp_file << svg_code;
            }
            else {
                temp_file.close();
                throw std::runtime_error("无法创建临时文件");
            }
            temp_file.close();

            // 从SVG文件中读取图像
            image.read(path);

            std::filesystem::remove(path);

            // 设置输出文件格式为PNG
            image.magick("PNG");

            // 将图像写入到PNG文件中
            Magick::Blob blob;
            //image.write("output.png");

            image.write(&blob);

            std::vector<uint8_t> data(blob.length());

            // 将Magick::Blob对象中的数据拷贝到std::vector<uint8_t>中
            const uint8_t* src = reinterpret_cast<const uint8_t*>(blob.data());
            std::copy(src, src + blob.length(), data.begin());

            self::Tools::HexDebug(data);
        }
        catch (const std::exception& ex) {
            std::cerr << "转换失败: " << ex.what() << std::endl;
            return 1;
        }
        return 0;
    };

    inline static int snowflake_test(void) {
        for (size_t row{ 0 }; row < 10; ++row) {
            auto id{ Global::snowflake_uuid.nextid() };
            fmt::print("snowflake id: {}\n", id);
        }

        for (size_t row{ 0 }; row < 10; ++row) {
            fmt::print("uuid: {}\n", self::Tools::UUIDGenerator());
        }
        return 0;
    }

	inline static void test(void) {
		StartWatch();

		const int status{ svg_test() };
		fmt::print("\nstatus: {}\n", status);

		StopWatch();
	};
private:
	inline static std::chrono::system_clock::time_point start, end;

	inline static void StartWatch(void) {
		std::cout << "\033[42mstart watch\033[0m\n";
		start = std::chrono::high_resolution_clock::now();
	};

	inline static void StopWatch(void) {
		end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed = end - start;
		std::cout << "\033[42mtime consumed " << elapsed.count() << " ms\033[0m\n";
		std::cout.flush();
	};
};

#endif