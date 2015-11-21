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

#include <stdio.h>
#include <string.h>
#include <gio/gio.h>
#include <ibus.h>
#include <QString>
#include <QVariant>
#include <KConfig>
#include <KConfigGroup>
#include "config.h"

#if !IBUS_CHECK_VERSION(1,3,99)
#ifndef DBUS_ERROR_FAILED
#define DBUS_ERROR_FAILED "org.freedesktop.DBus.Error.Failed"
#endif /* DBUS_ERROR_FAILED */
#endif

#define KCONFIG_FILENAME "ibusrc"

struct _IBusConfigKConfig {
    IBusConfigService parent;
    KConfig          *kconfig;
};

struct _IBusConfigKConfigClass {
    IBusConfigServiceClass parent;

};

/* functions prototype */
static void         ibus_config_kconfig_class_init      (IBusConfigKConfigClass   *_class);
static void         ibus_config_kconfig_init            (IBusConfigKConfig        *config);
static void         ibus_config_kconfig_destroy         (IBusConfigKConfig        *config);
#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean     ibus_config_kconfig_set_value       (IBusConfigService        *config,
                                                         const gchar              *section,
                                                         const gchar              *name,
                                                         const GValue             *value,
                                                         IBusError               **error);
static gboolean     ibus_config_kconfig_get_value       (IBusConfigService        *config,
                                                         const gchar              *section,
                                                         const gchar              *name,
                                                         GValue                   *value,
                                                         IBusError               **error);
static gboolean     ibus_config_kconfig_unset           (IBusConfigService        *config,
                                                         const gchar              *section,
                                                         const gchar              *name,
                                                         IBusError               **error);
#else
static gboolean     ibus_config_kconfig_set_value       (IBusConfigService        *config,
                                                         const gchar              *section,
                                                         const gchar              *name,
                                                         GVariant                 *value,
                                                         GError                  **error);
static GVariant    *ibus_config_kconfig_get_value       (IBusConfigService        *config,
                                                         const gchar              *section,
                                                         const gchar              *name,
                                                         GError                  **error);
static GVariant    *ibus_config_kconfig_get_values      (IBusConfigService        *config,
                                                         const gchar              *section,
                                                         GError                  **error);
static gboolean     ibus_config_kconfig_unset_value     (IBusConfigService        *config,
                                                         const gchar              *section,
                                                         const gchar              *name,
                                                         GError                  **error);
#endif
#if !IBUS_CHECK_VERSION(1,3,99)
static QVariant     _to_qvariant                        (const GValue             *value);
static void         _from_qvariant                      (GValue                   *value,
                                                         const QVariant           &qv);
#else
static QVariant     _to_qvariant                        (GVariant                 *value);
static GVariant    *_from_qvariant                      (const QVariant           &qv);
#endif

#if !IBUS_CHECK_VERSION(1,3,99)
static IBusConfigServiceClass *parent_class = NULL;

GType
ibus_config_kconfig_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo __type_info = {
        sizeof (IBusConfigKConfigClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_config_kconfig_class_init,
        (GClassFinalizeFunc)NULL,
        NULL,
        sizeof (IBusConfigKConfig),
        0,
        (GInstanceInitFunc) ibus_config_kconfig_init,
        NULL
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_CONFIG_SERVICE,
                                       "IBusConfigKConfig",
                                       &__type_info,
                                       (GTypeFlags) 0);
    }

    return type;
}
#else
G_DEFINE_TYPE (IBusConfigKConfig, ibus_config_kconfig, IBUS_TYPE_CONFIG_SERVICE)
#endif

static void
ibus_config_kconfig_class_init (IBusConfigKConfigClass *_class)
{
    GObjectClass *object_class = G_OBJECT_CLASS (_class);

#if !IBUS_CHECK_VERSION(1,3,99)
    parent_class = (IBusConfigServiceClass *) g_type_class_peek_parent (_class);
#endif

    IBUS_OBJECT_CLASS (object_class)->destroy = (IBusObjectDestroyFunc) ibus_config_kconfig_destroy;
    IBUS_CONFIG_SERVICE_CLASS (object_class)->set_value   = ibus_config_kconfig_set_value;
    IBUS_CONFIG_SERVICE_CLASS (object_class)->get_value   = ibus_config_kconfig_get_value;
#if !IBUS_CHECK_VERSION(1,3,99)
    IBUS_CONFIG_SERVICE_CLASS (object_class)->unset       = ibus_config_kconfig_unset;
#else
    IBUS_CONFIG_SERVICE_CLASS (object_class)->get_values  = ibus_config_kconfig_get_values;
    IBUS_CONFIG_SERVICE_CLASS (object_class)->unset_value = ibus_config_kconfig_unset_value;
#endif
}

