/*
    This file is part of darktable,
    Copyright (C) 2026 darktable developers.

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

#include "common/darktable.h"
#include "common/exif.h"
#include "gui/gtk.h"
#include <glib-2.0/glib-object.h>
#include <glib-2.0/glib.h>
#include "gui/metadata_tags.h"

typedef enum dt_metadata_tag_cols_t
{
  DT_METADATA_TAGS_COL_XMP = 0,
  DT_METADATA_TAGS_COL_TYPE,
  DT_METADATA_TAGS_COL_VISIBLE,
  DT_METADATA_TAGS_NUM_COLS
} dt_metadata_tag_cols_t;

static GtkListStore *liststore;
static GtkWidget *sel_entry;
static const gchar *sel_entry_text;
static GtkTreeView *sel_view;
static GList *taglist = NULL;
static GtkWidget *add_button;

static GHashTable *ipmd_tags_table = NULL;
static GHashTable *ipmd_top_table = NULL;
static GHashTable *ipmd_struct_table = NULL;

typedef struct dt_ipmd_data_t
{
  gchar *name;
  gchar *label;
  gchar *helptext;
  gchar *usernotes;
  gchar *ipmdschema;
  gchar *sortorder;
  gchar *datatype;
  gchar *dataformat;
  gchar *propoccurrence;
  gchar *isrequired;
  gchar *xmpid;
} dt_ipmd_data_t;

static void _free_ipmd_data(gpointer d)
{
  dt_ipmd_data_t *ipmd_data = (dt_ipmd_data_t *)d;

  g_free(ipmd_data->name); 
  g_free(ipmd_data->label); 
  g_free(ipmd_data->helptext); 
  g_free(ipmd_data->usernotes); 
  g_free(ipmd_data->ipmdschema); 
  g_free(ipmd_data->sortorder); 
  g_free(ipmd_data->datatype); 
  g_free(ipmd_data->dataformat); 
  g_free(ipmd_data->propoccurrence); 
  g_free(ipmd_data->isrequired); 
  g_free(ipmd_data->xmpid); 

  g_free(ipmd_data); 
}

static void _free_hashtable(gpointer d)
{
  GHashTable *hash_table = (GHashTable *)d;
  g_hash_table_destroy(hash_table);
}

static void _set_ipmd_data(JsonObject *obj, dt_ipmd_data_t *ipmd_data)
{
  const char *ipmd_field_names[] = {
    "name", "label", "helptext", "usernotes", "ipmdschema", "sortorder",
    "datatype", "dataformat", "propoccurrence", "isrequired", "XMPid"
  };

  gchar **ipmd_field_ptrs[] = {
    &ipmd_data->name, &ipmd_data->label, &ipmd_data->helptext,
    &ipmd_data->usernotes, &ipmd_data->ipmdschema, &ipmd_data->sortorder,
    &ipmd_data->datatype, &ipmd_data->dataformat, &ipmd_data->propoccurrence,
    &ipmd_data->isrequired, &ipmd_data->xmpid
  };

  for(int i = 0; i < G_N_ELEMENTS(ipmd_field_names); i++)
  {
    JsonNode *node = json_object_get_member(obj, ipmd_field_names[i]);
    if(node) *ipmd_field_ptrs[i] = json_node_dup_string(node);
  }   
}

static GHashTable *_get_ipmd_table(JsonObject *start_obj, const gchar* identifier)
{
  GHashTable *table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, _free_ipmd_data);

  JsonNode *top_node = json_object_get_member(start_obj, identifier);
  JsonObject *top_obj = json_node_get_object(top_node);
  GList *members = json_object_get_members(top_obj);

  for(GList *item = members; item; item = g_list_next(item))
  {
    gchar *node_identifier = g_strdup((gchar *)item->data);

    JsonNode *node = json_object_get_member(top_obj, node_identifier);
    JsonObject *obj = json_node_get_object(node);
    dt_ipmd_data_t *ipmd_data = g_malloc0(sizeof(dt_ipmd_data_t));
    _set_ipmd_data(obj, ipmd_data);
    g_hash_table_insert(table, node_identifier, ipmd_data); 
  }
  g_list_free(members);

  return table;
}

static gboolean _readIptcReferenceJson()
{
  // IPTC Photo Metadata Technical Reference Documentation
  // https://iptc.org/std/photometadata/documentation/techreference/

  char datadir[PATH_MAX] = {0};
  dt_loc_get_datadir(datadir, sizeof(datadir));
  gchar *full_path = g_build_filename(datadir, "iptc-pmd-techreference_2025.1.json", NULL);

  if(!g_file_test(full_path, G_FILE_TEST_EXISTS))
  {
    dt_print(DT_DEBUG_ALWAYS, "[metadata] iptc tech reference file not found: %s", full_path);
    g_free(full_path);
    return FALSE;
  }

  JsonParser *parser = json_parser_new();
  GError *error = NULL;

  if(!json_parser_load_from_file(parser, full_path, &error))
  {
    dt_print(DT_DEBUG_ALWAYS,
      "[metadata] failed to parse iptc tech reference file: %s",
      error ? error->message : "unknown error");
    if(error)
      g_error_free(error);
    g_object_unref(parser);
    g_free(full_path);
    return FALSE;
  }

  JsonNode *root = json_parser_get_root(parser);
  JsonObject *root_obj = json_node_get_object(root);

  // get list of all tagnames, no prefix, and map them to their ipmdid's
  ipmd_tags_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
  JsonNode *et_topnoprefix_node = json_object_get_member(root_obj, "et_topnoprefix");
  JsonObject *et_topnoprefix_obj = json_node_get_object(et_topnoprefix_node);
  GList *et_topnoprefix_members = json_object_get_members(et_topnoprefix_obj);

  for(GList *item = et_topnoprefix_members; item; item = g_list_next(item))
  {
    gchar *tagname = g_strdup((gchar *)item->data);

    JsonNode *tag_node = json_object_get_member(et_topnoprefix_obj, tagname);
    JsonObject *tag_obj = json_node_get_object(tag_node);
    JsonNode *ipmd_node = json_object_get_member(tag_obj, "ipmdid");
    gchar *ipmdid = json_node_dup_string(ipmd_node);
    g_hash_table_insert(ipmd_tags_table, tagname, ipmdid);
  }
  g_list_free(et_topnoprefix_members);

  // get the list of all impd_top elements
  ipmd_top_table = _get_ipmd_table(root_obj, "ipmd_top");

  // get the list of all ipmd_struct elements, each of them will be a GHashTable with its members
  ipmd_struct_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, _free_hashtable);
  JsonNode *ipmd_struct_node = json_object_get_member(root_obj, "ipmd_struct");
  JsonObject *ipmd_struct_obj = json_node_get_object(ipmd_struct_node);
  GList *ipmd_struct_members = json_object_get_members(ipmd_struct_obj);

  for(GList *item = ipmd_struct_members; item; item = g_list_next(item))
  {
    gchar *struct_name = g_strdup((gchar *)item->data);

    if(!g_strcmp0(struct_name, "AltLang"))
    {
      // AltLang is a special structure, skip it
      continue;
    }

    JsonNode *struct_node = json_object_get_member(ipmd_struct_obj, struct_name);
    JsonObject *struct_obj = json_node_get_object(struct_node);
    GList *struct_members = json_object_get_members(struct_obj);
    GHashTable *struct_member_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    for(GList *struct_item = struct_members; struct_item; struct_item = g_list_next(struct_item))
    {
      gchar *struct_item_name = g_strdup((gchar *)struct_item->data);

      JsonNode *struct_item_node = json_object_get_member(struct_obj, struct_item_name);
      JsonObject *struct_item_obj = json_node_get_object(struct_item_node);
      dt_ipmd_data_t *ipmd_data = g_malloc0(sizeof(dt_ipmd_data_t));
      _set_ipmd_data(struct_item_obj, ipmd_data);
      g_hash_table_insert(struct_member_table, struct_item_name, ipmd_data); 
    }
    g_list_free(struct_members);

    g_hash_table_insert(ipmd_struct_table, struct_name, struct_member_table);
  }
  g_list_free(ipmd_struct_members);

  g_object_unref(parser);
  g_free(full_path);



  GList *tagnames = g_hash_table_get_keys(ipmd_tags_table);
  for(GList *item = tagnames; item; item = g_list_next(item))
  {
    const gchar *tagname = (gchar *)item->data;
    const gchar *ipmdid = (gchar *) g_hash_table_lookup(ipmd_tags_table, tagname);

    printf("tagname: %s --> %s\n", tagname, ipmdid);
  }
  g_list_free(tagnames);



  GList *tags = g_hash_table_get_keys(ipmd_top_table);
  for(GList *item = tags; item; item = g_list_next(item))
  {
    printf("toptable tag: %s\n", (gchar *)item->data);
  }
  g_list_free(tags);



  GList *struct_tags = g_hash_table_get_keys(ipmd_struct_table);
  for(GList *item = struct_tags; item; item = g_list_next(item))
  {
    printf("hashtable struct tag: %s\n", (gchar *)item->data);
  }
  g_list_free(struct_tags);


  printf("--------------\n");
  const gchar *tagname = "CreatorContactInfo";
  const gchar *ipmdid = g_hash_table_lookup(ipmd_tags_table, tagname);
  printf("tagname: %s --> %s\n", tagname, ipmdid);
  const dt_ipmd_data_t *data = g_hash_table_lookup(ipmd_top_table, ipmdid);
  GHashTable *struct_table = g_hash_table_lookup(ipmd_struct_table, data->dataformat);

  GList *names = g_hash_table_get_keys(struct_table);
  for(GList *item = names; item; item = g_list_next(item))
  {
    const gchar *name = (gchar *)item->data;
    const dt_ipmd_data_t *struct_data = g_hash_table_lookup(struct_table, name);

    printf("%s %s\n", name, struct_data->name);
  }
  g_list_free(names);


  // g_hash_table_destroy(ipmd_tags_table);
  // g_hash_table_destroy(ipmd_top_table);
  // g_hash_table_destroy(ipmd_struct_table);

  return TRUE;
}



// routine to set individual visibility flag
static gboolean _set_matching_tag_visibility(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_idata)
{
  gboolean visible;
  gchar *tagname = NULL;
  gtk_tree_model_get(model, iter, DT_METADATA_TAGS_COL_XMP, &tagname, -1);
  if(!sel_entry_text[0])
    visible = TRUE;
  else
  {
    gchar *haystack = g_utf8_strdown(tagname, -1);
    gchar *needle = g_utf8_strdown(sel_entry_text, -1);
    visible = (g_strrstr(haystack, needle) != NULL);
    g_free(haystack);
    g_free(needle);
  }
  gtk_list_store_set(GTK_LIST_STORE(model), iter, DT_METADATA_TAGS_COL_VISIBLE, visible, -1);
  g_free(tagname);
  return FALSE;
}

// set the metadata tag visibility aligned with filter
static void _tag_name_changed(GtkEntry *entry, gpointer user_data)
{
  sel_entry_text = gtk_entry_get_text(GTK_ENTRY(sel_entry));
  GtkTreeModel *model = gtk_tree_view_get_model(sel_view);
  GtkTreeModel *store = gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(model));
  gtk_tree_model_foreach(store, (GtkTreeModelForeachFunc)_set_matching_tag_visibility, NULL);
}

gchar *dt_metadata_tags_get_selected()
{
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(sel_view);
  GtkTreeSelection *selection = gtk_tree_view_get_selection(sel_view);
  if(gtk_tree_selection_get_selected(selection, &model, &iter))
  {
    gchar *tagname;
    gtk_tree_model_get(model, &iter, DT_METADATA_TAGS_COL_XMP, &tagname, -1);
    return tagname;
  }
  return NULL;
}

static void _tree_selection_change(GtkTreeSelection *selection, gpointer user_data)
{
  const int nb = gtk_tree_selection_count_selected_rows(selection);
  gtk_widget_set_sensitive(add_button, nb > 0);
}

GtkWidget *dt_metadata_tags_dialog(GtkWidget *parent,
                                   gpointer metadata_activated_callback,
                                   gpointer user_data)
{
  GtkWidget *dialog = gtk_dialog_new_with_buttons(_("select tag"), GTK_WINDOW(parent),
                                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                                  _("_add"), GTK_RESPONSE_ACCEPT,
                                                  _("_done"), GTK_RESPONSE_NONE, NULL);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_NONE);
  gtk_window_set_default_size(GTK_WINDOW(dialog), DT_PIXEL_APPLY_DPI(500), DT_PIXEL_APPLY_DPI(300));
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);

  // keep a reference to the "add" button to toggle its sensitivity
  add_button = gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);

  sel_entry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(sel_entry), "");
  gtk_widget_set_tooltip_text(sel_entry, _("list filter"));
  gtk_entry_set_activates_default(GTK_ENTRY(sel_entry), TRUE);
  g_signal_connect(G_OBJECT(sel_entry), "changed", G_CALLBACK(_tag_name_changed), NULL);

  sel_view = GTK_TREE_VIEW(gtk_tree_view_new());
  GtkWidget *w = dt_gui_scroll_wrap(GTK_WIDGET(sel_view));
  gtk_widget_set_tooltip_text(GTK_WIDGET(sel_view), _("list of available tags. click 'add' button or double-click on tag to add the selected one"));
  GtkTreeSelection *selection = gtk_tree_view_get_selection(sel_view);
  gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
  g_signal_connect(selection, "changed", G_CALLBACK(_tree_selection_change), NULL);
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(_("tag"), renderer, "text", 0, NULL);
  gtk_tree_view_append_column(sel_view, col);
  renderer = gtk_cell_renderer_text_new();
  col = gtk_tree_view_column_new_with_attributes(_("type"), renderer, "text", 1, NULL);
  gtk_tree_view_append_column(sel_view, col);
  liststore = gtk_list_store_new(DT_METADATA_TAGS_NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);
  GtkTreeModel *model = gtk_tree_model_filter_new(GTK_TREE_MODEL(liststore), NULL);
  gtk_tree_model_filter_set_visible_column(GTK_TREE_MODEL_FILTER(model), DT_METADATA_TAGS_COL_VISIBLE);


  // read the IPTC technical reference
  if(!ipmd_tags_table)
    _readIptcReferenceJson();

  // populate the metadata tag list with exiv2 information
  if(!taglist)
    taglist = (GList *) dt_exif_get_exiv2_taglist();

  for(GList *tag = taglist; tag; tag = g_list_next(tag))
  {
    const char *tagname = tag->data;

    char *type = g_strstr_len(tagname, -1, ",");
    if(type)
    {
      type[0] = '\0';
      type++;
    }

    gtk_list_store_insert_with_values(liststore, NULL, -1,
                       DT_METADATA_TAGS_COL_XMP, tagname,
                       DT_METADATA_TAGS_COL_TYPE, type,
                       DT_METADATA_TAGS_COL_VISIBLE, TRUE,
                       -1);

    if(type)
    {
      type--;
      type[0] = ',';
    }
  }

  gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(liststore), DT_METADATA_TAGS_COL_XMP, GTK_SORT_ASCENDING);
  gtk_tree_view_set_model(sel_view, model);
  g_object_unref(model);
  g_signal_connect(G_OBJECT(sel_view), "row-activated", G_CALLBACK(metadata_activated_callback), user_data);

  dt_gui_dialog_add(GTK_DIALOG(dialog), sel_entry, w);
  return dialog;
}

// clang-format off
// modelines: These editor modelines have been set for all relevant files by tools/update_modelines.py
// vim: shiftwidth=2 expandtab tabstop=2 cindent
// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-spaces modified;
// clang-format on

