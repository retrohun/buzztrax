/* $Id$
 *
 * Buzztard
 * Copyright (C) 2011 Buzztard team <buzztard-devel@lists.sf.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef BT_MACHINE_LIST_MODEL_METHODS_H
#define BT_MACHINE_LIST_MODEL_METHODS_H

#include "machine-list-model.h"

extern BtMachineListModel *bt_machine_list_model_new(BtSetup *setup);
extern GObject *bt_machine_list_model_get_object(BtMachineListModel *model,GtkTreeIter *iter);

#endif // BT_MACHINE_LIST_MODEL_METHDOS_H