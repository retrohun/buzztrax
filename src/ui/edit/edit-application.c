// $Id: edit-application.c,v 1.77 2006-08-13 14:41:34 ensonic Exp $
/**
 * SECTION:bteditapplication
 * @short_description: class for a gtk based buzztard editor application
 * @see_also: #BtMainWindow
 *
 * Opens the #BtMainWindow and provide application level function like load,
 * save, run and exit.
 */ 
 
#define BT_EDIT
#define BT_EDIT_APPLICATION_C

#include "bt-edit.h"
#include <libbtcore/application-private.h>

//-- signal ids

//-- property ids

enum {
  EDIT_APPLICATION_SONG=1,
  EDIT_APPLICATION_MAIN_WINDOW
};

// this needs to be here because of gtk-doc and unit-tests
GST_DEBUG_CATEGORY(GST_CAT_DEFAULT);

struct _BtEditApplicationPrivate {
  /* used to validate if dispose has run */
  gboolean dispose_has_run;
  
  /* the currently loaded song */
  BtSong *song;
  /* shared ui ressources */
  BtUIRessources *ui_ressources;
  /* the top-level window of our app */
  BtMainWindow *main_window;
};

static BtApplicationClass *parent_class=NULL;

//-- event handler

static void on_songio_status_changed(BtSongIO *songio,GParamSpec *arg,gpointer user_data) {
  BtEditApplication *self=BT_EDIT_APPLICATION(user_data);
  BtMainStatusbar *statusbar;
  gchar *str;

  g_assert(BT_IS_SONG_IO(songio));
  g_assert(user_data);

  g_object_get(self->priv->main_window,"statusbar",&statusbar,NULL);
  
  /* @todo push loader status changes into the statusbar
   * - how to handle to push and pop stuff, first_acces=push_only, last_access=pop_only
   *   - str!=NULL old.pop & new.push
   *   - str==NULL old.pop & default.push
   */
  g_object_get(songio,"status",&str,NULL);
  GST_INFO("songio_status has changed : \"%s\"",safe_string(str));
  g_object_set(statusbar,"status",str,NULL);
  g_object_try_unref(statusbar);
  g_free(str);
}

//-- helper methods

static gboolean bt_edit_application_run_ui(const BtEditApplication *self) {
  gboolean res;
  
  g_assert(self);
  g_assert(self->priv->main_window);
  
  GST_INFO("application.run_ui launched");
  
  res=bt_main_window_run(self->priv->main_window);

  GST_INFO("application.run_ui finished");
  return(res);
}

//-- constructor methods

/**
 * bt_edit_application_new:
 *
 * Create a new instance
 *
 * Returns: the new instance or %NULL in case of an error
 */
BtEditApplication *bt_edit_application_new(void) {
  BtEditApplication *self;

  if(!(self=BT_EDIT_APPLICATION(g_object_new(BT_TYPE_EDIT_APPLICATION,NULL)))) {
    goto Error;
  }
  if(!bt_application_new(BT_APPLICATION(self))) {
    goto Error;
  }
  // create or ref the shared ui ressources
  if(!(self->priv->ui_ressources=bt_ui_ressources_new())) {
    goto Error;
  }
  GST_INFO("new edit app created, app->ref_ct=%d",G_OBJECT(self)->ref_count);
  if(!(self->priv->main_window=bt_main_window_new(self))) {
    goto Error;
  }
  GST_INFO("new edit app window created, app->ref_ct=%d",G_OBJECT(self)->ref_count);
  return(self);
Error:
  GST_WARNING("new edit app failed");
  g_object_try_unref(self);
  return(NULL);
}

//-- methods

/**
 * bt_edit_application_new_song:
 * @self: the application instance to create a new song in
 *
 * Creates a new blank song instance. If there is a previous song instance it
 * will be freed.
 *
 * Returns: %TRUE for success
 */
