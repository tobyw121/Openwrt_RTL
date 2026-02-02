/*
 * libuci plugin for Lua
 * Copyright (C) 2008 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <regex.h>
#include <memory.h>
#include <ctype.h>

#include <lauxlib.h>
#include <uci.h>

// 处理非法字符开头的标签名
#define JOINT_HEAD_STR "ILLEGAL_FIRST_CHAR_"
#define OPTION  (0)
#define START   (1)
#define END		(2)
#define BUF_LEN	(256)

#define MODNAME        "uci"
#define METANAME       MODNAME ".meta"
#define LOCAL_MERGE	   (1)
#define MERGE 		   (2)
#define CON_COPY 	   (0) 
#define PKG_COPY 	   (1) 
#define SEC_COPY 	   (2) 
#define OPT_COPY 	   (3) 
//#define DEBUG 1

#ifdef DEBUG
#define DPRINTF(s, a...) fprintf(stderr,"[Lua UCI] %s[%d] " s "\n", __FUNCTION__, __LINE__, ##a)
#else
#define DPRINTF(...) do {} while (0)
#endif

static int  delBlockFlag       = 0;  // pkg或section标识，若名称不合法则整块删除
static char blockName[BUF_LEN] = {0};
static struct uci_context *global_ctx = NULL;

#ifdef ENABLE_REMODEL

const xmlChar* REMODEL_NAME_01 = (xmlChar*)"1";    // remodel patten with space ' ', for example: 'Archer AX20'
const xmlChar* REMODEL_NAME_02 = (xmlChar*)"2";    // remodel patten with underscore '_', for example: "Archer_AX20"
const xmlChar* REMODEL_NAME_03 = (xmlChar*)"3";    // remodel patten with hyphen '-', for example: "Archer-AX20"
const xmlChar* REMODEL_NAME_04 = (xmlChar*)"4";    // remodel patten with hyphen '', for example: "ArcherAX20"
const xmlChar* REMODEL_NAME_MAX = (xmlChar*)"max";

const xmlChar* REMODEL_VER_01 = (xmlChar*)"1";    // remodel patten of version "1"
const xmlChar* REMODEL_VER_02 = (xmlChar*)"2";    // remodel patten of version "1.0"
const xmlChar* REMODEL_VER_03 = (xmlChar*)"3";    // remodel patten of version "1.0.0"
const xmlChar* REMODEL_VER_MAX = (xmlChar*)"max";

#define REMODEL_PARA_LEN 128

#endif 

static struct uci_context *
find_context(lua_State *L, int *offset)
{
	struct uci_context **ctx;
	if (!lua_isuserdata(L, 1)) {
		if (!global_ctx) {
			global_ctx = uci_alloc_context();
			if (!global_ctx) {
				luaL_error(L, "failed to allocate UCI context");
				return NULL;
			}
		}
		if (offset)
			*offset = 0;
		return global_ctx;
	}
	if (offset)
		*offset = 1;
	ctx = luaL_checkudata(L, 1, METANAME);
	if (!ctx || !*ctx) {
		luaL_error(L, "failed to get UCI context");
		return NULL;
	}

	return *ctx;
}

static struct uci_package *
find_package(lua_State *L, struct uci_context *ctx, const char *str, bool al)
{
	struct uci_package *p = NULL;
	struct uci_element *e;
	char *sep;
	char *name;

	sep = strchr(str, '.');
	if (sep) {
		name = malloc(1 + sep - str);
		if (!name) {
			luaL_error(L, "out of memory");
			return NULL;
		}
		strncpy(name, str, sep - str);
		name[sep - str] = 0;
	} else
		name = (char *) str;

	uci_foreach_element(&ctx->root, e) {
		if (strcmp(e->name, name) != 0)
			continue;

		p = uci_to_package(e);
		goto done;
	}

	if (al == true)
		uci_load(ctx, name, &p);
	else if (al) {
		uci_load(ctx, name, &p);
	}

done:
	if (name != str)
		free(name);
	return p;
}

static int
lookup_args(lua_State *L, struct uci_context *ctx, int offset, struct uci_ptr *ptr, char **buf)
{
	char *s = NULL;
	int n;

	n = lua_gettop(L);
	luaL_checkstring(L, 1 + offset);
	s = strdup(lua_tostring(L, 1 + offset));
	if (!s)
		goto error;

	memset(ptr, 0, sizeof(struct uci_ptr));
	if (!find_package(L, ctx, s, true))
		goto error;

	switch (n - offset) {
	case 4:
	case 3:
		ptr->option = luaL_checkstring(L, 3 + offset);
		/* fall through */
	case 2:
		ptr->section = luaL_checkstring(L, 2 + offset);
		ptr->package = luaL_checkstring(L, 1 + offset);
		if (uci_lookup_ptr(ctx, ptr, NULL, true) != UCI_OK)
			goto error;
		break;
	case 1:
		if (uci_lookup_ptr(ctx, ptr, s, true) != UCI_OK)
			goto error;
		break;
	default:
		luaL_error(L, "invalid argument count");
		goto error;
	}

	*buf = s;
	return 0;

error:
	if (s)
		free(s);
	return 1;
}

static int
uci_push_status(lua_State *L, struct uci_context *ctx, bool hasarg)
{
	char *str = NULL;

	if (!hasarg)
		lua_pushboolean(L, (ctx->err == UCI_OK));
	if (ctx->err) {
		uci_get_errorstr(ctx, &str, MODNAME);
		if (str) {
			lua_pushstring(L, str);
			free(str);
			return 2;
		}
	}
	return 1;
}

static void
uci_push_option(lua_State *L, struct uci_option *o)
{
	struct uci_element *e;
	int i = 0;

	switch(o->type) {
	case UCI_TYPE_STRING:
		lua_pushstring(L, o->v.string);
		break;
	case UCI_TYPE_LIST:
		lua_newtable(L);
		uci_foreach_element(&o->v.list, e) {
			i++;
			lua_pushstring(L, e->name);
			lua_rawseti(L, -2, i);
		}
		break;
	default:
		lua_pushnil(L);
		break;
	}
}

static void
uci_push_section(lua_State *L, struct uci_section *s, int index)
{
	struct uci_element *e;

	lua_newtable(L);
	lua_pushboolean(L, s->anonymous);
	lua_setfield(L, -2, ".anonymous");
	lua_pushstring(L, s->type);
	lua_setfield(L, -2, ".type");
	lua_pushstring(L, s->e.name);
	lua_setfield(L, -2, ".name");
	if (index >= 0) {
		lua_pushinteger(L, index);
		lua_setfield(L, -2, ".index");
	}

	uci_foreach_element(&s->options, e) {
		struct uci_option *o = uci_to_option(e);
		uci_push_option(L, o);
		lua_setfield(L, -2, o->e.name);
	}
}

static void
uci_push_package(lua_State *L, struct uci_package *p)
{
	struct uci_element *e;
	int i = 0;

	lua_newtable(L);
	uci_foreach_element(&p->sections, e) {
		uci_push_section(L, uci_to_section(e), i);
		lua_setfield(L, -2, e->name);
		i++;
	}
}

static int
uci_lua_unload(lua_State *L)
{
	struct uci_context *ctx;
	struct uci_package *p;
	const char *s;
	int offset = 0;

	ctx = find_context(L, &offset);
	luaL_checkstring(L, 1 + offset);
	s = lua_tostring(L, 1 + offset);
	p = find_package(L, ctx, s, false);
	if (p) {
		uci_unload(ctx, p);
		return uci_push_status(L, ctx, false);
	} else {
		lua_pushboolean(L, 0);
	}
	return 1;
}

static int
uci_lua_load(lua_State *L)
{
	struct uci_context *ctx;
	struct uci_package *p = NULL;
	const char *s;
	int offset = 0;

	ctx = find_context(L, &offset);
	uci_lua_unload(L);
	lua_pop(L, 1); /* bool ret value of unload */
	s = lua_tostring(L, -1);

	uci_load(ctx, s, &p);
	return uci_push_status(L, ctx, false);
}


static int
uci_lua_foreach(lua_State *L)
{
	struct uci_context *ctx;
	struct uci_package *p;
	struct uci_element *e, *tmp;
	const char *package, *type;
	bool ret = false;
	int offset = 0;
	int i = 0;

	ctx = find_context(L, &offset);
	package = luaL_checkstring(L, 1 + offset);

	if (lua_isnil(L, 2 + offset))
		type = NULL;
	else
		type = luaL_checkstring(L, 2 + offset);

	if (!lua_isfunction(L, 3 + offset) || !package)
		return luaL_error(L, "Invalid argument");

	p = find_package(L, ctx, package, true);
	if (!p)
		goto done;

	uci_foreach_element_safe(&p->sections, tmp, e) {
		struct uci_section *s = uci_to_section(e);

		i++;

		if (type && (strcmp(s->type, type) != 0))
			continue;

		lua_pushvalue(L, 3 + offset); /* iterator function */
		uci_push_section(L, s, i - 1);
		if (lua_pcall(L, 1, 1, 0) == 0) {
			ret = true;
			if (lua_isboolean(L, -1) && !lua_toboolean(L, -1))
				break;
		}
		else
		{
			lua_error(L);
			break;
		}
	}

done:
	lua_pushboolean(L, ret);
	return 1;
}

static int
uci_lua_get_any(lua_State *L, bool all)
{
	struct uci_context *ctx;
	struct uci_element *e = NULL;
	struct uci_ptr ptr;
	int offset = 0;
	char *s = NULL;
	int err = UCI_ERR_NOTFOUND;

	ctx = find_context(L, &offset);

	if (lookup_args(L, ctx, offset, &ptr, &s))
		goto error;

	uci_lookup_ptr(ctx, &ptr, NULL, true);
	if (!all && !ptr.s) {
		err = UCI_ERR_INVAL;
		goto error;
	}
	if (!(ptr.flags & UCI_LOOKUP_COMPLETE)) {
		err = UCI_ERR_NOTFOUND;
		goto error;
	}

	err = UCI_OK;
	e = ptr.last;
	switch(e->type) {
		case UCI_TYPE_PACKAGE:
			uci_push_package(L, ptr.p);
			break;
		case UCI_TYPE_SECTION:
			if (all)
				uci_push_section(L, ptr.s, -1);
			else
				lua_pushstring(L, ptr.s->type);
			break;
		case UCI_TYPE_OPTION:
			uci_push_option(L, ptr.o);
			break;
		default:
			err = UCI_ERR_INVAL;
			goto error;
	}
	if (!err)
		return 1;

error:
	if (s)
		free(s);

	lua_pushnil(L);
	return uci_push_status(L, ctx, true);
}

