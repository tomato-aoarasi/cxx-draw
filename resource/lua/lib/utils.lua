local json = require("cjson")
local http = require("socket.http")
local ltn12 = require("ltn12")

utils = {}

utils.authorization = ""
utils.url = "http://127.0.0.1:8555"

local uri = {
   gaussianBlur = "/utils/GaussianBlur"
}

function utils.gaussianBlur(filePath, saveDir, blurArgs)
    saveDir = saveDir or nil
    blurArgs.sigmaX = blurArgs.sigmaX or nil
    blurArgs.sigmaY = blurArgs.sigmaY or nil
    blurArgs.ksizeX = blurArgs.ksizeX or nil
    blurArgs.ksizeY = blurArgs.ksizeY or nil
    blurArgs.imageType = blurArgs.imageType or nil
    blurArgs.borderType = blurArgs.borderType or nil

    local data = {
        filePath = filePath,
        saveDir = saveDir,
        sigmaX = blurArgs.sigmaX,
        sigmaY = blurArgs.sigmaY,
        ksizeX = blurArgs.ksizeX,
        ksizeY = blurArgs.ksizeY,
        imageType = blurArgs.imageType,
        borderType = blurArgs.borderType
    }

    local payload = json.encode(data)

    local headers = {
        ["Content-Type"] = "application/json",
        ["Authorization"] = "Bearer " .. utils.authorization,
        ["Content-Length"] = #payload;
    }

    local url = utils.url .. uri.gaussianBlur

    local response_body = {}
    local result, status_code, response_headers = http.request{
        url = url,
        method = "POST",
        headers = headers,
        source = ltn12.source.string(payload),
        sink = ltn12.sink.table(response_body)
    }

    return json.decode(table.concat(response_body)), status_code
end

function utils.gaussianBlurSimple(filePath, sigmaX, imageType)
    sigmaX = sigmaX or nil
    imageType = imageType or nil
    return utils.gaussianBlur(filePath, nil, {sigmaX = sigmaX, imageType = imageType})
end

function utils.timestampToDate(timeStamp)
    return os.date("%Y-%m-%d %H:%M:%S", timeStamp)
end

function utils.isnan(value)
    return value ~= value
end

function utils.escapeHtml(str)
    str = string.gsub(str, "&", "&amp;")
    str = string.gsub(str, "<", "&lt;")
    str = string.gsub(str, ">", "&gt;")
    str = string.gsub(str, "\"", "&quot;")
    str = string.gsub(str, "'", "&#39;")
    return str
end

-- 转为SVG的正确比率值
function utils.toSVGScale(value)
    return value * 4 / 3
end

function utils.format(fmt, ...)
    local function string_count(str, match)
        local count = 0

        for match in string.gmatch(str, match) do
            count = count + 1
        end
        return count
    end

    local arg = {...}

    local appoint_index = string.find(fmt, "{0}")

    if appoint_index ~= nil then
        for index = 1, #arg do
            local argv = "{" .. tostring(index - 1) .. "}"
            local count = string_count(fmt, argv)
            if count < 1 then
                break
            end
            fmt = string.gsub(fmt, argv, tostring(arg[index]))
        end
    else
        for index = 1, #arg do
            fmt = string.gsub(fmt, "{}", tostring(arg[index]), 1)
        end
    end
    return fmt
end

return utils