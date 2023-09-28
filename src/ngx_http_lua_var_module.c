#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_http_lua_var_module.h>


static ngx_http_module_t ngx_http_lua_var_module_ctx = {
    NULL,                                    /* preconfiguration */
    NULL,                                    /* postconfiguration */

    NULL,                                    /* create main configuration */
    NULL,                                    /* init main configuration */

    NULL,                                    /* create server configuration */
    NULL,                                    /* merge server configuration */

    NULL,                                    /* create location configuration */
    NULL                                     /* merge location configuration */
};


ngx_module_t ngx_http_lua_var_module = {
    NGX_MODULE_V1,
    &ngx_http_lua_var_module_ctx,        /* module context */
    NULL,                                /*  module directives */
    NGX_HTTP_MODULE,                     /* module type */
    NULL,                                /* init master */
    NULL,                                /* init module */
    NULL,                                /* init process */
    NULL,                                /* init thread */
    NULL,                                /* exit thread */
    NULL,                                /* exit process */
    NULL,                                /* exit master */
    NGX_MODULE_V1_PADDING
};


ngx_int_t
ngx_http_lua_var_ffi_uri(ngx_http_request_t *r, ngx_str_t *uri)
{
    uri->data = r->uri.data;
    uri->len = r->uri.len;

    return NGX_OK;
}


ngx_int_t
ngx_http_lua_var_ffi_host(ngx_http_request_t *r, ngx_str_t *host)
{
    ngx_http_core_srv_conf_t  *cscf;

    if (r->headers_in.server.len) {
        host->len = r->headers_in.server.len;
        host->data = r->headers_in.server.data;
        return NGX_OK;
    }

    cscf = ngx_http_get_module_srv_conf(r, ngx_http_core_module);

    host->len = cscf->server_name.len;
    host->data = cscf->server_name.data;

    return NGX_OK;
}


ngx_int_t
ngx_http_lua_var_ffi_remote_addr(ngx_http_request_t *r, ngx_str_t *remote_addr)
{
    remote_addr->len = r->connection->addr_text.len;
    remote_addr->data = r->connection->addr_text.data;

    return NGX_OK;
}


ngx_int_t
ngx_http_lua_var_ffi_request_time(ngx_http_request_t *r, unsigned char *buf)
{
    ngx_time_t      *tp;
    ngx_msec_int_t   ms;
    int              len;

    tp = ngx_timeofday();

    ms = (ngx_msec_int_t)
             ((tp->sec - r->start_sec) * 1000 + (tp->msec - r->start_msec));
    ms = ngx_max(ms, 0);

    len = ngx_sprintf(buf, "%T.%03M", (time_t) ms / 1000, ms % 1000) - buf;
    return len;
}


ngx_int_t
ngx_http_lua_var_ffi_upstream_response_time(ngx_http_request_t *r,
    unsigned char *buf, int type)
{
    size_t                      len;
    ngx_uint_t                  i;
    ngx_msec_int_t              ms, total_ms;
    ngx_http_upstream_state_t  *state;

    if (r->upstream_states == NULL || r->upstream_states->nelts == 0) {
        return NGX_ERROR;
    }

    i = 0;
    total_ms = 0;
    state = r->upstream_states->elts;

    for ( ;; ) {
        if (type == 1) {
            ms = state[i].header_time;

        } else if (type == 2) {
            ms = state[i].connect_time;

        } else {
            ms = state[i].response_time;
        }

        ms = ngx_max(ms, 0);
        total_ms = total_ms + ms;

        if (++i == r->upstream_states->nelts) {
            break;
        }

        if (!state[i].peer) {
            if (++i == r->upstream_states->nelts) {
                break;
            }
        }
    }

    len = ngx_sprintf(buf, "%T.%03M", (time_t) total_ms / 1000,
                      total_ms % 1000) - buf;
    return len;
}


ngx_int_t
ngx_http_lua_var_ffi_scheme(ngx_http_request_t *r, ngx_str_t *scheme)
{
#if (NGX_HTTP_SSL)

    if (r->connection->ssl) {
        scheme->len = sizeof("https") - 1;
        scheme->data = (u_char *) "https";

        return NGX_OK;
    }

#endif

    scheme->len = sizeof("http") - 1;
    scheme->data = (u_char *) "http";

    return NGX_OK;
}

// 迁移开始
ngx_int_t
ngx_http_lua_var_ffi_ngx_http_variable_headers_internal(ngx_http_request_t *r,
    ngx_str_t *v, uintptr_t data, u_char sep)
{
    size_t             len;
    u_char            *p, *end;
    ngx_uint_t         i, n;
    ngx_array_t       *a;
    ngx_table_elt_t  **h;

    a = (ngx_array_t *) ((char *) r + data);

    n = a->nelts;
    h = a->elts;

    len = 0;

    for (i = 0; i < n; i++) {

        if (h[i]->hash == 0) {
            continue;
        }

        len += h[i]->value.len + 2;
    }

    if (len == 0) {
        return NGX_OK;
    }

    len -= 2;

    if (n == 1) {
        v->len = (*h)->value.len;
        v->data = (*h)->value.data;

        return NGX_OK;
    }

    p = ngx_pnalloc(r->pool, len);
    if (p == NULL) {
        return NGX_ERROR;
    }

    v->len = len;
    v->data = p;

    end = p + len;

    for (i = 0; /* void */ ; i++) {

        if (h[i]->hash == 0) {
            continue;
        }

        p = ngx_copy(p, h[i]->value.data, h[i]->value.len);

        if (p == end) {
            break;
        }

        *p++ = sep; *p++ = ' ';
    }

    return NGX_OK;
}