static int
uci_lua_get(lua_State *L)
{
	return uci_lua_get_any(L, false);
}

static int
uci_lua_get_all(lua_State *L)
{
	return uci_lua_get_any(L, true);
}

static int
uci_lua_add(lua_State *L)
{
	struct uci_context *ctx;
	struct uci_section *s = NULL;
	struct uci_package *p;
	const char *package;
	const char *type;
	const char *name = NULL;
	int offset = 0;

	ctx = find_context(L, &offset);
	package = luaL_checkstring(L, 1 + offset);
	type = luaL_checkstring(L, 2 + offset);
	p = find_package(L, ctx, package, true);
	if (!p)
		goto fail;

	if (uci_add_section(ctx, p, type, &s) || !s)
		goto fail;

	name = s->e.name;
	lua_pushstring(L, name);
	return 1;

fail:
	lua_pushnil(L);
	return uci_push_status(L, ctx, true);
}

static int
uci_lua_delete(lua_State *L)
{
	struct uci_context *ctx;
	struct uci_ptr ptr;
	int offset = 0;
	char *s = NULL;

	ctx = find_context(L, &offset);

	if (lookup_args(L, ctx, offset, &ptr, &s))
		goto error;

	uci_delete(ctx, &ptr);

error:
	if (s)
		free(s);
	return uci_push_status(L, ctx, false);
}

static int
uci_lua_rename(lua_State *L)
{
	struct uci_context *ctx;
	struct uci_ptr ptr;
	int err = UCI_ERR_MEM;
	char *s = NULL;
	int nargs, offset = 0;

	ctx = find_context(L, &offset);
	nargs = lua_gettop(L);
	if (lookup_args(L, ctx, offset, &ptr, &s))
		goto error;

	switch(nargs - offset) {
	case 1:
		/* Format: uci.set("p.s.o=v") or uci.set("p.s=v") */
		break;
	case 4:
		/* Format: uci.set("p", "s", "o", "v") */
		ptr.value = luaL_checkstring(L, nargs);
		break;
	case 3:
		/* Format: uci.set("p", "s", "v") */
		ptr.value = ptr.option;
		ptr.option = NULL;
		break;
	default:
		err = UCI_ERR_INVAL;
		goto error;
	}

	err = uci_lookup_ptr(ctx, &ptr, NULL, true);
	if (err)
		goto error;

	if (((ptr.s == NULL) && (ptr.option != NULL)) || (ptr.value == NULL)) {
		err = UCI_ERR_INVAL;
		goto error;
	}

	err = uci_rename(ctx, &ptr);
	if (err)
		goto error;

error:
	return uci_push_status(L, ctx, false);
}

static int
uci_lua_reorder(lua_State *L)
{
	struct uci_context *ctx;
	struct uci_ptr ptr;
	int err = UCI_ERR_MEM;
	char *s = NULL;
	int nargs, offset = 0;

	ctx = find_context(L, &offset);
	nargs = lua_gettop(L);
	if (lookup_args(L, ctx, offset, &ptr, &s))
		goto error;

	switch(nargs - offset) {
	case 1:
		/* Format: uci.set("p.s=v") or uci.set("p.s=v") */
		if (ptr.option) {
			err = UCI_ERR_INVAL;
			goto error;
		}
		break;
	case 3:
		/* Format: uci.set("p", "s", "v") */
		ptr.value = ptr.option;
		ptr.option = NULL;
		break;
	default:
		err = UCI_ERR_INVAL;
		goto error;
	}

	err = uci_lookup_ptr(ctx, &ptr, NULL, true);
	if (err)
		goto error;

	if ((ptr.s == NULL) || (ptr.value == NULL)) {
		err = UCI_ERR_INVAL;
		goto error;
	}

	err = uci_reorder_section(ctx, ptr.s, strtoul(ptr.value, NULL, 10));
	if (err)
		goto error;

error:
	return uci_push_status(L, ctx, false);
}


static int
uci_lua_set(lua_State *L)
{
	struct uci_context *ctx;
	struct uci_ptr ptr;
	bool istable = false;
	int err = UCI_ERR_MEM;
	char *s = NULL;
	int i, nargs, offset = 0;

	ctx = find_context(L, &offset);
	nargs = lua_gettop(L);
	if (lookup_args(L, ctx, offset, &ptr, &s))
		goto error;

	switch(nargs - offset) {
	case 1:
		/* Format: uci.set("p.s.o=v") or uci.set("p.s=v") */
		break;
	case 4:
		/* Format: uci.set("p", "s", "o", "v") */
		if (lua_istable(L, nargs)) {
			if (lua_objlen(L, nargs) < 1)
				return luaL_error(L, "Cannot set an uci option to an empty table value");
			lua_rawgeti(L, nargs, 1);
			ptr.value = luaL_checkstring(L, -1);
			lua_pop(L, 1);
			istable = true;
		} else {
			ptr.value = luaL_checkstring(L, nargs);
		}
		break;
	case 3:
		/* Format: uci.set("p", "s", "v") */
		ptr.value = ptr.option;
		ptr.option = NULL;
		break;
	default:
		err = UCI_ERR_INVAL;
		goto error;
	}

	err = uci_lookup_ptr(ctx, &ptr, NULL, true);
	if (err)
		goto error;

	if (((ptr.s == NULL) && (ptr.option != NULL)) || (ptr.value == NULL)) {
		err = UCI_ERR_INVAL;
		goto error;
	}

	if (istable) {
		if (lua_objlen(L, nargs) == 1) {
			i = 1;
			if (ptr.o)
				err = uci_delete(ctx, &ptr);
		} else {
			i = 2;
			err = uci_set(ctx, &ptr);
			if (err)
				goto error;
		}

		for (; i <= lua_objlen(L, nargs); i++) {
			lua_rawgeti(L, nargs, i);
			ptr.value = luaL_checkstring(L, -1);
			err = uci_add_list(ctx, &ptr);
			lua_pop(L, 1);
			if (err)
				goto error;
		}
	} else {
		err = uci_set(ctx, &ptr);
		if (err)
			goto error;
	}


error:
	return uci_push_status(L, ctx, false);
}

enum pkg_cmd {
	CMD_SAVE,
	CMD_COMMIT,
	CMD_REVERT
};

static int
uci_lua_package_cmd(lua_State *L, enum pkg_cmd cmd)
{
	struct uci_context *ctx;
	struct uci_element *e, *tmp;
	struct uci_ptr ptr;
	char *s = NULL;
	int nargs, offset = 0;

	ctx = find_context(L, &offset);
	nargs = lua_gettop(L);
	if ((cmd != CMD_REVERT) && (nargs - offset > 1))
		goto err;

	if (lookup_args(L, ctx, offset, &ptr, &s))
		goto err;

	uci_lookup_ptr(ctx, &ptr, NULL, true);

	uci_foreach_element_safe(&ctx->root, tmp, e) {
		struct uci_package *p = uci_to_package(e);

		if (ptr.p && (ptr.p != p))
			continue;

		ptr.p = p;
		switch(cmd) {
		case CMD_COMMIT:
			uci_commit(ctx, &p, false);
			break;
		case CMD_SAVE:
			uci_save(ctx, p);
			break;
		case CMD_REVERT:
			uci_revert(ctx, &ptr);
			break;
		}
	}

err:
	return uci_push_status(L, ctx, false);
}

static int
uci_lua_save(lua_State *L)
{
	return uci_lua_package_cmd(L, CMD_SAVE);
}

static int
uci_lua_commit(lua_State *L)
{
	return uci_lua_package_cmd(L, CMD_COMMIT);
}

static int
uci_lua_revert(lua_State *L)
{
	return uci_lua_package_cmd(L, CMD_REVERT);
}

static void
uci_lua_add_change(lua_State *L, struct uci_element *e)
{
	struct uci_delta *h;
	const char *name;
	const char *value;

	h = uci_to_delta(e);
	if (!h->section)
		return;

	lua_getfield(L, -1, h->section);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		lua_newtable(L);
		lua_pushvalue(L, -1); /* copy for setfield */
		lua_setfield(L, -3, h->section);
	}

	name = h->e.name;
	value = h->value ? h->value : "";

	if (name) {
		lua_getfield(L, -1, name);

		/* this delta is a list add operation */
		if (h->cmd == UCI_CMD_LIST_ADD) {
			/* there seems to be no table yet */
			if (!lua_istable(L, -1)) {
				lua_newtable(L);

				/* if there is a value on the stack already, add */
				if (!lua_isnil(L, -2)) {
					lua_pushvalue(L, -2);
					lua_rawseti(L, -2, 1);
					lua_pushstring(L, value);
					lua_rawseti(L, -2, 2);

				/* this is the first table item */
				} else {
					lua_pushstring(L, value);
					lua_rawseti(L, -2, 1);
				}

				lua_setfield(L, -3, name);

			/* a table is on the top of the stack and this is a subsequent,
			 * list_add, append this value to table */
			} else {
				lua_pushstring(L, value);
				lua_rawseti(L, -2, lua_objlen(L, -2) + 1);
			}

		/* non-list change, simply set/replace field */
		} else {
			lua_pushstring(L, value);
			lua_setfield(L, -3, name);
		}

		lua_pop(L, 1);
	} else {
		lua_pushstring(L, value);
		lua_setfield(L, -2, ".type");
	}

	lua_pop(L, 1);
}

