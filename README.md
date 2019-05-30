lua-var-nginx-module
====================

Fetchs Nginx variable by Luajit with FFI way which is fast and cheap.

Compares to `ngx.var.*`, performance has increased by more than five times. ^_^


Table of Contents
=================
* [Install](#install)
* [Methods](#methods)
    * [request](#request)
    * [fetch](#fetch)
* [TODO](#todo)

Method
======

### request

`syntax: req = ngxvar.request()`

Returns the request object of current request. We can cache it at your Lua code
land if we try to fetch more than one variable in one request.

[Back to TOC](#table-of-contents)

### fetch

`syntax: val = ngxvar.fetch(name, req)`

Returns the Nginx variable value by name.

```nginx
 location /t {
     content_by_lua_block {
         local var = require("resty.ngxvar")
         local req = var.fetch("_request")

         ngx.say(var.fetch("host", req))
         ngx.say(var.fetch("uri", req))
     }
 }
```

[Back to TOC](#table-of-contents)

TODO
====

* support more variables.

[Back to TOC](#table-of-contents)