ngx_int_t
ngx_http_lua_var_ffi_http_cookie(ngx_http_request_t *r, ngx_str_t *http_cookie)
{
    uintptr_t data = offsetof(ngx_http_request_t, headers_in.cookies);

    return ngx_http_lua_var_ffi_ngx_http_variable_headers_internal(r, http_cookie, data, ';');
}

ngx_int_t
ngx_http_lua_var_ffi_request_method(ngx_http_request_t *r, ngx_str_t *request_method)
{
    if (r->main->method_name.data) {
        request_method->len = r->main->method_name.len;
        request_method->data = r->main->method_name.data;
    }

    return NGX_OK;
}

ngx_int_t
ngx_http_lua_var_ffi_remote_port(ngx_http_request_t *r, ngx_str_t *remote_port)
{   
    ngx_uint_t  port;

    remote_port->data = ngx_pnalloc(r->pool, sizeof("65535") - 1);
    if (remote_port->data == NULL) {
        return NGX_ERROR;
    }

    port = ngx_inet_get_port(r->connection->sockaddr);

    if (port > 0 && port < 65536) {
        remote_port->len = ngx_sprintf(remote_port->data, "%ui", port) - remote_port->data;
    }

    return NGX_OK;
}

ngx_int_t
ngx_http_lua_var_ffi_server_port(ngx_http_request_t *r, ngx_str_t *server_port)
{   
    ngx_uint_t  port;

    if (ngx_connection_local_sockaddr(r->connection, NULL, 0) != NGX_OK) {
        return NGX_ERROR;
    }

    server_port->data = ngx_pnalloc(r->pool, sizeof("65535") - 1);
    if (server_port->data == NULL) {
        return NGX_ERROR;
    }

    port = ngx_inet_get_port(r->connection->local_sockaddr);

    if (port > 0 && port < 65536) {
        server_port->len = ngx_sprintf(server_port->data, "%ui", port) - server_port->data;
    }

    return NGX_OK;
}


// 迁移ngxvar  ngx_http_variable_request 开始
ngx_int_t
ngx_http_lua_var_ffi_server_protocol(ngx_http_request_t *r, ngx_str_t *server_protocol)
{   
    uintptr_t data = offsetof(ngx_http_request_t, http_protocol);

    ngx_str_t  *s;

    s = (ngx_str_t *) ((char *) r + data);

    if (s->data) {
        server_protocol->len = s->len;
        server_protocol->data = s->data;
    }

    return NGX_OK;
}

ngx_int_t
ngx_http_lua_var_ffi_request_uri(ngx_http_request_t *r, ngx_str_t *request_uri)
{   
    uintptr_t data = offsetof(ngx_http_request_t, unparsed_uri);

    ngx_str_t  *s;

    s = (ngx_str_t *) ((char *) r + data);

    if (s->data) {
        request_uri->len = s->len;
        request_uri->data = s->data;
    }

    return NGX_OK;
}

ngx_int_t
ngx_http_lua_var_ffi_query_string(ngx_http_request_t *r, ngx_str_t *query_string)
{   
    uintptr_t data = offsetof(ngx_http_request_t, args);

    ngx_str_t  *s;

    s = (ngx_str_t *) ((char *) r + data);

    if (s->data) {
        query_string->len = s->len;
        query_string->data = s->data;
    }

    return NGX_OK;
}

// 上面有了
// ngx_int_t
// ngx_http_lua_var_ffi_uri(ngx_http_request_t *r, ngx_str_t *uri)
// {   
//     uintptr_t data = offsetof(ngx_http_request_t, uri);

//     ngx_str_t  *s;

//     s = (ngx_str_t *) ((char *) r + data);

//     if (s->data) {
//         uri->len = s->len;
//         uri->data = s->data;
//     }

//     return NGX_OK;
// }

// 迁移ngxvar  ngx_http_variable_request 结束


// 迁移ngxvar  ngx_http_variable_header 开始

ngx_int_t
ngx_http_lua_var_ffi_http_host(ngx_http_request_t *r, ngx_str_t *http_host)
{   
    uintptr_t data = offsetof(ngx_http_request_t, headers_in.host);

    ngx_table_elt_t  *h;

    h = *(ngx_table_elt_t **) ((char *) r + data);

    if (h) {
        http_host->len = h->value.len;
        http_host->data = h->value.data;
    }

    return NGX_OK;
}

ngx_int_t
ngx_http_lua_var_ffi_http_user_agent(ngx_http_request_t *r, ngx_str_t *http_user_agent)
{   
    uintptr_t data = offsetof(ngx_http_request_t, headers_in.user_agent);

    ngx_table_elt_t  *h;

    h = *(ngx_table_elt_t **) ((char *) r + data);

    if (h) {
        http_user_agent->len = h->value.len;
        http_user_agent->data = h->value.data;
    }

    return NGX_OK;
}

ngx_int_t
ngx_http_lua_var_ffi_http_referer(ngx_http_request_t *r, ngx_str_t *http_referer)
{   
    uintptr_t data = offsetof(ngx_http_request_t, headers_in.referer);

    ngx_table_elt_t  *h;

    h = *(ngx_table_elt_t **) ((char *) r + data);

    if (h) {
        http_referer->len = h->value.len;
        http_referer->data = h->value.data;
    }

    return NGX_OK;
}

ngx_int_t
ngx_http_lua_var_ffi_content_type(ngx_http_request_t *r, ngx_str_t *content_type)
{   
    uintptr_t data = offsetof(ngx_http_request_t, headers_in.content_type);

    ngx_table_elt_t  *h;

    h = *(ngx_table_elt_t **) ((char *) r + data);

    if (h) {
        content_type->len = h->value.len;
        content_type->data = h->value.data;
    }

    return NGX_OK;
}

// 迁移ngxvar  ngx_http_variable_header 结束