gboolean bt_edit_application_new_song(const BtEditApplication *self) {
  gboolean res=FALSE;
  BtSong *song;
  
  g_return_val_if_fail(BT_IS_EDIT_APPLICATION(self),FALSE);

  // create new song
  if((song=bt_song_new(BT_APPLICATION(self)))) {
    BtSetup *setup;
    BtSequence *sequence;
    BtMachine *machine;
    gchar *id;

    g_object_get(song,"setup",&setup,"sequence",&sequence,NULL);
    // add some initial timelines
    g_object_set(sequence,"length",10,NULL);
    // add audiosink
    id=bt_setup_get_unique_machine_id(setup,"master");
    if((machine=BT_MACHINE(bt_sink_machine_new(song,id)))) {
      GHashTable *properties;

      GST_DEBUG("sink-machine-refs: %d",(G_OBJECT(machine))->ref_count);
      g_object_get(machine,"properties",&properties,NULL);
      if(properties) {
        gchar str[G_ASCII_DTOSTR_BUF_SIZE];
        g_hash_table_insert(properties,g_strdup("xpos"),g_strdup(g_ascii_dtostr(str,G_ASCII_DTOSTR_BUF_SIZE,0.0)));
        g_hash_table_insert(properties,g_strdup("ypos"),g_strdup(g_ascii_dtostr(str,G_ASCII_DTOSTR_BUF_SIZE,0.0)));
      }
      if(bt_machine_enable_input_level(machine) &&
        bt_machine_enable_input_gain(machine)
      ) {
        GST_DEBUG("sink-machine-refs: %d",(G_OBJECT(machine))->ref_count);
        // set new song
        g_object_set(G_OBJECT(self),"song",song,NULL);
        res=TRUE;
      }
      else {
        GST_WARNING("Can't add input level/gain element in sink machine");
      }
      GST_DEBUG("sink-machine-refs: %d",(G_OBJECT(machine))->ref_count);
      g_object_unref(machine);
    }
    else {
      GST_WARNING("Can't create sink machine");
    }
    g_free(id);

    // release references
    g_object_try_unref(setup);
    g_object_try_unref(sequence);
    g_object_unref(song);
  }
  return(res);
}

/**
 * bt_edit_application_load_song:
 * @self: the application instance to load a new song in
  *@file_name: the song filename to load
 *
 * Loads a new song. If there is a previous song instance it will be freed.
 *
 * Returns: true for success
 */
