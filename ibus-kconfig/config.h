/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2010 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2011-2015 Ni Hui <shuizhuyuanluo@126.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __CONFIG_KCONFIG_H__
#define __CONFIG_KCONFIG_H__

#include <ibus.h>

#define IBUS_TYPE_CONFIG_KCONFIG    \
    (ibus_config_kconfig_get_type ())
#define IBUS_CONFIG_KCONFIG(obj)            \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_CONFIG_KCONFIG, IBusConfigKConfig))
#define IBUS_CONFIG_KCONFIG_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_CONFIG_KCONFIG, IBusConfigKConfigClass))
#define IBUS_IS_CONFIG_KCONFIG(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_CONFIG_KCONFIG))
#define IBUS_IS_CONFIG_KCONFIG_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_CONFIG_KCONFIG))
#define IBUS_CONFIG_KCONFIG_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_CONFIG_KCONFIG, IBusConfigKConfigClass))

typedef struct _IBusConfigKConfig IBusConfigKConfig;
typedef struct _IBusConfigKConfigClass IBusConfigKConfigClass;

GType              ibus_config_kconfig_get_type     (void);
#if !IBUS_CHECK_VERSION(1,3,99)
IBusConfigKConfig *ibus_config_kconfig_new          (IBusConnection     *connection);
#else
IBusConfigKConfig *ibus_config_kconfig_new          (GDBusConnection    *connection);
#endif

#endif