static void
uci_lua_changes_pkg(lua_State *L, struct uci_context *ctx, const char *package)
{
	struct uci_package *p = NULL;
	struct uci_element *e;
	bool autoload = false;

	p = find_package(L, ctx, package, false);
	if (!p) {
		autoload = true;
		p = find_package(L, ctx, package, true);
		if (!p)
			return;
	}

	if (uci_list_empty(&p->delta) && uci_list_empty(&p->saved_delta))
		goto done;

	lua_newtable(L);
	uci_foreach_element(&p->saved_delta, e) {
		uci_lua_add_change(L, e);
	}
	uci_foreach_element(&p->delta, e) {
		uci_lua_add_change(L, e);
	}
	lua_setfield(L, -2, p->e.name);

done:
	if (autoload)
		uci_unload(ctx, p);
}

static int
uci_lua_changes(lua_State *L)
{
	struct uci_context *ctx;
	const char *package = NULL;
	char **config = NULL;
	int nargs;
	int i, offset = 0;

	ctx = find_context(L, &offset);
	nargs = lua_gettop(L);
	switch(nargs - offset) {
	case 1:
		package = luaL_checkstring(L, 1 + offset);
	case 0:
		break;
	default:
		return luaL_error(L, "invalid argument count");
	}

	lua_newtable(L);
	if (package) {
		uci_lua_changes_pkg(L, ctx, package);
	} else {
		if (uci_list_configs(ctx, &config) != 0)
			goto done;

		for(i = 0; config[i] != NULL; i++) {
			uci_lua_changes_pkg(L, ctx, config[i]);
		}
	}

done:
	return 1;
}

static int
uci_lua_get_confdir(lua_State *L)
{
	struct uci_context *ctx = find_context(L, NULL);
	lua_pushstring(L, ctx->confdir);
	return 1;
}

static int
uci_lua_set_confdir(lua_State *L)
{
	struct uci_context *ctx;
	int offset = 0;

	ctx = find_context(L, &offset);
	luaL_checkstring(L, 1 + offset);
	uci_set_confdir(ctx, lua_tostring(L, -1));
	return uci_push_status(L, ctx, false);
}

static int
uci_lua_get_savedir(lua_State *L)
{
	struct uci_context *ctx = find_context(L, NULL);
	lua_pushstring(L, ctx->savedir);
	return 1;
}

static int
uci_lua_add_delta(lua_State *L)
{
	struct uci_context *ctx;
	int offset = 0;

	ctx = find_context(L, &offset);
	luaL_checkstring(L, 1 + offset);
	uci_add_delta_path(ctx, lua_tostring(L, -1));
	return uci_push_status(L, ctx, false);
}

static int
uci_lua_set_savedir(lua_State *L)
{
	struct uci_context *ctx;
	int offset = 0;

	ctx = find_context(L, &offset);
	luaL_checkstring(L, 1 + offset);
	uci_set_savedir(ctx, lua_tostring(L, -1));
	return uci_push_status(L, ctx, false);
}

static int
uci_lua_gc(lua_State *L)
{
	struct uci_context *ctx = find_context(L, NULL);
	uci_free_context(ctx);
	return 0;
}

static int
uci_lua_cursor(lua_State *L)
{
	struct uci_context **u;
	int argc = lua_gettop(L);

	u = lua_newuserdata(L, sizeof(struct uci_context *));
	luaL_getmetatable(L, METANAME);
	lua_setmetatable(L, -2);

	*u = uci_alloc_context();
	if (!*u)
		return luaL_error(L, "Cannot allocate UCI context");
	switch (argc) {
		case 2:
			if (lua_isstring(L, 2) &&
				(uci_set_savedir(*u, luaL_checkstring(L, 2)) != UCI_OK))
				return luaL_error(L, "Unable to set savedir");
			/* fall through */
		case 1:
			if (lua_isstring(L, 1) &&
				(uci_set_confdir(*u, luaL_checkstring(L, 1)) != UCI_OK))
				return luaL_error(L, "Unable to set savedir");
			break;
		default:
			break;
	}
	return 1;
}

// 判断模块名和section有无子节点
// 若无，复制节点时需要将nodeRecursiveCopy level=3，这样可以保证格式<a></a>，否则xml表示空节点为<a/>
// 1-有子节点，0-无子节点
static int hasChild (xmlNodePtr testNode)
{
	xmlNodePtr node = testNode->xmlChildrenNode;
	while (NULL != node)
	{
		if ( xmlStrcmp(node->name, BAD_CAST "text") ) // #define BAD_CAST (xmlChar *)
		{
			return 1;
		}
		node = node->next;
	}
	return 0;
}

static void nodeRecursiveCopy (xmlNodePtr addNode, xmlNodePtr newNode, int level)
{
	xmlNodePtr newChild = NULL;
	xmlNodePtr addChild = NULL;
	xmlNodePtr tmp   = NULL;
	xmlChar *secName = NULL;
	xmlChar *content = NULL;

	if ( OPT_COPY == level ) // option
	{
		if ( !xmlStrcmp(addNode->name, BAD_CAST "list") )
		{
			level = 2; // 像复制option一样复制list各个元素
			newChild = addNode->xmlChildrenNode; // 先释放原来list的元素，再整个复制
			while ( NULL != newChild )
			{
				tmp = newChild->next;
				xmlUnlinkNode(newChild);
				xmlFreeNode(newChild);
				newChild = tmp;
			}
		}
		else
		{
			content = xmlNodeGetContent(newNode);
			xmlNodeSetContent(addNode, (const xmlChar *)(content) );
			xmlFree(content); // 需要释放内存
			return;
		}
	}
	else if ( SEC_COPY == level ) // section
	{
		if ( xmlHasProp(newNode, BAD_CAST "name") )
		{
			secName = xmlGetProp(newNode, BAD_CAST "name");
			xmlNewProp(addNode, BAD_CAST "name", secName);
			xmlFree(secName); // 需要释放内存
		}
		if ( 0 == hasChild(newNode) ) // 空section，会莫名添加text子节点，需要特殊处理
		{
			xmlAddChild(addNode, xmlNewText(BAD_CAST"\r\n") );
			return;
		}
	}
	else if ( PKG_COPY == level )
	{
		if ( 0 == hasChild(newNode) )
		{
			xmlAddChild(addNode, xmlNewText(BAD_CAST"\r\n") );
			return;
		}
	}

	newChild = newNode->xmlChildrenNode;
	while ( NULL != newChild )
	{
		if ( !xmlStrcmp(newChild->name, BAD_CAST "text") )
		{
			tmp = newChild->next;
			xmlUnlinkNode(newChild);
			xmlFreeNode(newChild);
			newChild = tmp;
			continue;
		}
		addChild = xmlNewNode(NULL, newChild->name);
		xmlAddChild(addNode, addChild);
		nodeRecursiveCopy(addChild, newChild, level + 1);
		newChild = newChild->next;
	}
}

static int execRegex (char *str, char *pattern)
{
	int err = 0;
	regex_t reg;
	regmatch_t pmatch[1];

	memset(&reg, 0, sizeof(regex_t));
	memset(pmatch, 0, sizeof(regmatch_t));
	
	// 指针使用前判空
	if ( NULL == str || NULL == pattern )
	{
		return -1;
	}

	if ( regcomp(&reg, pattern, REG_EXTENDED) < 0 )
	{
		return -1;
	}
	err = regexec(&reg, str, 1, pmatch, 0); // 匹配结果最大允许数为1
	if ( !err ) // matched
	{
		regfree(&reg);
		return 0;
	}
	regfree(&reg);
	return -1;
}

// 处理非法xml节点——1.以数字，下划线开头；2.标签名含特殊字符
static void fixIllegalName (char *line)
{
	char sn[BUF_LEN] = {0}; // xml node name
	char * val_s = NULL;
	char en[BUF_LEN] = {0}; // end of option
	char tp[BUF_LEN*2] = {0}; // temp buf
	char tp1[BUF_LEN*2]= {0};
	int  ps = 0; // 指针开始位置
	int  pe = 0; // 指针结束位置
	int  val_l = 0;
	char *pattern1 = "[^A-Za-z0-9._-]"; // 除字母，数组，.，_，-，外的非法字符
	char *pattern2 = "<.*>.*</.*>";     // 匹配option
	char *pattern3 = "</.*>";		    // 匹配end node，pkg或section，list
	char *pattern4 = "<.*>";		    // 匹配start node，pkg或section，list
	char *tmp = NULL;
	int   flag     = -1;
	
	if ( NULL == line )
	{
		return;
	}

	// 1.判断节点类型：option, start, end
	if ( !execRegex(line, pattern2) ) // option
	{
		while ( line[ps] != '<' ) // 找到标签开始位置，为了处理特殊情况：'<'前面有空格
		{
			ps++;
		}
		ps++;
		pe = ps;

		flag = OPTION;
		while ( line[pe] )
		{
			if ( line[pe] == '>' ) // 得到name
			{
				strncpy(sn, line + ps, pe - ps);
				pe++;
				ps = pe;
				continue;
			}
			
			if ( line[pe] == '<' && line[pe + 1] == '/' ) // 得到value
			{
				val_s = line + ps;
				val_l = pe - ps;
				break;
			}
			pe++;
		}
		strncpy(en, sn, BUF_LEN);
	}
	else if ( !execRegex(line, pattern3) ) // end node, </a>
	{
		while ( !(line[ps] == '<' && line[ps + 1] == '/') )
		{
			ps++;
		}
		ps = ps + 2;
		pe = ps;
		
		flag = END;
		while ( line[pe] )
		{
			if ( line[pe] == '>' )
			{
				strncpy(sn, line + ps, pe - ps);
				break;
			}
			pe++;
		}
	}
	else if ( !execRegex(line, pattern4) ) // start node, <a>
	{
		while ( line[ps] != '<' )
		{
			ps++;
		}
		ps++;
		pe = ps;
		
		flag = START;
		while ( line[pe] )
		{
			if ( line[pe] == '>' )
			{
				strncpy(sn, line + ps, pe - ps);
				break;
			}
			pe++;
		}
	}
	
	// 2.判断name是否有非法字符
	if ( START == flag )
	{
		//if ( (tmp = strstr(sn, " name=")) || (tmp = strstr(sn, " modify=")) || (tmp = strstr(sn, " merge=")) )
		if ( (tmp = strstr(sn, " ")) ) // 直接定位空格符，这样多个属性时就不受顺序的影响了
		{
			strncpy(tp, sn, tmp - sn);
			if ( !execRegex(tp, pattern1) )
			{
				delBlockFlag = 1;
				memset(blockName, 0, BUF_LEN);
				strncpy(blockName, tp, BUF_LEN);
				memset(line, 0, BUF_LEN*6);
				return;
			}
		}
		if ( tp[0] == '\0' && !execRegex(sn, pattern1) )
		{
			delBlockFlag = 1;
			memset(blockName, 0, BUF_LEN);
			strncpy(blockName, sn, BUF_LEN);
			memset(line, 0, BUF_LEN*6);
			return;
		}
	}
	else if ( OPTION == flag )
	{
		//if ( (tmp = strstr(sn, " modify=")) || (tmp = strstr(sn, " merge=")) )
		if ( (tmp = strstr(sn, " ")) )
		{
			strncpy(tp, sn, tmp - sn);
			if ( !execRegex(tp, pattern1) )
			{
				memset(line, 0, BUF_LEN*6);
				return;
			}
			memset(en, 0, BUF_LEN);
			strncpy(en, tp, BUF_LEN);
		}
		if ( tp[0] == '\0' && !execRegex(sn, pattern1) )
		{
			memset(line, 0, BUF_LEN*6);
			return;
		}
	}
	else
	{
		if ( !execRegex(sn, pattern1) ) // 有非法字符，删除该行配置
		{
			memset(line, 0, BUF_LEN*6);
			return;
		}
	}
	
	// 3.特殊处理以数字，下划线_，.，-开头的name，在头部拼接JOINT_HEAD_STR
	if ( (sn[0] >= '0' && sn[0] <= '9') || (sn[0] == '_') || (sn[0] == '.') || (sn[0] == '-') )
	{
		memset(tp, 0, BUF_LEN*2);
		sprintf(tp, "%s%s", JOINT_HEAD_STR, sn);

		if ( OPTION == flag )
		{
			ps = strlen(tp);
			// copy option_val '%s'; val_s and line point to one memory			
			if (val_s)
			{
				memmove(line + ps + 2, val_s, val_l);
				line[ps + 2 + val_l] = '\0';
			}
			// copy option_name '<JOINT_HEAD_STR_name>'
			line[0] = '<';
			memcpy(line + 1, tp, ps);
			line[ps + 1] = '>';
			// copy option_end '</name>\n'
			sprintf(tp1, "%s%s", JOINT_HEAD_STR, en);
			strcat(line, "</");
			strcat(line, tp1);
			strcat(line, ">\n");
		}
		else if ( START == flag )
		{
			sprintf(line, "<%s>\n", tp);
		}
		else if ( END == flag )
		{
			sprintf(line, "</%s>\n", tp);
		}
	}
}

