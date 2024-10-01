/*
    This file is part of darktable,
    Copyright (C) 2010-2024 darktable developers.

    darktable is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    darktable is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with darktable.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <json-glib/json-glib.h>

G_BEGIN_DECLS

void dt_json_add_int(JsonBuilder *json_builder, const gchar *key, const int32_t value);

void dt_json_add_bool(JsonBuilder *json_builder, const gchar *key, const gboolean value);

void dt_json_add_string(JsonBuilder *json_builder, const gchar *key, const gchar *value);

void dt_json_add_int_from_dt_conf(JsonBuilder *json_builder, const gchar *config_string);

void dt_json_add_bool_from_dt_conf(JsonBuilder *json_builder, const gchar *config_string);

void dt_json_add_string_from_dt_conf(JsonBuilder *json_builder, const gchar *config_string);

int32_t dt_json_get_int(JsonReader *json_reader, const gchar *key);

gboolean dt_json_get_bool(JsonReader *json_reader, const gchar *key);

const gchar *dt_json_get_string(JsonReader *json_reader, const gchar *key);

G_END_DECLS

// clang-format off
// modelines: These editor modelines have been set for all relevant files by tools/update_modelines.py
// vim: shiftwidth=2 expandtab tabstop=2 cindent
// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-spaces modified;
// clang-format on