gboolean bt_edit_application_load_song(const BtEditApplication *self,const char *file_name) {
  gboolean res=FALSE;
  BtSongIO *loader;
  BtSong *song;

  g_return_val_if_fail(BT_IS_EDIT_APPLICATION(self),FALSE);

  GST_INFO("song name = %s",file_name);

  if((loader=bt_song_io_new(file_name))) {
    GdkCursor *cursor=gdk_cursor_new(GDK_WATCH);
    GdkWindow *window=GTK_WIDGET(self->priv->main_window)->window;
      
    gdk_window_set_cursor(window,cursor);
    gdk_cursor_unref(cursor);
    gtk_widget_set_sensitive(GTK_WIDGET(self->priv->main_window),FALSE);
      
    g_signal_connect(G_OBJECT(loader),"notify::status",G_CALLBACK(on_songio_status_changed),(gpointer)self);
    while(gtk_events_pending()) gtk_main_iteration();
    
    // create new song
    if((song=bt_song_new(BT_APPLICATION(self)))) {
      if(bt_song_io_load(loader,song)) {
        BtSetup *setup;
        BtWavetable *wavetable;
        BtMachine *machine;
      
        g_object_get(song,"setup",&setup,"wavetable",&wavetable,NULL);
        // get sink-machine
        if((machine=bt_setup_get_machine_by_type(setup,BT_TYPE_SINK_MACHINE))) {
          if(bt_machine_enable_input_level(machine) &&
            bt_machine_enable_input_gain(machine)
          ) {
            GList *missing_machines,*missing_waves;

            // DEBUG
            bt_song_write_to_dot_file(song);
            // DEBUG
            // set new song
            g_object_set(G_OBJECT(self),"song",song,NULL);
            res=TRUE;

            // get missing element info
            g_object_get(G_OBJECT(setup),"missing-machines",&missing_machines,NULL);
            g_object_get(G_OBJECT(wavetable),"missing-waves",&missing_waves,NULL);
            // tell about missing machines and/or missing waves
            if(missing_machines || missing_waves) {
              GtkWidget *label,*icon,*hbox,*vbox;
              gchar *str; 
              GtkWidget *dialog;
      
              dialog = gtk_dialog_new_with_buttons(_("Missing elements in song"),
                                                    GTK_WINDOW(self->priv->main_window),
                                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                                    NULL);
            
              hbox=gtk_hbox_new(FALSE,12);
              gtk_container_set_border_width(GTK_CONTAINER(hbox),6);
            
              icon=gtk_image_new_from_stock(GTK_STOCK_DIALOG_WARNING,GTK_ICON_SIZE_DIALOG);
              gtk_container_add(GTK_CONTAINER(hbox),icon);
              
              vbox=gtk_vbox_new(FALSE,6);
              label=gtk_label_new(NULL);
              str=g_strdup_printf("<big><b>%s</b></big>",_("Missing elements in song"));
              gtk_label_set_markup(GTK_LABEL(label),str);
              gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
              g_free(str);
              gtk_container_add(GTK_CONTAINER(vbox),label);
              if(missing_machines) {
                GList *node;
                GtkWidget *missing_list, *missing_list_view;
                gchar *missing_text,*ptr;
                gint length=0;
                
                label=gtk_label_new(_("The machines listed below are missing or failed to load."));
                gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
                gtk_container_add(GTK_CONTAINER(vbox),label);
                
                for(node=missing_machines;node;node=g_list_next(node)) {
                  length+=2+strlen((gchar *)(node->data));
                }
                ptr=missing_text=g_malloc(length);
                for(node=missing_machines;node;node=g_list_next(node)) {
                  length=g_sprintf(ptr,"%s\n",(gchar *)(node->data));
                  ptr=&ptr[length];
                }
                
                missing_list = gtk_text_view_new();
                gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(missing_list), FALSE);
                gtk_text_view_set_editable(GTK_TEXT_VIEW(missing_list), FALSE);
                gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(missing_list), GTK_WRAP_WORD);
                gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(missing_list)),missing_text,-1);
                gtk_widget_show(missing_list);
                g_free(missing_text);
              
                missing_list_view = gtk_scrolled_window_new(NULL, NULL);
                gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (missing_list_view), GTK_SHADOW_IN);
                gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (missing_list_view), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
                gtk_container_add(GTK_CONTAINER(missing_list_view), missing_list);
                gtk_widget_show(missing_list_view);
                gtk_container_add(GTK_CONTAINER(vbox),missing_list_view);
              }
              if(missing_waves) {
                GList *node;
                GtkWidget *missing_list, *missing_list_view;
                gchar *missing_text,*ptr;
                gint length=0;
                
                label=gtk_label_new(_("The waves listed below are missing or failed to load."));
                gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
                gtk_container_add(GTK_CONTAINER(vbox),label);
      
                for(node=missing_waves;node;node=g_list_next(node)) {
                  length+=2+strlen((gchar *)(node->data));
                }
                ptr=missing_text=g_malloc(length);
                for(node=missing_waves;node;node=g_list_next(node)) {
                  length=g_sprintf(ptr,"%s\n",(gchar *)(node->data));
                  ptr=&ptr[length];
                }
                
                missing_list = gtk_text_view_new();
                gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(missing_list), FALSE);
                gtk_text_view_set_editable(GTK_TEXT_VIEW(missing_list), FALSE);
                gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(missing_list), GTK_WRAP_WORD);
                gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(missing_list)),missing_text,-1);
                gtk_widget_show(missing_list);
                g_free(missing_text);
              
                missing_list_view = gtk_scrolled_window_new(NULL, NULL);
                gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (missing_list_view), GTK_SHADOW_IN);
                gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (missing_list_view), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
                gtk_container_add(GTK_CONTAINER(missing_list_view), missing_list);
                gtk_widget_show(missing_list_view);
                gtk_container_add(GTK_CONTAINER(vbox),missing_list_view);        
              }       
              gtk_container_add(GTK_CONTAINER(hbox),vbox);
              gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),hbox);
              gtk_widget_show_all(dialog);
                                                              
              gtk_dialog_run(GTK_DIALOG(dialog));
              gtk_widget_destroy(dialog);
            }      
          }
          else {
            GST_WARNING("Can't add input level/gain element in sink machine");
          }
          g_object_unref(machine);
        }
        else {
          GST_WARNING("Can't look up sink machine");
        }
        g_object_try_unref(setup);
            g_object_unref(wavetable);
      }
      else {
        GST_ERROR("could not load song \"%s\"",file_name);
      }
    }  
    gtk_widget_set_sensitive(GTK_WIDGET(self->priv->main_window),TRUE);
    gdk_window_set_cursor(window,NULL);
    g_object_unref(song);
    g_object_unref(loader);
  }
  return(res);
}