static void parseIllegalName (char *line)
{
	char sub1[BUF_LEN] = {0};
	char sub2[BUF_LEN] = {0};
	char tmp[BUF_LEN]  = {0};
	char *substr = NULL;
	int  len1 	 = 0;
	int  len2 	 = 0;
	
	if ( NULL == line )
	{
		return;
	}

	sprintf(tmp, "%s%s", "<", JOINT_HEAD_STR);
	substr = strstr(line, tmp);
	if ( substr )
	{
		len1 = strlen(tmp);
		len2 = substr - line;
		strncpy(sub1, line, len2);
		strncpy(sub2, line + len1 + len2, BUF_LEN - (len1 + len2) );
		memset(line, 0, BUF_LEN*6);
		sprintf(line, "%s%s%s", sub1, "<", sub2);
	}

	memset(tmp, 0, BUF_LEN);
	sprintf(tmp, "%s%s", "</", JOINT_HEAD_STR);
	substr = strstr(line, tmp);
	if ( substr )
	{
		len1 = strlen(tmp);
		len2 = substr - line;
		memset(sub1, 0, BUF_LEN);
		strncpy(sub1, line, len2);
		memset(sub2, 0, BUF_LEN);
		strncpy(sub2, line + len1 + len2, BUF_LEN - (len1 + len2) );
		memset(line, 0, BUF_LEN*6);
		sprintf(line, "%s%s%s", sub1, "</", sub2);
	}
}

// 给配置文件添加根节点，因为defaultconfig.xml不是标准xml，<config>和<US>平级没有根节点
static int addXMLRootNode (lua_State *L, const char *wrFile, const char *rdFile)
{	
	FILE *rd = NULL;
	FILE *wr = NULL;
	char buf[BUF_LEN*6]  = {0};
	char tmp[BUF_LEN*2]  = {0};
	int  ps   = 0;
	int  pe   = 0;
	int depth = 0;

	char *pattern1 = "<.*>.*</.*>"; // OPTION
	char *pattern2 = "</.*>"; 		// END
	char *pattern3 = "<.*>";  		// START
	
	char *pattern4 = "<.*/>";  		// null node, <domain/>

	if ( NULL == (rd = fopen(rdFile, "r")) )
	{
		luaL_error(L, "cannot open %s: %s", rdFile, strerror(errno));
		return -1;
	}
	if ( NULL == (wr = fopen(wrFile, "w")) )
	{
		luaL_error(L, "cannot open %s: %s", wrFile, strerror(errno));
		fclose(rd);
		return -1;
	}
	
	if ( NULL != fgets(buf, BUF_LEN*6, rd) && !strncmp(buf, "<?xml ", 6) )
	{
		fputs(buf, wr);
	}

	// 插入根节点开始
	memset(buf, 0, BUF_LEN*6);
	strcpy(buf, "<rootNode>\n");
	fputs(buf, wr);

	// 读一行，写一行
	while ( !feof(rd) )
	{
		memset(buf, 0, BUF_LEN*6);
		if ( NULL != fgets(buf, BUF_LEN*6, rd) )
		{
			if ( delBlockFlag )
			{
				if ( !execRegex(buf, pattern1) )      // OPTION
				{
					continue;
				}
				else if ( !execRegex(buf, pattern2) ) // END
				{
					memset(tmp, 0, BUF_LEN*2);
					sprintf(tmp, "%s%s%s", "</", blockName, ">");
					if ( strstr(buf, tmp) && depth-- == 0 )
					{
						delBlockFlag = 0;
						depth = 0;
					}
				}
				else if ( !execRegex(buf, pattern3) ) // START
				{
					ps = 0;
					pe = 0;
					while ( buf[ps] != '<' ) // 找到标签开始位置，为了处理特殊情况：'<'前面有空格
					{
						ps++;
					}
					ps++;
					pe = ps;
					memset(tmp, 0, BUF_LEN*2);
					while ( buf[pe] )
					{
						if ( buf[pe] == '>' || isblank(buf[pe]) )
						{
							strncpy(tmp, buf + ps, pe - ps);
							break;
						}
						pe++;
					}

					if ( !strcmp(blockName, tmp) ) // same illegal xml node
					{
						depth++;
					}
				}
			}
			else
			{
				if ( !execRegex(buf, pattern4) )
				{
					continue;
				}
				fixIllegalName(buf);
				if ( !execRegex(buf, pattern1) || !execRegex(buf, pattern2) || !execRegex(buf, pattern3) ) // 非空行才插入
				{
					fputs(buf, wr);
				}
			}
		}
	}

	// 插入根节点结束
	memset(buf, 0, BUF_LEN*6);
	strcpy(buf, "</rootNode>");
	fputs(buf, wr);

	fclose(rd);
	fclose(wr);
	return 0;
}

// 删除添加的根节点
static int deleteXMLRootNode (lua_State *L, const char *wrFile, char *rdFile)
{
	FILE *rd = NULL;
	FILE *wr = NULL;
	char buf[BUF_LEN*6] = {0};

	if ( NULL == (rd = fopen(rdFile, "r")) )
	{
		luaL_error(L, "cannot open %s: %s", rdFile, strerror(errno));
		return -1;
	}
	if ( NULL == (wr = fopen(wrFile, "w")) )
	{
		luaL_error(L, "cannot open %s: %s", wrFile, strerror(errno));
		fclose(rd);
		return -1;
	}

	while ( !feof(rd) )
	{
		memset(buf, 0, BUF_LEN*6);
		if ( NULL != fgets(buf, BUF_LEN*6, rd) )
		{
			if ( !strncmp(buf, "<rootNode>", 10) || !strncmp(buf, "</rootNode>", 11) )
			{
				continue;
			}
			else
			{
				parseIllegalName(buf);
				fputs(buf, wr);
			}
		}
	}

	fclose(rd);
	fclose(wr);
	return 0;
}

static xmlXPathObjectPtr get_nodeset(xmlDocPtr doc, const xmlChar *xpath) 
{ 
     xmlXPathContextPtr context;
     xmlXPathObjectPtr result;
	 
     context = xmlXPathNewContext(doc);
     if (context == NULL) 
     {
         return NULL;
     }

     result = xmlXPathEvalExpression(xpath, context);
     xmlXPathFreeContext(context);
     if (result == NULL) 
     {
         return NULL; 
     }

     if (xmlXPathNodeSetIsEmpty(result->nodesetval)) 
     {
         xmlXPathFreeObject(result);
         return NULL;
     }
     return result;
}

