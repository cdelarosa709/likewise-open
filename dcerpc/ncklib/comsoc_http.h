#ifndef _COMSOC_HTTP_H
#define _COMSOC_HTTP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef _DCE_PROTOTYPE_
#include <dce/dce.h>
#endif

#include <comsoc.h>

EXTERNAL rpc_socket_vtbl_t rpc_g_http_socket_vtbl;

#endif