/**
 * bt_edit_application_save_song:
 * @self: the application instance to save a song from
  *@file_name: the song filename to save
 *
 * Saves a song.
 *
 * Returns: true for success
 */
gboolean bt_edit_application_save_song(const BtEditApplication *self,const char *file_name) {
  gboolean res=FALSE;
  BtSongIO *saver;

  g_return_val_if_fail(BT_IS_EDIT_APPLICATION(self),FALSE);

  GST_INFO("song name = %s",file_name);

  if((saver=bt_song_io_new(file_name))) {
    GdkCursor *cursor=gdk_cursor_new(GDK_WATCH);
    GdkWindow *window=GTK_WIDGET(self->priv->main_window)->window;
      
    gdk_window_set_cursor(window,cursor);
    gdk_cursor_unref(cursor);
    gtk_widget_set_sensitive(GTK_WIDGET(self->priv->main_window),FALSE);
      
    g_signal_connect(G_OBJECT(saver),"notify::status",G_CALLBACK(on_songio_status_changed),(gpointer)self);
    while(gtk_events_pending()) gtk_main_iteration();
    if(bt_song_io_save(saver,self->priv->song)) {
      res=TRUE;
    }
    else {
      GST_ERROR("could not save song \"%s\"",file_name);
    }
    GST_INFO("saving done");

    gtk_widget_set_sensitive(GTK_WIDGET(self->priv->main_window),TRUE);
    gdk_window_set_cursor(window,NULL);
    g_object_unref(saver);
  }
  return(res);
}


/**
 * bt_edit_application_run:
 * @self: the application instance to run
 *
 * start the gtk based editor application
 *
 * Returns: true for success
 */
gboolean bt_edit_application_run(const BtEditApplication *self) {
  gboolean res=FALSE;

  g_return_val_if_fail(BT_IS_EDIT_APPLICATION(self),FALSE);

  GST_INFO("application.run launched");

  if(bt_edit_application_new_song(self)) {
    res=bt_edit_application_run_ui(self);
  }
  GST_INFO("application.run finished");
  return(res);
}

/**
 * bt_edit_application_load_and_run:
 * @self: the application instance to run
 * @input_file_name: the file to load initially
 *
 * load the file of the supplied name and start the gtk based editor application
 *
 * Returns: true for success
 */
gboolean bt_edit_application_load_and_run(const BtEditApplication *self, const gchar *input_file_name) {
  gboolean res=FALSE;

  g_return_val_if_fail(BT_IS_EDIT_APPLICATION(self),FALSE);

  GST_INFO("application.load_and_run launched");

  if(bt_edit_application_load_song(self,input_file_name)) {
    res=bt_edit_application_run_ui(self);
  }
  else {
    GST_WARNING("loading song failed");
    // start normaly
    bt_edit_application_run(self);
    // @todo show error message
  }
  GST_INFO("application.load_and_run finished");
  return(res);
}

//-- wrapper

//-- default signal handler

//-- class internals

/* returns a property for the given property_id for this object */
static void bt_edit_application_get_property(GObject      *object,
                               guint         property_id,
                               GValue       *value,
                               GParamSpec   *pspec)
{
  BtEditApplication *self = BT_EDIT_APPLICATION(object);
  return_if_disposed();
  switch (property_id) {
    case EDIT_APPLICATION_SONG: {
      g_value_set_object(value, self->priv->song);
    } break;
    case EDIT_APPLICATION_MAIN_WINDOW: {
      g_value_set_object(value, self->priv->main_window);
    } break;
    default: {
       G_OBJECT_WARN_INVALID_PROPERTY_ID(object,property_id,pspec);
    } break;
  }
}