// 本地化：doc对应defaultconfig，type为LOCAL_MERGE
// 配置合并：doc对应userconfig，type为MERGE
static int mergeConfig (lua_State *L, xmlNodePtr defaultRoot, xmlDocPtr doc, const char *country, int type, int merge_no_flag)
{
	int mergeFlag = 0;
	int listFlag  = 0;
	
	// 使用xpath，提升查找效率
	xmlXPathObjectPtr result = NULL;
	char *xpathb = (char *)malloc(256);
	char *xpaths = (char *)malloc(256);
	char *xpathl = (char *)malloc(256);
	xmlNodeSetPtr nodeset = NULL;
	xmlNodePtr cur = NULL;
	xmlNodePtr tmp = NULL;
	int index = 0;
	
	// 新增配置
	xmlNodePtr add_rank0 = NULL;
	xmlNodePtr add_rank1 = NULL;
	xmlNodePtr add_rank2 = NULL;
	xmlNodePtr add_rank3 = NULL;
	
	// 基准配置，如LOCAL_MERGE时defaultconf的<US>..</US>，MERGE时defaultconf的<config>..</config>
	xmlNodePtr df_rank0 = NULL;
	xmlNodePtr df_rank1 = NULL;
	xmlNodePtr df_rank2 = NULL;
	xmlNodePtr df_rank3 = NULL;
	xmlNodePtr df_rank4 = NULL; // 用于处理list

	// 等待合并配置，如LOCAL_MERGE时defaultconf的<config>..</config>，MERGE时userconf的<config>..</config>
	xmlNodePtr userRoot = NULL;
	xmlNodePtr us_rank0 = NULL;

	xmlChar *defSecName = NULL; // section name
	xmlChar *defContent = NULL; // defaultconf option value
	xmlChar *usrContent = NULL; // userconf option value
	
	userRoot = xmlDocGetRootElement(doc);
	us_rank0 = userRoot->xmlChildrenNode;
	df_rank0 = defaultRoot->xmlChildrenNode;

	if ( LOCAL_MERGE == type ) // 本地化
	{
		// 寻找国家码对应节点，假设为country="US"
		while ( NULL != df_rank0 && xmlStrcmp(df_rank0->name, BAD_CAST country) )
		{
			df_rank0 = df_rank0->next;
		}
		if ( NULL == df_rank0 )
		{
			return mergeFlag;
		}
		
		// 寻找<config>
		while ( NULL != us_rank0 && xmlStrcmp(us_rank0->name, BAD_CAST "config") )
		{
			us_rank0 = us_rank0->next;
		}
		if ( NULL == us_rank0 ) // 没找到<config>, 直接把整个<US>复制到<config>
		{
			add_rank0 = xmlNewNode(NULL, BAD_CAST "config");
			nodeRecursiveCopy(add_rank0, df_rank0, CON_COPY);
			xmlAddChild(defaultRoot, add_rank0);
			mergeFlag = 1;
			return mergeFlag;
		}
	}
	else // 配置合并
	{
		// 寻找defaultconf的<config>
		while (NULL != df_rank0 && xmlStrcmp(df_rank0->name, BAD_CAST "config"))
		{
			df_rank0 = df_rank0->next;
		}
		if (NULL == df_rank0)
		{
			return mergeFlag;
		}
		
		// 寻找userconf的<config>
		while (NULL != us_rank0 && xmlStrcmp(us_rank0->name, BAD_CAST "config"))
		{
			us_rank0 = us_rank0->next;
		}
		if (NULL == us_rank0) // 没有<config>节点, 直接拷贝default <config>
		{
			add_rank0 = xmlNewNode(NULL, df_rank0->name);
			nodeRecursiveCopy(add_rank0, df_rank0, CON_COPY);
			xmlAddChild(userRoot, add_rank0);
			mergeFlag = 1;
			return mergeFlag;
		}
	}

	/* 遍历default <config>下所有pkg */
	df_rank1 = df_rank0->xmlChildrenNode;
	while ( NULL != df_rank1 )
	{
		// 莫名添加的text节点，需要过滤掉
		if ( !xmlStrcmp(df_rank1->name, BAD_CAST "text") )
		{
			tmp = df_rank1->next;
			xmlUnlinkNode(df_rank1);
			xmlFreeNode(df_rank1);
			df_rank1 = tmp;
			continue;
		}
		
		memset(xpaths, 0, 256);
		memset(xpathl, 0, 256);
		sprintf(xpaths, "/rootNode/config");
		sprintf(xpathl, "%s/%s", xpaths, df_rank1->name);
		result = get_nodeset(doc, BAD_CAST xpathl);
		if ( NULL == result ) // pkg不存在，直接添加
		{
			add_rank1 = xmlNewNode(NULL, df_rank1->name);
			nodeRecursiveCopy(add_rank1, df_rank1, PKG_COPY);

			// 查找上一级节点，将新增节点添加为其子节点
			result = get_nodeset(doc, BAD_CAST xpaths);
			if ( NULL != result )
			{
				nodeset = result->nodesetval;
				cur = nodeset->nodeTab[0];
				xmlAddChild(cur, add_rank1);
				mergeFlag = 1;
			}
		}
		else
		{
			nodeset = result->nodesetval;
			cur = nodeset->nodeTab[0];
			if ( !hasChild(cur) ) // pkg存在，但无子节点，为了保证xml格式正确，需要先删除userconfig中的此pkg节点再将新的添加进来
			{
				// 先删除userconfig中的内容为空的pkg节点
				xmlUnlinkNode(cur);
				xmlFreeNode(cur);
				
				add_rank1 = xmlNewNode(NULL, df_rank1->name);
				nodeRecursiveCopy(add_rank1, df_rank1, PKG_COPY);
				
				result = get_nodeset(doc, BAD_CAST xpaths);
				if ( NULL != result )
				{
					nodeset = result->nodesetval;
					cur = nodeset->nodeTab[0];
					xmlAddChild(cur, add_rank1);
					mergeFlag = 1;
				}
				df_rank1 = df_rank1->next;
				continue;
			}
			else
			{
				df_rank2 = df_rank1->xmlChildrenNode;
				memset(xpaths, 0, 256);
				strncpy(xpaths, xpathl, 256);
				while ( NULL != df_rank2 )
				{
					if ( !xmlStrcmp(df_rank2->name, BAD_CAST "text") )
					{
						tmp = df_rank2->next;
						xmlUnlinkNode(df_rank2);
						xmlFreeNode(df_rank2);
						df_rank2 = tmp;
						continue;
					}
					
					memset(xpathl, 0, 256);
					if ( xmlHasProp(df_rank2, BAD_CAST "name") )
					{
						defSecName = xmlGetProp(df_rank2, BAD_CAST "name");
						sprintf(xpathl, "%s/%s[@name=\"%s\"]", xpaths, df_rank2->name, defSecName);
					}
					else
					{
						sprintf(xpathl, "%s/%s", xpaths, df_rank2->name);
					}
			
					result = get_nodeset(doc, BAD_CAST xpathl);
					if ( NULL == result ) // section不存在，合并，处理配置丢失的情况
					{
						add_rank2 = xmlNewNode(NULL, df_rank2->name);
						nodeRecursiveCopy(add_rank2, df_rank2, SEC_COPY);
						result = get_nodeset(doc, BAD_CAST xpaths);
						if ( NULL != result )
						{
							nodeset = result->nodesetval;
							cur = nodeset->nodeTab[0];
							xmlAddChild(cur, add_rank2);
							mergeFlag = 1;
						}
					}
					else
					{
						if ( xmlHasProp(df_rank2, BAD_CAST "merge") && merge_no_flag == 1 ) // section存在且带属性merge="no"，直接跳过
						{
							df_rank2 = df_rank2->next;
							continue;
						}
						
						nodeset = result->nodesetval;
						cur = nodeset->nodeTab[0];
						if ( !hasChild(cur) ) // section存在，但无子节点，为了保证xml格式正确，需要先删除userconfig中的此section节点再将新的添加进来
						{
							// 先删除userconfig中的内容为空的section节点
							xmlUnlinkNode(cur);
							xmlFreeNode(cur);
							
							add_rank2 = xmlNewNode(NULL, df_rank2->name);
							nodeRecursiveCopy(add_rank2, df_rank2, SEC_COPY);
							result = get_nodeset(doc, BAD_CAST xpaths);
							if ( NULL != result )
							{
								nodeset = result->nodesetval;
								cur = nodeset->nodeTab[0];
								xmlAddChild(cur, add_rank2);
								mergeFlag = 1;
							}
							df_rank2 = df_rank2->next;
							continue;
						}
						else
						{
							df_rank3 = df_rank2->xmlChildrenNode;
							memset(xpathb, 0, 256);
							strncpy(xpathb, xpathl, 256); // .../pkg/sec
							while ( NULL != df_rank3 )
							{
								if ( !xmlStrcmp(df_rank3->name, BAD_CAST "text") )
								{
									tmp = df_rank3->next;
									xmlUnlinkNode(df_rank3);
									xmlFreeNode(df_rank3);
									df_rank3 = tmp;
									continue;
								}
								
								memset(xpathl, 0, 256);
								sprintf(xpathl, "%s/%s", xpathb, df_rank3->name);
								result = get_nodeset(doc, BAD_CAST xpathl);
								if ( NULL == result ) // option不存在
								{
									if ( xmlHasProp(df_rank3, BAD_CAST "merge") && merge_no_flag == 1 ) // option不存在且带属性merge="no"，直接跳过，不合并
									{
										df_rank3 = df_rank3->next;
										continue;
									}
									
									add_rank3 = xmlNewNode(NULL, df_rank3->name);
									nodeRecursiveCopy(add_rank3, df_rank3, OPT_COPY);
									result = get_nodeset(doc, BAD_CAST xpathb);
									if ( NULL != result )
									{
										nodeset = result->nodesetval;
										cur = nodeset->nodeTab[0];
										xmlAddChild(cur, add_rank3);
										mergeFlag = 1;
									}
								}
								else
								{
									nodeset = result->nodesetval;
									cur = nodeset->nodeTab[0]; // 获取查找结果集
										
									if ( !xmlStrcmp(df_rank3->name, BAD_CAST "list") )
									{
										df_rank4 = df_rank3->xmlChildrenNode;
										while ( NULL != df_rank4 ) // 获取defaultconf list的名称
										{
											if ( !xmlStrcmp(df_rank4->name, BAD_CAST "text") )
											{
												tmp = df_rank4->next;
												xmlUnlinkNode(df_rank4);
												xmlFreeNode(df_rank4);
												df_rank4 = tmp;
												continue;
											}
											else
											{
												break;
											}
										}
										
										if (NULL == df_rank4) //no child list node
										{
											listFlag = 0;
											df_rank3 = df_rank3->next;
											continue;
										}
										
										for ( index = 0; index < nodeset->nodeNr; index++ )
										{
											cur = nodeset->nodeTab[index];
											if ( !xmlStrcmp(cur->xmlChildrenNode->name, df_rank4->name) )
											{
												listFlag = 1; // 在userconf中找到了名称相同的list
												break;
											}
										}
										
										if (listFlag) // 存在相同的list
										{
											if ( LOCAL_MERGE == type ) // 本地化，完全替换
											{
												nodeRecursiveCopy(cur, df_rank3, OPT_COPY);
												mergeFlag = 1;
											}
											else // 配置合并
											{
												if ( xmlHasProp(df_rank3, BAD_CAST "modify") ) // 用户不可改配置
												{
													defContent = xmlNodeGetContent(df_rank3);
													usrContent = xmlNodeGetContent(cur);
													if ( xmlStrcmp(defContent, usrContent) ) // 更新不可改项
													{
														nodeRecursiveCopy(cur, df_rank3, OPT_COPY);
														mergeFlag = 1;
													}
													xmlFree(defContent);
													xmlFree(usrContent);
												}
											}
										}
										else // 没找到相同的list，直接添加
										{
											add_rank3 = xmlNewNode(NULL, df_rank3->name);
											nodeRecursiveCopy(add_rank3, df_rank3, OPT_COPY);
											result  = get_nodeset(doc, BAD_CAST xpathb);
											nodeset = result->nodesetval;
											cur = nodeset->nodeTab[0];
											xmlAddChild(cur, add_rank3);
											mergeFlag = 1;
										}
									}
									else
									{
										if ( LOCAL_MERGE == type ) // 本地化，不用判断option属性，直接更新
										{
											nodeRecursiveCopy(cur, df_rank3, OPT_COPY);
											mergeFlag = 1;
										}
										else
										{
											// 找到相同的option，且为不可改项，取default value覆盖user value
											if ( xmlHasProp(df_rank3, BAD_CAST "modify") )
											{
												defContent = xmlNodeGetContent(df_rank3);
												usrContent = xmlNodeGetContent(cur);
												if ( xmlStrcmp(defContent, usrContent) ) // 更新不可改项
												{
													nodeRecursiveCopy(cur, df_rank3, OPT_COPY);
													mergeFlag = 1;
												}
												xmlFree(defContent);
												xmlFree(usrContent);
											}
										}
									}
								}
								listFlag = 0;
								df_rank3 = df_rank3->next;
							}
						}
					}
					if ( NULL != defSecName )
					{
						xmlFree(defSecName);
						defSecName = NULL;
					}
					df_rank2 = df_rank2->next;
				}
			}
		}
		df_rank1 = df_rank1->next;
	}
	
	free(xpaths);
	free(xpathl);
	free(xpathb);
	return mergeFlag;
}

