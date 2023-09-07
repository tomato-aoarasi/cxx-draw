local json = require("cjson")

export = {}

function export.encode(status, svg_code, data, extra_control)
    status = status or 0
    svg_code = svg_code or ""
    data = data or nil
    extra_control = extra_control or false

    payload = json.encode({
	status = status,
	code = svg_code,
	data = data,
	extraControl = extra_control
    })

    return tostring(payload)
end

function export.error(status, code, msg, extra)
    extra = extra or {}
    msg = msg or ""

    return export.encode(status, "", {msg = msg, code = code, extra = extra}, false)
end

function export.complete_control(svg_code, lua_file, lua_function , content)
    local data = {
        luaFile = lua_file,
        luaFunction = lua_function,
        content = content or {}
    }
    return export.encode(0, svg_code, data, true)
end

function export.success_control(svg_code)
    return export.encode(0, svg_code)
end

return export