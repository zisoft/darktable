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

#include "common/json.h"
#include "control/conf.h"

static const gchar *_get_key_from_config_string(const gchar *config_string)
{
  const gchar *key = config_string + strlen(config_string);
  while(--key > config_string && *key != '/')
    ;
  return ++key;
}

void dt_json_add_int(JsonBuilder *json_builder, const gchar *key, const int32_t value)
{
  json_builder_set_member_name(json_builder, key);
  json_builder_add_int_value(json_builder, value);
}

void dt_json_add_bool(JsonBuilder *json_builder, const gchar *key, const gboolean value)
{
  json_builder_set_member_name(json_builder, key);
  json_builder_add_boolean_value(json_builder, value);
}

void dt_json_add_string(JsonBuilder *json_builder, const gchar *key, const gchar *value)
{
  json_builder_set_member_name(json_builder, key);
  json_builder_add_string_value(json_builder, value);
}

void dt_json_add_int_from_dt_conf(JsonBuilder *json_builder, const gchar *config_string)
{
  const gchar *key = _get_key_from_config_string(config_string);
  dt_json_add_int(json_builder, key, dt_conf_get_int(config_string));
}

void dt_json_add_bool_from_dt_conf(JsonBuilder *json_builder, const gchar *config_string)
{
  const gchar *key = _get_key_from_config_string(config_string);
  dt_json_add_bool(json_builder, key, dt_conf_get_bool(config_string));
}

void dt_json_add_string_from_dt_conf(JsonBuilder *json_builder, const gchar *config_string)
{
  const gchar *key = _get_key_from_config_string(config_string);
  dt_json_add_string(json_builder, key, dt_conf_get_string_const(config_string));
}

int32_t dt_json_get_int(JsonReader *json_reader, const gchar *key)
{
  json_reader_read_member(json_reader, key);
  const int32_t value = json_reader_get_int_value(json_reader);
  json_reader_end_element(json_reader);
  return value;
}

gboolean dt_json_get_bool(JsonReader *json_reader, const gchar *key)
{
  json_reader_read_member(json_reader, key);
  const gboolean value = json_reader_get_boolean_value(json_reader);
  json_reader_end_element(json_reader);
  return value;
}

const gchar *dt_json_get_string(JsonReader *json_reader, const gchar *key)
{
  json_reader_read_member(json_reader, key);
  const gchar *value = json_reader_get_string_value(json_reader);
  json_reader_end_element(json_reader);
  return value;
}