static int mergeConfig_step2 (lua_State *L, xmlNodePtr userRoot, xmlDocPtr dftDoc)
{
	int deleteFlag = 0;
	int anonymous = 0;
	
	/* 使用xpath，提升查找效率 */
	xmlXPathObjectPtr result = NULL;
	char *xpathb = (char *)malloc(256);
	char *xpaths = (char *)malloc(256);
	char *xpathl = (char *)malloc(256);
	char *xpath_rep = (char *)malloc(256);
	char *xpath_mod = (char *)malloc(256);
	xmlNodeSetPtr nodeset = NULL;
	xmlNodePtr cur = NULL;
	xmlNodePtr tmp = NULL;

	/* user-config 各层次节点 */
	xmlNodePtr us_rank0 = NULL;
	xmlNodePtr us_rank1 = NULL;
	xmlNodePtr us_rank2 = NULL;
	xmlNodePtr us_rank3 = NULL;

	xmlNodePtr defaultRoot = NULL;
	xmlNodePtr df_rank0 = NULL;

	xmlChar *usrSecName = NULL; /* user-config section name */
	xmlChar *usrContent = NULL; /* user-config option value */

	us_rank0 = userRoot->xmlChildrenNode;
	defaultRoot = xmlDocGetRootElement(dftDoc);
	df_rank0 = defaultRoot->xmlChildrenNode;

	// 寻找dftcfg的<config>
	while (NULL != df_rank0 && xmlStrcmp(df_rank0->name, BAD_CAST "config"))
	{
		df_rank0 = df_rank0->next;
	}
	if (NULL == df_rank0)
	{
		return deleteFlag;
	}
	
	// 寻找userconf的<config>
	while (NULL != us_rank0 && xmlStrcmp(us_rank0->name, BAD_CAST "config"))
	{
		us_rank0 = us_rank0->next;
	}
	if (NULL == us_rank0)
	{
		return deleteFlag;
	}

	/* 遍历user <config>下所有pkg */
	us_rank1 = us_rank0->xmlChildrenNode;
	while ( NULL != us_rank1 )
	{
		// 莫名添加的text节点，跳过
		if ( !xmlStrcmp(us_rank1->name, BAD_CAST "text") )
		{
			us_rank1 = us_rank1->next;
			continue;
		}

		memset(xpaths, 0, 256);
		memset(xpathl, 0, 256);
		sprintf(xpaths, "/rootNode/config");
		sprintf(xpathl, "%s/%s", xpaths, us_rank1->name); // 寻找 .../pkg

		result = get_nodeset(dftDoc, BAD_CAST xpathl);
		if ( NULL == result ) // usrcfg的pkg在dftcfg不存在
		{
			//usrcfg比dftcfg多出来的cfg,认为其是非法的，删除
			DPRINTF("delete illegal user-config package: %s", us_rank1->name);

			tmp = us_rank1->next;
			xmlUnlinkNode(us_rank1);
			xmlFreeNode(us_rank1);
			us_rank1 = tmp;
			deleteFlag = 1;
			continue;
		}
		else // usrcfg的pkg在dftcfg存在
		{
			nodeset = result->nodesetval;
			cur = nodeset->nodeTab[0];
			if ( hasChild(cur) ) // 对应的dftcfg的pkg有子节点
			{
				us_rank2 = us_rank1->xmlChildrenNode;
				memset(xpaths, 0, 256);
				strncpy(xpaths, xpathl, 256);
				while ( NULL != us_rank2 )
				{
					// 莫名添加的text节点，跳过
					if ( !xmlStrcmp(us_rank2->name, BAD_CAST "text") )
					{
						us_rank2 = us_rank2->next;
						continue;
					}

					/* usrcfg 里面的 replicate section需要删掉 */
					if ( !xmlStrcmp(us_rank2->name, BAD_CAST "replicate"))
					{
						DPRINTF("delete: %s.%s", us_rank1->name,us_rank2->name);
						tmp = us_rank2->next;
						xmlUnlinkNode(us_rank2);
						xmlFreeNode(us_rank2);
						us_rank2 = tmp;
						deleteFlag = 1;
						continue;
					}
					
					memset(xpathl, 0, 256);
					anonymous = 0;
					// 寻找.../pkg/sec
					if ( xmlHasProp(us_rank2, BAD_CAST "name") ) // 具名sec
					{
						usrSecName = xmlGetProp(us_rank2, BAD_CAST "name");
						sprintf(xpathl, "%s/%s[@name=\"%s\"]", xpaths, us_rank2->name, usrSecName); 
					}
					else // 匿名sec
					{
						sprintf(xpathl, "%s/%s", xpaths, us_rank2->name);
						anonymous = 1;
					}
					result = get_nodeset(dftDoc, BAD_CAST xpathl);

					/*
					 * 如果usrcfg的匿名section在dftcfg找到了相应的匹配，但是匹配出来的dftcfg的sec却是具名的
					 * 那么认为匹配错了，匹配的目的主要是为了找出usrcfg相比dftcfg多出来的cfg，因此
					 * 原则必须是sec类型一致且名称一致则认为匹配上了，否则认为是usrcfg多出来的sec
					 */
					if (result)
					{
						nodeset = result->nodesetval;
						cur = nodeset->nodeTab[0];
						if ( xmlHasProp(cur, BAD_CAST "name") && anonymous == 1)
						{
							result = NULL;
						}
					}

					if ( NULL == result ) // usrcfg 的section类型在dftcfg不存在
					{
						memset(xpath_rep, 0, 256);
						sprintf(xpath_rep, "%s/replicate/%s", xpaths, us_rank2->name);
						result = get_nodeset(dftDoc, BAD_CAST xpath_rep);
						if ( NULL != result ) // section定义了“replicate”
						{
							nodeset = result->nodesetval;
							cur = nodeset->nodeTab[0];
							usrContent = xmlNodeGetContent(cur);
							if ( !xmlStrcmp(usrContent, BAD_CAST "no") ) // 如果 replicate=no，则usrcfg多出来的section为非法
							{
								DPRINTF("delete replicate:no userconfig section: %s.%s[@name=%s]", us_rank1->name,us_rank2->name,
								(anonymous = 0) ? usrSecName : BAD_CAST "anonymous");

								tmp = us_rank2->next;
								xmlUnlinkNode(us_rank2);
								xmlFreeNode(us_rank2);
								us_rank2 = tmp;
								deleteFlag = 1;
								continue;
							}
						}
					}
					else // usrcfg 的section在dftcfg存在
					{
						nodeset = result->nodesetval;
						cur = nodeset->nodeTab[0];

						if ( hasChild(cur) ) // 有子节点
						{
							us_rank3 = us_rank2->xmlChildrenNode;
							memset(xpathb, 0, 256);
							strncpy(xpathb, xpathl, 256); // .../pkg/sec
							while ( NULL != us_rank3 )
							{
								if ( !xmlStrcmp(us_rank3->name, BAD_CAST "text") )
								{
									us_rank3 = us_rank3->next;
									continue;
								}

								memset(xpath_mod, 0, 256);
								sprintf(xpath_mod, "%s/modify", xpathb); // .../pkg/sec/modify
								
								memset(xpathl, 0, 256);
								sprintf(xpathl, "%s/%s", xpathb, us_rank3->name); // .../pkg/sec/option
								result = get_nodeset(dftDoc, BAD_CAST xpathl);
								if ( NULL == result ) // option在dftcfg不存在,新增的option需要做modify属性判断
								{
									result = get_nodeset(dftDoc, BAD_CAST xpath_mod);
									if ( NULL != result ) // dftcfg中 section 定义了option modify属性
									{
										nodeset = result->nodesetval;
										cur = nodeset->nodeTab[0];
										usrContent = xmlNodeGetContent(cur);
										if ( !xmlStrcmp(usrContent, BAD_CAST "no") ) // option modify=no，则usrcfg里面的option为非法
										{
											DPRINTF("delete option modify=no userconfig option: %s.%s.%s ", us_rank1->name,us_rank2->name,us_rank3->name);

											tmp = us_rank3->next;
											xmlUnlinkNode(us_rank3);
											xmlFreeNode(us_rank3);
											us_rank3 = tmp;
											deleteFlag = 1;
											continue;
										}
									}
								}
								us_rank3 = us_rank3->next; // 下一个option
							}
						}
					}
					if ( NULL != usrSecName )
					{
						xmlFree(usrSecName);
						usrSecName = NULL;
					}
					us_rank2 = us_rank2->next; // 下一个section
				}
			}
		}
		us_rank1 = us_rank1->next; // 下一个pkg
	}
	
	free(xpaths);
	free(xpathl);
	free(xpathb);
	free(xpath_rep);
	free(xpath_mod);
	return deleteFlag;
}