static void
ibus_config_kconfig_init (IBusConfigKConfig *config)
{
    config->kconfig = new KConfig (KCONFIG_FILENAME);
}

static void
ibus_config_kconfig_destroy (IBusConfigKConfig *config)
{
    config->kconfig->sync();
    delete config->kconfig;
    config->kconfig = NULL;

#if !IBUS_CHECK_VERSION(1,3,99)
    IBUS_OBJECT_CLASS (parent_class)->destroy ((IBusObject *)config);
#else
    IBUS_OBJECT_CLASS (ibus_config_kconfig_parent_class)->destroy ((IBusObject *)config);
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static QVariant
_to_qvariant (const GValue    *value)
{
    QVariant qv;
    GType type = G_VALUE_TYPE (value);

    /// FIXME dirty workaround to determine string type
    switch (type) {
    case G_TYPE_STRING:
        {
            qv = QVariant::fromValue ("S" + QString::fromUtf8 (g_value_get_string (value)));
            qv.convert (QVariant::String);
        }
        break;
    case G_TYPE_INT:
        {
            qv = QVariant::fromValue ("I" + QString::number (g_value_get_int (value)));
            qv.convert (QVariant::String);
        }
        break;
    case G_TYPE_UINT:
        {
            qv = QVariant::fromValue ("U" + QString::number (g_value_get_uint (value)));
            qv.convert (QVariant::String);
        }
        break;
    case G_TYPE_BOOLEAN:
        {
            qv = QVariant::fromValue ("B" + QString::number (g_value_get_boolean (value)));
            qv.convert (QVariant::String);
        }
        break;
    case G_TYPE_FLOAT:
        {
            qv = QVariant::fromValue ("F" + QString::number (g_value_get_float (value)));
            qv.convert (QVariant::String);
        }
    case G_TYPE_DOUBLE:
        {
            qv = QVariant::fromValue ("D" + QString::number (g_value_get_double (value)));
            qv.convert (QVariant::String);
        }
        break;
    default:
        if (type == G_TYPE_VALUE_ARRAY)
        {
            GValueArray *array = (GValueArray *) g_value_get_boxed (value);

            QVariantList list;
            guint i;
            for (i = 0; array && i < array->n_values; i++) {
                QVariant cv = _to_qvariant (&(array->values[i]));
                list << cv;
            }

            qv = list;
            qv.convert (QVariant::List);
        }
        else
            g_assert_not_reached ();
    }
    return qv;
}

static void
_from_qvariant (GValue         *value,
                const QVariant &qv)
{
    g_assert (value);
    g_assert (!qv.isNull());

    switch (qv.type()) {
    case QVariant::String:
    case QVariant::Int:
    case QVariant::Double:
    case QVariant::Bool:
        {
            char lt = qv.toString().at(0).toAscii();
            QString lqv = qv.toString().mid(1);
//             qWarning() << "lqv" << lqv;
            switch (lt) {
                case 'S':
                    g_value_init (value, G_TYPE_STRING);
                    g_value_set_string (value, lqv.toUtf8().data());
                    return;
                case 'I':
                    g_value_init (value, G_TYPE_INT);
                    g_value_set_int (value, lqv.toInt());
                    return;
                case 'U':
                    g_value_init (value, G_TYPE_UINT);
                    g_value_set_uint (value, lqv.toUInt());
                    return;
                case 'B':
                    g_value_init (value, G_TYPE_BOOLEAN);
                    g_value_set_boolean (value, lqv.toInt());
                    return;
                case 'F':
                    g_value_init (value, G_TYPE_FLOAT);
                    g_value_set_float (value, lqv.toFloat());
                    return;
                case 'D':
                    g_value_init (value, G_TYPE_DOUBLE);
                    g_value_set_double (value, lqv.toDouble());
                    return;
                default:
                    g_assert_not_reached ();
            }
        }
        return;
    case QVariant::List:
        {
            QStringList list = qv.toStringList();
            g_value_init (value, G_TYPE_VALUE_ARRAY);

            GValueArray *va;

            va = g_value_array_new (list.size());
            foreach (const QString& cv, list) {
                GValue tmp = {0};
                _from_qvariant (&tmp, cv);
                g_value_array_append (va, &tmp);
            }

            g_value_take_boxed (value, va);
        }
        return;
    default:
        g_assert_not_reached ();
    }
}
#else
static QVariant
_to_qvariant (GVariant *value)
{
    QVariant qv;

    switch (g_variant_classify (value)) {
    case G_VARIANT_CLASS_STRING:
        {
            qv = QVariant::fromValue ("S" + QString::fromUtf8 (g_variant_get_string (value, NULL)));
            qv.convert (QVariant::String);
        }
        break;
    case G_VARIANT_CLASS_INT32:
        {
            qv = QVariant::fromValue ("I" + QString::number (g_variant_get_int32 (value)));
            qv.convert (QVariant::String);
        }
        break;
    case G_VARIANT_CLASS_BOOLEAN:
        {
            qv = QVariant::fromValue ("B" + QString::number (g_variant_get_boolean (value)));
            qv.convert (QVariant::String);
        }
        break;
    case G_VARIANT_CLASS_DOUBLE:
        {
            qv = QVariant::fromValue ("D" + QString::number (g_variant_get_double (value)));
            qv.convert (QVariant::String);
        }
        break;
    case G_VARIANT_CLASS_ARRAY:
        {
            QVariantList list;

            QVariant cv;
            GVariantIter iter;
            GVariant *child;
            g_variant_iter_init (&iter, value);
            while ((child = g_variant_iter_next_value (&iter)) != NULL) {
                cv = _to_qvariant (child);
                list << cv;
                g_variant_unref (child);
            }

            qv = list;
            qv.convert (QVariant::List);
        }
        break;
    default:
        break;
    }

    return qv;
}

static GVariant *
_from_qvariant (const QVariant &qv)
{
    g_assert (!qv.isNull());

    switch (qv.type()) {
    case QVariant::String:
    case QVariant::Int:
    case QVariant::Double:
    case QVariant::Bool:
        {
            char lt = qv.toString().at(0).toLatin1();
            QString lqv = qv.toString().mid(1);
            switch (lt) {
                case 'S':
                    return g_variant_new_string (lqv.toUtf8().data());
                case 'I':
                case 'U':
                    return g_variant_new_int32 (lqv.toInt());
                case 'B':
                    return g_variant_new_boolean (lqv.toInt());
                case 'F':
                case 'D':
                    return g_variant_new_double (lqv.toDouble());
                default:
                    g_assert_not_reached ();
            }
        }
    case QVariant::List:
        {
            QStringList list = qv.toStringList();
            char lt = list.first().at(0).toLatin1();
            GVariantBuilder builder;
            switch (lt) {
            case 'S':
                g_variant_builder_init (&builder, G_VARIANT_TYPE("as")); break;
            case 'I':
            case 'U':
                g_variant_builder_init (&builder, G_VARIANT_TYPE("ai")); break;
            case 'F':
            case 'D':
                g_variant_builder_init (&builder, G_VARIANT_TYPE("ad")); break;
            case 'B':
                g_variant_builder_init (&builder, G_VARIANT_TYPE("ab")); break;
            default:
                g_assert_not_reached ();
            }

            foreach (const QString& cv, list) {
                switch (lt) {
                case 'S':
                    g_variant_builder_add (&builder, "s", cv.mid(1).toUtf8().data());
                    break;
                case 'I':
                case 'U':
                    g_variant_builder_add (&builder, "i", cv.mid(1).toInt());
                    break;
                case 'F':
                case 'D':
                    g_variant_builder_add (&builder, "d", cv.mid(1).toDouble());
                    break;
                case 'B':
                    g_variant_builder_add (&builder, "b", cv.mid(1).toInt());
                    break;
                default:
                    g_assert_not_reached ();
                }
            }
            return g_variant_builder_end (&builder);
        }
    default:
        g_assert_not_reached ();
    }
}
#endif

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_config_kconfig_set_value (IBusConfigService      *config,
                               const gchar            *section,
                               const gchar            *name,
                               const GValue           *value,
                               IBusError             **error)
#else
static gboolean
ibus_config_kconfig_set_value (IBusConfigService      *config,
                               const gchar            *section,
                               const gchar            *name,
                               GVariant               *value,
                               GError                **error)
#endif
{
    fprintf(stderr,"set value %s/%s\n",section,name);
    QVariant qv = _to_qvariant (value);
    if (qv.isNull()) {
#if !IBUS_CHECK_VERSION(1,3,99)
        *error = ibus_error_new_from_printf (DBUS_ERROR_FAILED,
                                             "Can not set value [%s->%s]", section, name);
#else
        gchar *str = g_variant_print (value, TRUE);
        *error = g_error_new (G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                        "Can not set config value [%s:%s] to %s.",
                        section, name, str);
        g_free (str);
#endif
        return FALSE;
    }

    /// FIXME dirty workaround to determine list and string
    if (qv.type() == QVariant::List)
        ((IBusConfigKConfig *)config)->kconfig->group (section).writeEntry (name, qv);
    else
        ((IBusConfigKConfig *)config)->kconfig->group (section).writeEntry (name, "@@" + qv.toString().toUtf8());

    ((IBusConfigKConfig *)config)->kconfig->sync();
    return TRUE;
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_config_kconfig_get_value (IBusConfigService      *config,
                               const gchar            *section,
                               const gchar            *name,
                               GValue                 *value,
                               IBusError             **error)
#else
static GVariant *
ibus_config_kconfig_get_value (IBusConfigService      *config,
                               const gchar            *section,
                               const gchar            *name,
                               GError                **error)
#endif
{
    fprintf(stderr,"get value %s/%s\n",section,name);
    /// FIXME dirty workaround to determine list and string
    QVariant qv;
    QString qvs = ((IBusConfigKConfig *)config)->kconfig->group (section).readEntry (name, QString());
    if (!qvs.isEmpty()) {
        if (qvs.startsWith("@@"))
            qv = qvs.mid(2);
        else
            qv = ((IBusConfigKConfig *)config)->kconfig->group (section).readEntry (name, QVariantList());
    }

    if (qv.isNull()) {
#if !IBUS_CHECK_VERSION(1,3,99)
        *error = ibus_error_new_from_printf (DBUS_ERROR_FAILED,
                                             "Can not get value [%s->%s]", section, name);
        return FALSE;
#else
        *error = g_error_new (G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                        "Config value [%s:%s] does not exist.", section, name);
        return NULL;
#endif
    }

#if !IBUS_CHECK_VERSION(1,3,99)
    _from_qvariant (value, qv);
    return TRUE;
#else
    GVariant *variant = _from_qvariant (qv);
    return variant;
#endif
}

static GVariant *
ibus_config_kconfig_get_values (IBusConfigService      *config,
                                const gchar            *section,
                                GError                **error)
{
    fprintf(stderr,"get values %s\n",section);
    KConfigGroup group = ((IBusConfigKConfig *)config)->kconfig->group (section);
    QStringList names = group.keyList();

    if (names.isEmpty()) {
        *error = g_error_new (G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                        "Config values [%s] does not exist.", section);
        return NULL;
    }

    GVariantBuilder *builder = g_variant_builder_new (G_VARIANT_TYPE ("a{sv}"));
    foreach (const QString& name, names) {
        /// FIXME dirty workaround to determine list and string
        QVariant qv;
        QString qvs = group.readEntry (name, QString());
        if (!qvs.isEmpty()) {
            if (qvs.startsWith("@@"))
                qv = qvs.mid(2);
            else
                qv = group.readEntry (name, QVariantList());
        }
        if (!qv.isNull()) {
            GVariant *variant = _from_qvariant (qv);
            g_variant_builder_add (builder, "{sv}", name.toUtf8().data(), variant);
        }
    }

    return g_variant_builder_end (builder);
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_config_kconfig_unset (IBusConfigService      *config,
                           const gchar            *section,
                           const gchar            *name,
                           IBusError             **error)
#else
static gboolean
ibus_config_kconfig_unset_value (IBusConfigService      *config,
                                 const gchar            *section,
                                 const gchar            *name,
                                 GError                **error)
#endif
{
    fprintf(stderr,"unset value %s/%s\n",section,name);
    ((IBusConfigKConfig *)config)->kconfig->group (section).deleteEntry (name);

    return TRUE;
}

IBusConfigKConfig *
#if !IBUS_CHECK_VERSION(1,3,99)
ibus_config_kconfig_new (IBusConnection  *connection)
#else
ibus_config_kconfig_new (GDBusConnection *connection)
#endif
{
    IBusConfigKConfig *config;
    config = (IBusConfigKConfig *) g_object_new (IBUS_TYPE_CONFIG_KCONFIG,
#if !IBUS_CHECK_VERSION(1,3,99)
                                                 "path", IBUS_PATH_CONFIG,
#else
                                                 "object-path", IBUS_PATH_CONFIG,
#endif
                                                 "connection", connection,
                                                 NULL);
    return config;
}
