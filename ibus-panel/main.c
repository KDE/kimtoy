/*
 *  This file is part of KIMToy, an input method frontend for KDE
 *  Copyright (C) 2011-2012 Ni Hui <shuizhuyuanluo@126.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ibus.h>
#include <stdlib.h>
#include <locale.h>
#include "panel.h"

static IBusBus *bus = NULL;
static IBusPanelImpanel *impanel = NULL;

#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef struct _x11_source {
    GSource source;
    Display *dpy;
} x11_source_t;

static gboolean
x11_fd_prepare(GSource *source, gint *timeout)
{
    *timeout = -1;
    return FALSE;
}

static gboolean
x11_fd_check (GSource *source)
{
    return TRUE;
}

static gboolean
x11_fd_dispatch(GSource* source, GSourceFunc callback, gpointer user_data)
{
    Display *dpy = ((x11_source_t*)source)->dpy;

    XEvent e;

    while (XPending(dpy))
    {

        XNextEvent(dpy, &e);
        if (e.type == KeyPress)
        {
//             g_print("KeyPress!!!\n");

            static const gchar *names[64] = {0};
            names[0] = "xkb:us::eng";

            static int inited = 0;
            if (!inited)
            {
                IBusConfig *config = ibus_bus_get_config (bus);
                GVariant *engines = ibus_config_get_value (config, "general", "preload_engines");

                int i = 1;

                GVariantIter iter;
                GVariant *child;
                g_variant_iter_init (&iter, engines);
                while ((child = g_variant_iter_next_value (&iter)) != NULL) {
                    const gchar *engine_name = g_variant_get_string (child, NULL);
//                     g_print("engine_name %s\n", engine_name);
                    names[i] = engine_name;
                    i++;
                    g_variant_unref (child);
                }

                inited = 1;
            }

            static int n = 0;
            n = n==0? 1 : 0;
            ibus_bus_set_global_engine(bus, names[n]);
        }

    }

    return TRUE;
}

/* options */
static gboolean ibus = FALSE;
static gboolean verbose = FALSE;

static GMainLoop *main_loop = NULL;

static const GOptionEntry entries[] =
{
    { "ibus", 'i', 0, G_OPTION_ARG_NONE, &ibus, "component is executed by ibus", NULL },
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "verbose", NULL },
    { NULL },
};

static void
ibus_disconnected_cb (IBusBus  *bus,
                      gpointer  user_data)
{
#if !IBUS_CHECK_VERSION(1,4,99)
    ibus_quit ();
#else
    if (main_loop) {
        g_main_loop_quit (main_loop);
    }
#endif
}

static void
ibus_impanel_start (void)
{
    ibus_init ();
    bus = ibus_bus_new ();
    if (!ibus_bus_is_connected (bus)) {
        exit (-1);
    }
    g_signal_connect (bus, "disconnected", G_CALLBACK (ibus_disconnected_cb), NULL);
    impanel = ibus_panel_impanel_new (ibus_bus_get_connection (bus));
    ibus_bus_request_name (bus, IBUS_SERVICE_PANEL, 0);
    ibus_panel_impanel_set_bus(impanel, bus);
#if !IBUS_CHECK_VERSION(1,4,99)
    ibus_main ();
#else

    Display*    dpy     = XOpenDisplay(0);
    Window      root    = DefaultRootWindow(dpy);
    XEvent      ev;

    unsigned int    modifiers       = ControlMask;
    int             keycode         = XKeysymToKeycode(dpy, XK_space);

    XGrabKey(dpy, keycode, modifiers, root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, keycode, modifiers | LockMask, root, False, GrabModeAsync, GrabModeAsync); // capslock
    XGrabKey(dpy, keycode, modifiers | Mod2Mask, root, False, GrabModeAsync, GrabModeAsync); // numlock
    XGrabKey(dpy, keycode, modifiers | LockMask | Mod2Mask, root, False, GrabModeAsync, GrabModeAsync);

    XSelectInput(dpy, root, KeyPressMask);

//     XCloseDisplay(dpy);

    main_loop = g_main_loop_new(NULL, FALSE);

    int dpyfd = ConnectionNumber(dpy);
    GPollFD dpy_pollfd = {dpyfd, G_IO_IN | G_IO_HUP | G_IO_ERR, 0};

    GSourceFuncs x11_source_funcs = {
        x11_fd_prepare,
        x11_fd_check,
        x11_fd_dispatch,
        NULL, /* finalize */
        NULL, /* closure_callback */
        NULL /* closure_marshal */
    };

    GSource *x11_source = g_source_new(&x11_source_funcs, sizeof(x11_source_t));
    ((x11_source_t*)x11_source)->dpy = dpy;

    g_source_add_poll(x11_source, &dpy_pollfd);
    g_source_set_can_recurse(x11_source, TRUE);
    g_source_attach(x11_source, NULL);

    g_main_loop_run (main_loop);

    g_main_loop_unref (main_loop);
    main_loop = NULL;

    XUngrabKey(dpy, keycode, modifiers, root);
    XUngrabKey(dpy, keycode, modifiers | LockMask, root);
    XUngrabKey(dpy, keycode, modifiers | Mod2Mask, root);
    XUngrabKey(dpy, keycode, modifiers | LockMask | Mod2Mask, root);

    XCloseDisplay(dpy);
#endif
}

gint
main (gint argc, gchar **argv)
{
    GError *error = NULL;
    GOptionContext *context;

    setlocale (LC_ALL, "");

    context = g_option_context_new ("- ibus impanel component");

    g_option_context_add_main_entries (context, entries, "ibus-impanel");

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_print ("Option parsing failed: %s\n", error->message);
        exit (-1);
    }

    ibus_impanel_start ();

    return 0;
}