#ifdef ENABLE_REMODEL

static int replace_name(char* content, const char *remodel_name, xmlChar *flg)
{
    char *tmp = content;
    char *ptr = NULL;
    char str[REMODEL_PARA_LEN] = {0};

    strcpy(content, remodel_name);

    if (0 == xmlStrcmp(flg, REMODEL_NAME_01)){
		while(*tmp != 0) {
			if (!isalnum(*tmp)) {
				*tmp = ' ';
			}
			tmp++;
		}
    }
    else if (0 == xmlStrcmp(flg, REMODEL_NAME_02)){
		while(*tmp != 0) {
			if (!isalnum(*tmp)) {
				*tmp = '_';
			}
			tmp++;
		}
    }
    else if (0 == xmlStrcmp(flg, REMODEL_NAME_03)){
		while(*tmp != 0) {
			if (!isalnum(*tmp)) {
				*tmp = '-';
			}
			tmp++;
		}
    }
    else if (0 == xmlStrcmp(flg, REMODEL_NAME_04)){
        ptr = strtok(content, " ");
        while(ptr)
        {
            strncat(str, ptr, strlen(ptr));
            ptr = strtok(NULL, " ");
        }
        strncpy(content, str, strlen(str));
        content[strlen(str)] = '\0';
    }
    else {
        return -1;
    }

    return 0;
}

static int replace_ver(char* content, const char *remodel_ver, xmlChar *flg)
{
    char *pos1 = NULL;
    char *pos2 = NULL;

    pos1 = strchr(remodel_ver, '.');
    if (pos1 != NULL) {
        pos2 = strchr(pos1+1, '.');
    }
    
    if (0 == xmlStrcmp(flg, REMODEL_VER_01)){
        if (pos1 == NULL) {
            strcpy(content, remodel_ver);
        } 
        else {
            strncpy(content, remodel_ver, pos1 - remodel_ver);
        }
    }
    else if (0 == xmlStrcmp(flg, REMODEL_VER_02)){
        if (pos1 == NULL) {
            strcpy(content, remodel_ver);
            strcpy(content + strlen(remodel_ver), ".0");
        }
        else if (pos2 == NULL) {
            strcpy(content, remodel_ver);
        }
        else {
            strncpy(content, remodel_ver, pos2 - remodel_ver);
        }
    }
    else if (0 == xmlStrcmp(flg, REMODEL_VER_03)){
        if (pos1 == NULL) {
            strcpy(content, remodel_ver);
            strcpy(content + strlen(remodel_ver), ".0.0");
        }
        else if (pos2 == NULL) {
            strcpy(content, remodel_ver);
            strcpy(content + strlen(remodel_ver), ".0");
        }
        else {
            strcpy(content, remodel_ver);
        }
    }
    else {
        return -1;
    }
    
    return 0;
}

static int check_node_content(xmlNodePtr pNode)
{
    if (pNode->children->type != XML_TEXT_NODE) {
        return 0;
    }
    return 1;
}

static int check_remodel_para(xmlNodePtr pNode, const char *remodel_name, const char *remodel_ver)
{
	xmlChar *name = NULL;
	xmlChar *ver = NULL;
    char content[REMODEL_PARA_LEN] = {0};
	
    memset(content, 0, REMODEL_PARA_LEN);
    
    if ( xmlHasProp(pNode, BAD_CAST "remodel_name") )
    {
        if ( !check_node_content(pNode)) {
            return 0;
        }
        
        name = xmlGetProp(pNode, BAD_CAST "remodel_name");
        if (0 == xmlStrcmp(name, REMODEL_NAME_01)
            || 0 == xmlStrcmp(name, REMODEL_NAME_02)
            || 0 == xmlStrcmp(name, REMODEL_NAME_03)
            || 0 == xmlStrcmp(name, REMODEL_NAME_04))
        {
            replace_name(content, remodel_name, name);
			xmlNodeSetContent(pNode, (const xmlChar *)(content));
        }
        return 1;
    }

    if ( xmlHasProp(pNode, BAD_CAST "remodel_ver") )
    {
        if (0 == check_node_content(pNode)) {
            return 0;
        }
        
        ver = xmlGetProp(pNode, BAD_CAST "remodel_ver");
        if (0 == xmlStrcmp(ver, REMODEL_VER_01)
            || 0 == xmlStrcmp(ver, REMODEL_VER_02)
            || 0 == xmlStrcmp(ver, REMODEL_VER_03)) 
        {
            replace_ver(content, remodel_ver, ver);
			xmlNodeSetContent(pNode, (const xmlChar *)(content));
        }
        return 1;
    }

    return 0;
}

static int check_valid_para(const char *remodel_name, const char *remodel_ver, char *form_name, char *form_ver)
{
    int len = 0, ii = 0;
    int flg = 0;
    
    if (strlen(remodel_name) >= REMODEL_PARA_LEN || strlen(remodel_ver) >= REMODEL_PARA_LEN) {
        return 0;
    }

    /*
    flg = 0;
    len = strlen(remodel_name);
    for (ii = 0; ii < len; ii++) {
        if (!isalnum(*(remodel_name + ii))) {
            if (++flg > 1){
                break;
            }
        }
    }
    */
    strcpy(form_name, remodel_name);

    flg = 0;
    len = strlen(remodel_ver);
    for (ii = 0; ii < len; ii++) {
        if (!isdigit(*(remodel_ver + ii))) {
            if (*(remodel_ver + ii) != '.') {
                return 0;
            }
            flg++;
        }
    }
    if (flg > 2) {
        return 0;
    }
    strcpy(form_ver, remodel_ver);

    return 1;
}

static void remodel (xmlDocPtr doc, const char *remodel_name, const char *remodel_ver)
{
    xmlNodePtr userRoot = NULL;
	xmlNodePtr us_rank0 = NULL;
	xmlNodePtr us_rank1 = NULL;
	xmlNodePtr us_rank2 = NULL;
	xmlNodePtr us_rank3 = NULL;
	xmlNodePtr us_rank4 = NULL;
    int ret = 0;
    char form_name[REMODEL_PARA_LEN] = {0};
    char form_ver[REMODEL_PARA_LEN] = {0};

    if ( ! check_valid_para(remodel_name, remodel_ver, form_name, form_ver)){
        return;
    }
    
	userRoot = xmlDocGetRootElement(doc);
	us_rank0 = userRoot->xmlChildrenNode;

    while ( NULL != us_rank0 && xmlStrcmp(us_rank0->name, BAD_CAST "config") ) {
        us_rank0 = us_rank0->next;
    }
    if ( NULL == us_rank0 ){
        return;
    }

	us_rank1 = us_rank0->xmlChildrenNode;
    while (us_rank1 != NULL) {
        us_rank2 = us_rank1->xmlChildrenNode;
        while (us_rank2 != NULL) {
            ret = check_remodel_para(us_rank2, form_name, form_ver);
            if (ret == 0) {
                us_rank3 = us_rank2->xmlChildrenNode;
                while (us_rank3 != NULL) {
                    ret = check_remodel_para(us_rank3, form_name, form_ver);
                    if (ret == 0){
                        us_rank4 = us_rank3->xmlChildrenNode;
                        while (us_rank4 != NULL) {
                            check_remodel_para(us_rank4, form_name, form_ver);
                            us_rank4 = us_rank4->next;
                        }
                    }
                    us_rank3 = us_rank3->next;
                }
            }
            us_rank2 = us_rank2->next;
        }
        us_rank1 = us_rank1->next;
    }
    return;
}

/* 
 * fn		static int uci_lua_remodel(lua_State *L)
 * brief	remodel product
 * details	function to change product name and version of default config. 
 *			in luci, show uci:remodel(product_name,product_ver, defaultconfig.xml, country)
 *
 * param[in]	string of -1, country of current model, using in the feature
 * 				string of -2, path of defaultconfig.xml, the xml to parse
 *              string of -3, product version to change
 *              string of -4, product name to change
 * param[out]	the xml file of change product name and version
 *
 * return	state of remodel result
 * retval	-1 for failed, 0 for success
 *
 */
static int
uci_lua_remodel(lua_State *L)
{
	const char *user_config = NULL;
	const char *country     = NULL;
	int mergeFlag = 0;
	int ret = 0;

	xmlDocPtr  doc 		= NULL;
	xmlNodePtr userRoot = NULL;

    const char *remodel_name    = NULL;
    const char *remodel_ver    = NULL;
    
    remodel_name = luaL_checkstring(L, -4); 
    remodel_ver  = luaL_checkstring(L, -3); 

	user_config = luaL_checkstring(L, -2);
	country 	= luaL_checkstring(L, -1);

	// 参数判断
	if ( NULL == user_config || NULL == country )
	{
		luaL_error(L, "uci_lua_remodel : params error!");
		goto error;
	}

	if ( NULL == remodel_name || NULL == remodel_ver )
    {
        luaL_error(L, "uci_lua_remodel : params error!");
        goto error;
    }
	
	ret = addXMLRootNode(L, "/tmp/local_unmerged.xml", user_config);
	if (ret) // 添加根节点出错, 返回-1, 接口调用失败
	{
		luaL_error(L, "uci_lua_remodel : addXMLRootNode error!");
		goto error;
	}

	doc = xmlReadFile("/tmp/local_unmerged.xml", "UTF-8", XML_PARSE_NOBLANKS);
	if (NULL == doc)
	{
		unlink("/tmp/local_unmerged.xml");
		luaL_error(L, "uci_lua_remodel : xmlReadFile error!");
		goto error;
	}

	userRoot = xmlDocGetRootElement(doc);
	if (NULL == userRoot)
	{
		unlink("/tmp/local_unmerged.xml");
		xmlFreeDoc(doc);
		luaL_error(L, "uci_lua_remodel : xmlDocGetRootElement error!");
		goto error;
	}
	
    remodel(doc, remodel_name, remodel_ver);
	xmlSaveFormatFileEnc("/tmp/local_merged.xml", doc, "UTF-8", 1);

	unlink(user_config);
	ret = deleteXMLRootNode(L, user_config, "/tmp/local_merged.xml");
	if (ret) // 添加根节点出错, 返回-1, 接口调用失败
	{
		unlink("/tmp/local_merged.xml");
		unlink("/tmp/local_unmerged.xml");
		xmlFreeDoc(doc);
		luaL_error(L, "uci_lua_remodel : deleteXMLRootNode error!");
		goto error;
	}
	
	// 清理现场
	unlink("/tmp/local_merged.xml");
	unlink("/tmp/local_unmerged.xml");
	xmlFreeDoc(doc);
	lua_pushnumber(L, mergeFlag);
	return 1;

error :
	lua_pushnumber(L, ret);
	return -1;
}

