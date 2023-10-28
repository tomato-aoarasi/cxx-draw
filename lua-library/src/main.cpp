#include <main.h>

//ʹ��luaL_Regע���µ�C������Lua��
static luaL_Reg lua_draw_functions[] =
{
	{"handle_text", handle_text},
	{"get_text_size", get_text_size},
	{NULL, NULL}  // ���������һ�Ա�����{NULL, NULL}��������ʾ����
};


/* luaopen_XXX��XXXΪ�����ƣ���������Ϊtestlib.so��XXX��Ϊtestlib */
int luaopen_libluadraw(lua_State* L)
{
	luaL_newlib(L, lua_draw_functions);  //Lua 5.2֮����luaL_newlib������luaL_register
	return 1;
}