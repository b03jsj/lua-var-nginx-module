local get_request   = require("resty.core.base").get_request
local get_string_buf= require("resty.core.base").get_string_buf
local ffi           = require("ffi")
local C             = ffi.C
local ffi_string    = ffi.string
local ngx           = ngx
local ngx_var       = ngx.var
local str_t         = ffi.new("ngx_str_t[1]")
local pcall         = pcall
local num_type      = {}
local tonumber      = tonumber
local str_buf       = get_string_buf(1024)


ffi.cdef([[
int ngx_http_lua_var_ffi_uri(ngx_http_request_t *r, ngx_str_t *uri);
int ngx_http_lua_var_ffi_host(ngx_http_request_t *r, ngx_str_t *host);
int ngx_http_lua_var_ffi_remote_addr(ngx_http_request_t *r,
    ngx_str_t *remote_addr);
int ngx_http_lua_var_ffi_request_time(ngx_http_request_t *r,
    unsigned char *buf);
int ngx_http_lua_var_ffi_upstream_response_time(ngx_http_request_t *r,
    unsigned char *buf, int type);
int ngx_http_lua_var_ffi_scheme(ngx_http_request_t *r, ngx_str_t *scheme);

    
int ngx_http_lua_var_ffi_server_protocol(ngx_http_request_t *r, ngx_str_t *server_protocol);
int ngx_http_lua_var_ffi_request_uri(ngx_http_request_t *r, ngx_str_t *request_uri);
int ngx_http_lua_var_ffi_query_string(ngx_http_request_t *r, ngx_str_t *query_string);
int ngx_http_lua_var_ffi_http_host(ngx_http_request_t *r, ngx_str_t *http_host);
int ngx_http_lua_var_ffi_http_user_agent(ngx_http_request_t *r, ngx_str_t *http_user_agent);
int ngx_http_lua_var_ffi_http_referer(ngx_http_request_t *r, ngx_str_t *http_referer);
int ngx_http_lua_var_ffi_content_type(ngx_http_request_t *r, ngx_str_t *content_type);
]])


local var_patched = pcall(function () return C.ngx_http_lua_var_ffi_uri end)
local vars = {
    request_method = ngx.req.get_method,
}


function vars.uri(r)
    r = r or get_request()
    if not r then
        return nil, "no request found"
    end

    C.ngx_http_lua_var_ffi_uri(r, str_t)
    return ffi_string(str_t[0].data, str_t[0].len)
end


function vars.host(r)
    r = r or get_request()
    if not r then
        return nil, "no request found"
    end

    C.ngx_http_lua_var_ffi_host(r, str_t)
    return (ffi_string(str_t[0].data, str_t[0].len))
end


function vars.status()
    return ngx.status
end


function vars.remote_addr(r)
    r = r or get_request()
    if not r then
        return nil, "no request found"
    end

    C.ngx_http_lua_var_ffi_remote_addr(r, str_t)
    return ffi_string(str_t[0].data, str_t[0].len)
end


function vars.request_time(r)
    r = r or get_request()
    if not r then
        return nil, "no request found"
    end

    local len = C.ngx_http_lua_var_ffi_request_time(r, str_buf)
    if len == 0 then
        return 0
    end

    return tonumber(ffi_string(str_buf, len))
end
num_type.request_time = true


local function upstream_response_time(r, typ)
    r = r or get_request()
    if not r then
        return nil, "no request found"
    end

    local len = C.ngx_http_lua_var_ffi_upstream_response_time(r, str_buf, typ)
    if len < 0 then
        return nil, "not found"
    end

    if len == 0 then
        return 0
    end

    return tonumber(ffi_string(str_buf, len))
end


vars.upstream_response_time = function (r)
    return upstream_response_time(r, 0)
end
num_type.upstream_response_time = true


vars.upstream_header_time = function (r)
    return upstream_response_time(r, 1)
end
num_type.upstream_header_time = true


vars.upstream_connect_time = function (r)
    return upstream_response_time(r, 2)
end
num_type.upstream_connect_time = true


function vars.scheme(r)
    r = r or get_request()
    if not r then
        return nil, "no request found"
    end

    C.ngx_http_lua_var_ffi_scheme(r, str_t)
    return ffi_string(str_t[0].data, str_t[0].len)
end

-- 新增var获取开始
function vars.server_protocol(r)
    r = r or get_request()
    if not r then
        return nil, "no request found"
    end

    C.ngx_http_lua_var_ffi_server_protocol(r, str_t)
    return ffi_string(str_t[0].data, str_t[0].len)
end

function vars.request_uri(r)
    r = r or get_request()
    if not r then
        return nil, "no request found"
    end

    C.ngx_http_lua_var_ffi_request_uri(r, str_t)
    return ffi_string(str_t[0].data, str_t[0].len)
end

function vars.query_string(r)
    r = r or get_request()
    if not r then
        return nil, "no request found"
    end

    C.ngx_http_lua_var_ffi_query_string(r, str_t)
    return ffi_string(str_t[0].data, str_t[0].len)
end

function vars.http_host(r)
    r = r or get_request()
    if not r then
        return nil, "no request found"
    end

    C.ngx_http_lua_var_ffi_http_host(r, str_t)
    return ffi_string(str_t[0].data, str_t[0].len)
end

function vars.http_user_agent(r)
    r = r or get_request()
    if not r then
        return nil, "no request found"
    end

    C.ngx_http_lua_var_ffi_http_user_agent(r, str_t)
    return ffi_string(str_t[0].data, str_t[0].len)
end

function vars.http_referer(r)
    r = r or get_request()
    if not r then
        return nil, "no request found"
    end

    C.ngx_http_lua_var_ffi_http_referer(r, str_t)
    return ffi_string(str_t[0].data, str_t[0].len)
end

function vars.content_type(r)
    r = r or get_request()
    if not r then
        return nil, "no request found"
    end

    C.ngx_http_lua_var_ffi_content_type(r, str_t)
    return ffi_string(str_t[0].data, str_t[0].len)
end


-- 新增var获取结束

for _, name in ipairs({"request_length", "bytes_sent"}) do
    vars[name] = function ()
        return tonumber(ngx_var[name])
    end
    num_type[name] = true
end


local _M = {}


function _M.request()
    local r = get_request()
    if not r then
        return nil, "no request found"
    end

    return r
end


function _M.fetch(name, request)
    local method = vars[name]

    if not var_patched or not method then
        if num_type[name] then
            return tonumber(ngx_var[name])
        end

        return ngx_var[name]
    end

    return method(request)
end


return _M