#endif /* end of ENABLE_REMODEL */ 

static int
uci_lua_merge(lua_State *L)
{
	const char *user_config    = NULL;
	const char *default_config = NULL;
	const char *country  	   = NULL;
	int mergeFlag = 0; // 1-有配置更新到user
	int merge_no_flag = 1;//重启状态下merge=no配置生效
	int ret = 0;

	xmlDocPtr  defaultDoc  = NULL; // typedef xmlDoc *xmlDocPtr
	xmlDocPtr  userDoc     = NULL;
	xmlNodePtr defaultRoot = NULL; // typedef xmlNode *xmlNodePtr;
	xmlNodePtr userRoot = NULL;

	user_config    = luaL_checkstring(L, -3); // 倒数第三个元素
	default_config = luaL_checkstring(L, -2); // 倒数第二个元素
	country 	   = luaL_checkstring(L, -1); // 从栈顶开始，倒数第一个元素
	if ( NULL == user_config || NULL == default_config || NULL == country )
	{
		luaL_error(L, "uci_lua_merge : params error!");
		goto error;
	}

	ret = addXMLRootNode(L, "/tmp/dc.xml", default_config);
	if (ret) // 添加根节点出错, 返回-1, 接口调用失败
	{
		luaL_error(L, "uci_lua_merge : addXMLRootNode default_config error!");
		goto error;
	}
	ret = addXMLRootNode(L, "/tmp/uc.xml", user_config);
	if (ret) // 添加根节点出错, 返回-1, 接口调用失败
	{
		luaL_error(L, "uci_lua_merge : addXMLRootNode user_config error!");
		goto error;
	}

	defaultDoc = xmlReadFile("/tmp/dc.xml", "UTF-8", XML_PARSE_NOBLANKS);
	if ( NULL == defaultDoc )
	{
		unlink("/tmp/uc.xml");
		unlink("/tmp/dc.xml");
		luaL_error(L, "uci_lua_merge : xmlReadFile default_config error!");
		goto error;
	}
	userDoc = xmlReadFile("/tmp/uc.xml", "UTF-8", XML_PARSE_NOBLANKS);
	if ( NULL == userDoc )
	{
		unlink("/tmp/uc.xml");
		unlink("/tmp/dc.xml");
		xmlFreeDoc(defaultDoc);
		luaL_error(L, "uci_lua_merge : xmlReadFile user_config error!");
		goto error;
	}

	defaultRoot = xmlDocGetRootElement(defaultDoc);
	if ( NULL == defaultRoot )
	{
		unlink("/tmp/uc.xml");
		unlink("/tmp/dc.xml");
		xmlFreeDoc(defaultDoc);
		xmlFreeDoc(userDoc);
		luaL_error(L, "uci_lua_merge : xmlDocGetRootElement error!");
		goto error;
	}

	userRoot = xmlDocGetRootElement(userDoc);
	if ( NULL == userRoot )
	{
		unlink("/tmp/uc.xml");
		unlink("/tmp/dc.xml");
		xmlFreeDoc(defaultDoc);
		xmlFreeDoc(userDoc);
		luaL_error(L, "uci_lua_merge : xmlDocGetRootElement error!");
		goto error;
	}

	// (1)本地化，将<US>参数合到<config>中去，保存在defaultRoot
	mergeFlag = mergeConfig(L, defaultRoot, defaultDoc, country, LOCAL_MERGE, merge_no_flag);

	// (2)合并userconfig和本地化后的defaultconfig
	mergeFlag = mergeConfig(L, defaultRoot, userDoc, country, MERGE, merge_no_flag);
	mergeConfig_step2(L, userRoot, defaultDoc);
	
	// (3)最终配置保存到文件merged.xml
	xmlSaveFormatFileEnc("/tmp/merged.xml", userDoc, "UTF-8", 1);
	
	// 清理tmp.xml的根节点<rootNode>，保存到最终的reload-userconf.xml
	unlink(user_config);
	ret = deleteXMLRootNode(L, user_config, "/tmp/merged.xml");
	if (ret) // 添加根节点出错, 返回-1, 接口调用失败
	{
		unlink("/tmp/merged.xml");
		unlink("/tmp/uc.xml");
		unlink("/tmp/dc.xml");
		xmlFreeDoc(defaultDoc);
		xmlFreeDoc(userDoc);
		luaL_error(L, "uci_lua_merge : deleteXMLRootNode error!");
		goto error;
	}

	// 清理现场
	unlink("/tmp/merged.xml");
	unlink("/tmp/uc.xml");
	unlink("/tmp/dc.xml");
	xmlFreeDoc(defaultDoc);
	xmlFreeDoc(userDoc);
	lua_pushnumber(L, mergeFlag); // mergeFlag是uci:merge的返回值
	return 1;

error :
	lua_pushnumber(L, -1);
	return -1;
}

// -1接口调用失败，0成功，1有新配置合并
static int
uci_lua_reset_merge_local(lua_State *L)
{
	const char *user_config = NULL;
	const char *country     = NULL;
	int mergeFlag = 0;
	int merge_no_flag = 0;//出厂状态下忽略merge=no配置
	int ret = 0;

	xmlDocPtr  doc 		= NULL;
	xmlNodePtr userRoot = NULL;

	user_config = luaL_checkstring(L, -2);
	country 	= luaL_checkstring(L, -1);

	// 参数判断
	if ( NULL == user_config || NULL == country )
	{
		luaL_error(L, "uci_lua_reset_merge_local : params error!");
		goto error;
	}
	
	ret = addXMLRootNode(L, "/tmp/local_unmerged.xml", user_config);
	if (ret) // 添加根节点出错, 返回-1, 接口调用失败
	{
		luaL_error(L, "uci_lua_reset_merge_local : addXMLRootNode error!");
		goto error;
	}

	doc = xmlReadFile("/tmp/local_unmerged.xml", "UTF-8", XML_PARSE_NOBLANKS);
	if (NULL == doc)
	{
		unlink("/tmp/local_unmerged.xml");
		luaL_error(L, "uci_lua_reset_merge_local : xmlReadFile error!");
		goto error;
	}

	userRoot = xmlDocGetRootElement(doc);
	if (NULL == userRoot)
	{
		unlink("/tmp/local_unmerged.xml");
		xmlFreeDoc(doc);
		luaL_error(L, "uci_lua_reset_merge_local : xmlDocGetRootElement error!");
		goto error;
	}

	mergeFlag = mergeConfig(L, userRoot, doc, country, LOCAL_MERGE, merge_no_flag);

	xmlSaveFormatFileEnc("/tmp/local_merged.xml", doc, "UTF-8", 1);

	unlink(user_config);
	ret = deleteXMLRootNode(L, user_config, "/tmp/local_merged.xml");
	if (ret) // 添加根节点出错, 返回-1, 接口调用失败
	{
		unlink("/tmp/local_merged.xml");
		unlink("/tmp/local_unmerged.xml");
		xmlFreeDoc(doc);
		luaL_error(L, "uci_lua_reset_merge_local : deleteXMLRootNode error!");
		goto error;
	}
	
	// 清理现场
	unlink("/tmp/local_merged.xml");
	unlink("/tmp/local_unmerged.xml");
	xmlFreeDoc(doc);
	lua_pushnumber(L, mergeFlag);
	return 1;

error :
	lua_pushnumber(L, ret);
	return -1;
}

static const luaL_Reg uci[] = {
	{ "__gc", uci_lua_gc },
	{ "cursor", uci_lua_cursor },
	{ "load", uci_lua_load },
	{ "unload", uci_lua_unload },
	{ "get", uci_lua_get },
	{ "get_all", uci_lua_get_all },
	{ "add", uci_lua_add },
	{ "set", uci_lua_set },
	{ "rename", uci_lua_rename },
	{ "save", uci_lua_save },
	{ "delete", uci_lua_delete },
	{ "commit", uci_lua_commit },
	{ "revert", uci_lua_revert },
	{ "reorder", uci_lua_reorder },
	{ "changes", uci_lua_changes },
	{ "foreach", uci_lua_foreach },
	{ "add_history", uci_lua_add_delta },
	{ "add_delta", uci_lua_add_delta },
	{ "get_confdir", uci_lua_get_confdir },
	{ "set_confdir", uci_lua_set_confdir },
	{ "get_savedir", uci_lua_get_savedir },
	{ "set_savedir", uci_lua_set_savedir },
	{ "merge", uci_lua_merge},
	{ "reset_merge_local", uci_lua_reset_merge_local},
#ifdef ENABLE_REMODEL
	{ "remodel", uci_lua_remodel},
#endif 
	{ NULL, NULL },
};

int
luaopen_uci(lua_State *L)
{
	/* create metatable */
	luaL_newmetatable(L, METANAME);

	/* metatable.__index = metatable */
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	/* fill metatable */
	luaL_register(L, NULL, uci);
	lua_pop(L, 1);

	/* create module */
	luaL_register(L, MODNAME, uci);

	return 0;
}
