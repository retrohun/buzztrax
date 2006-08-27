/* $Id: bt-edit.h,v 1.36 2006-08-27 20:31:30 ensonic Exp $
 */

#ifndef BT_EDIT_H
#define BT_EDIT_H

#include <math.h>
#include <stdio.h>

//-- libbtcore
#include <libbtcore/core.h>
//-- gtk+
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
//-- libgnomecanvas
#include <libgnomecanvas/libgnomecanvas.h>
//-- libgnome
#ifdef USE_GNOME
#include <libgnome/gnome-init.h>
#include <libgnome/gnome-help.h>
#endif

#include "edit-application-methods.h"
#include "machine-canvas-item-methods.h"
#include "machine-menu-methods.h"
#include "machine-properties-dialog-methods.h"
#include "machine-preferences-dialog-methods.h"
#include "main-menu-methods.h"
#include "main-pages-methods.h"
#include "main-page-machines-methods.h"
#include "main-page-patterns-methods.h"
#include "main-page-sequence-methods.h"
#include "main-page-waves-methods.h"
#include "main-page-info-methods.h"
#include "main-statusbar-methods.h"
#include "main-toolbar-methods.h"
#include "main-window-methods.h"
#include "pattern-properties-dialog-methods.h"
#include "pattern-view-methods.h"
#include "sequence-view-methods.h"
#include "settings-dialog-methods.h"
#include "settings-page-audiodevices-methods.h"
#include "tools.h"
#include "ui-ressources-methods.h"
#include "volume-popup.h"
#include "wire-analysis-dialog-methods.h"
#include "wire-canvas-item-methods.h"

//-- misc
#ifndef GST_CAT_DEFAULT
  #define GST_CAT_DEFAULT bt_edit_debug
#endif
#if defined(BT_EDIT) && !defined(BT_EDIT_APPLICATION_C)
  GST_DEBUG_CATEGORY_EXTERN(GST_CAT_DEFAULT);
#endif

/**
 * GNOME_CANVAS_BROKEN_PROPERTIES:
 *
 * gnome canvas has a broken design,
 * it does not allow derived classes to have G_PARAM_CONSTRUCT_ONLY properties
 */
#define GNOME_CANVAS_BROKEN_PROPERTIES 1

#endif // BT_EDIT_H
