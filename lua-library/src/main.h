#pragma once

#include <common.hpp>
#include <codecvt>
#include <memory>
#include <lua.hpp>
#include <opencv2/freetype.hpp>
#include <ft2build.h>
#include <bits/locale_conv.h>

using namespace std::string_literals;

# if 0
/* ����C++��Average ���� */
int average(lua_State* L)
{
	int num = lua_gettop(L);  //��ȡ��������

	double sum = 0;
	double avg = 0;

	for (int i = 1; i <= num; i++)   //ע��Luaջ�±��1��ʼ
	{
		sum += lua_tonumber(L, i);  //���
	}

	avg = sum / num;   //��ƽ��

	lua_pushnumber(L, avg);  //ƽ��ֵ��ջ

	lua_pushnumber(L, sum);  //����ջ

	return 2;   //return ����ֵ����
}

/* ����C++��sub���� */
int sub(lua_State* L)
{
	int num = lua_gettop(L);
	if (num != 2)
	{
		std::cout << "Input param number is not correct!" << std::endl;
		return 0;
	}

	int a = lua_tonumber(L, 1);   //��ȡ��һ��������������
	int b = lua_tonumber(L, 2);   //��ȡ�ڶ�������������

	int diff = a - b;
	lua_pushnumber(L, diff);   //���ѹ��ջ

	return 1;
}

/* ����C++��add���� */
int add(lua_State* L)
{
	int num = lua_gettop(L);
	if (num != 2)
	{
		std::cout << "Input param number is not correct!" << std::endl;
		return 0;
	}

	int a = lua_tonumber(L, 1);   //��ȡ��һ��������������
	int b = lua_tonumber(L, 2);   //��ȡ�ڶ�������������

	int sum = a + b;
	lua_pushnumber(L, sum);   //���ѹ��ջ

	return 1;
}
# endif

/* ����C++��Average ���� */
int handle_text(lua_State* L)
{
	std::string text{}, fontPath{};
	int fontSize{}, maxSize{};
	bool isRows{ false };
	// ������������
	uint8_t maxLine{ 255 };

	text = lua_tostring(L, 1);
	fontPath = lua_tostring(L, 2);
	fontSize = lua_tointeger(L, 3);
	maxSize = lua_tointeger(L, 4);

	if (!lua_isnoneornil(L, 5)) {
		isRows = lua_toboolean(L, 5);
	}

	if (!lua_isnoneornil(L, 6)) {
		maxLine = lua_tointeger(L, 6);
	}

	auto freetype2{ cv::freetype::createFreeType2() };
	freetype2->loadFontData(fontPath, 0);

	// ����
	if (isRows) {
		// *��ʼ*

		std::vector<std::string> texts{ };
		// ����\n\r
		if (text.find("\r\n") != std::string::npos) {
			texts = utils::split(text, "\r\n");
		}// ����\n\r
		else if (text.find("\n\r") != std::string::npos) {
			texts = utils::split(text, "\n\r");
		}// ����\n
		else {
			texts = utils::split(text, "\n");
		}

		std::vector<std::string> texts_handled{ };

		// ���std:string �� λ��
		std::vector<std::string>temps{};

		// ��ǰ��
		uint8_t current_line{ 0 };

		// �ַ���ת����Ҫ
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

		for (const auto& element : texts) {
			if (current_line >= maxLine) { break; }
			// ���flagΪtrue˵���˶��в���
			bool flag{ false };

			int offset_w{ freetype2->getTextSize(element, fontSize, -1, nullptr).width };

			std::wstring wstr{ converter.from_bytes(element) };

			for (int i = 0; i < wstr.length(); i++) {
				std::string temp = converter.to_bytes(wstr.substr(0, i + 1));
				cv::Size text_size = freetype2->getTextSize(temp, fontSize, -1, nullptr); // ��ȡ�ı��Ĵ�С
				if (text_size.width > maxSize) { // ����ı���ȳ��������
					if (current_line >= maxLine) { break; }
					temps.emplace_back(converter.to_bytes(wstr.substr(0, i)));
					wstr = wstr.substr(i); // ʣ���ı�
					i = 0; // ���¿�ʼ����
					flag = true;
					++current_line;
				}
			}

			if (current_line < maxLine and flag) {
				temps.emplace_back(converter.to_bytes(wstr));
				wstr.clear();
				++current_line;
			}

			// һ��vector���зָ�
			if (flag) {
				for (const auto& str : temps) {
					texts_handled.emplace_back(str);
				}
				temps.clear();
			}
			// һ��vectorһ�зָ�
			else {
				texts_handled.emplace_back(element);
				++current_line;
			}
		}

		lua_newtable(L);//����һ����񣬷���ջ��
		
		int count{ 1 };
		for (const auto& text : texts_handled) {
			lua_pushnumber(L, count);
			lua_pushstring(L, text.c_str());//ѹ��key
			lua_settable(L, -3);
			++count;
		}
		// *����*
	}
	// ����
	else {
		// *��ʼ*

		int offset_w{ freetype2->getTextSize(text, fontSize, -1, nullptr).width };

		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		while (offset_w > maxSize) {
			std::wstring wtext{ converter.from_bytes(text) };

			wtext.pop_back();

			text = converter.to_bytes(wtext);
			std::string temp{ text + "..."s };
			offset_w = freetype2->getTextSize(temp, fontSize, -1, nullptr).width;

			if (offset_w <= maxSize) {
				text = temp;
				break;
			}
		}

		lua_pushstring(L, text.c_str());
		// *����*
	}

	return 1;
}

extern "C" int luaopen_libluadraw(lua_State * L);