/* sets the given properties for this object */
static void bt_edit_application_set_property(GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  BtEditApplication *self = BT_EDIT_APPLICATION(object);
  return_if_disposed();
  switch (property_id) {
    case EDIT_APPLICATION_SONG: {
      //if(self->priv->song) GST_INFO("song->ref_ct=%d",G_OBJECT(self->priv->song)->ref_count);
      g_object_try_unref(self->priv->song);
      self->priv->song=BT_SONG(g_value_dup_object(value));
      GST_DEBUG("set the song for edit_application: %p",self->priv->song);
    } break;
    default: {
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object,property_id,pspec);
    } break;
  }
}

static void bt_edit_application_dispose(GObject *object) {
  BtEditApplication *self = BT_EDIT_APPLICATION(object);

  return_if_disposed();
  self->priv->dispose_has_run = TRUE;

  /* This should destory the window as this is a child of the app.
   * Problem 1: On the other hand, this *NEVER* gets called as long as the window keeps its
   * strong reference to the app.
   * Solution 1: Only use weak refs when reffing upstream objects
   */
  GST_DEBUG("!!!! self=%p, self->ref_ct=%d",self,G_OBJECT(self)->ref_count);

  if(self->priv->song) {
    GST_INFO("song->ref_ct=%d",G_OBJECT(self->priv->song)->ref_count);
    bt_song_stop(self->priv->song);
  }
  g_object_try_unref(self->priv->song);

  //if(self->priv->main_window)
    //GST_INFO("main_window->ref_ct=%d",G_OBJECT(self->priv->main_window)->ref_count);
  //g_object_try_unref(self->priv->main_window);
  
  g_object_try_unref(self->priv->ui_ressources);

  GST_DEBUG("  chaining up");
  G_OBJECT_CLASS(parent_class)->dispose(object);
  GST_DEBUG("  done");
}

static void bt_edit_application_finalize(GObject *object) {
  //BtEditApplication *self = BT_EDIT_APPLICATION(object);
  
  //GST_DEBUG("!!!! self=%p",self);

  G_OBJECT_CLASS(parent_class)->finalize(object);
  GST_DEBUG("  done");
}

static void bt_edit_application_init(GTypeInstance *instance, gpointer g_class) {
  BtEditApplication *self = BT_EDIT_APPLICATION(instance);
  
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self, BT_TYPE_EDIT_APPLICATION, BtEditApplicationPrivate);
}

static void bt_edit_application_class_init(BtEditApplicationClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

  parent_class=g_type_class_peek_parent(klass);
  g_type_class_add_private(klass,sizeof(BtEditApplicationPrivate));

  gobject_class->set_property = bt_edit_application_set_property;
  gobject_class->get_property = bt_edit_application_get_property;
  gobject_class->dispose      = bt_edit_application_dispose;
  gobject_class->finalize     = bt_edit_application_finalize;

  klass->song_changed = NULL;

  g_object_class_install_property(gobject_class,EDIT_APPLICATION_SONG,
                                  g_param_spec_object("song",
                                     "song construct prop",
                                     "the song object, the wire belongs to",
                                     BT_TYPE_SONG, /* object type */
                                     G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class,EDIT_APPLICATION_MAIN_WINDOW,
                                  g_param_spec_object("main-window",
                                     "main window prop",
                                     "the main window of this application",
                                     BT_TYPE_MAIN_WINDOW, /* object type */
                                     G_PARAM_READABLE));
}

GType bt_edit_application_get_type(void) {
  static GType type = 0;
  if (G_UNLIKELY(type == 0)) {
    static const GTypeInfo info = {
      G_STRUCT_SIZE(BtEditApplicationClass),
      NULL, // base_init
      NULL, // base_finalize
      (GClassInitFunc)bt_edit_application_class_init, // class_init
      NULL, // class_finalize
      NULL, // class_data
      G_STRUCT_SIZE(BtEditApplication),
      0,   // n_preallocs
      (GInstanceInitFunc)bt_edit_application_init, // instance_init
      NULL // value_table
    };
    type = g_type_register_static(BT_TYPE_APPLICATION,"BtEditApplication",&info,0);
  }
  return type;
}
