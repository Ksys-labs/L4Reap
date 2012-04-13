/***************************************************************************
 *   Copyright (C) 2005-2007 Stefan Schwarzer, Jens Schneider,             *
 *                           Matthias Hardt, Guido Madaus                  *
 *                                                                         *
 *   Copyright (C) 2007-2008 BerLinux Solutions GbR                        *
 *                           Stefan Schwarzer & Guido Madaus               *
 *                                                                         *
 *   Copyright (C) 2009-2011 BerLinux Solutions GmbH                       *
 *                                                                         *
 *   Authors:                                                              *
 *      Stefan Schwarzer   <stefan.schwarzer@diskohq.org>,                 *
 *      Matthias Hardt     <matthias.hardt@diskohq.org>,                   *
 *      Jens Schneider     <pupeider@gmx.de>,                              *
 *      Guido Madaus       <guido.madaus@diskohq.org>,                     *
 *      Patrick Helterhoff <patrick.helterhoff@diskohq.org>,               *
 *      René Bählkow       <rene.baehlkow@diskohq.org>                     *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License version 2.1 as published by the Free Software Foundation.     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 **************************************************************************/

#ifdef __HAVE_GSTREAMER__

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <string.h>
#include <signal.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifndef DISABLE_FAULT_HANDLER
#include <sys/wait.h>
#endif
#include <locale.h>             /* for LC_ALL */
#include "mmsmedia/mmsgst.h"



/* FIXME: This is just a temporary hack.  We should have a better
 * check for siginfo handling. */
#ifdef SA_SIGINFO
#define USE_SIGINFO
#endif

extern volatile gboolean glib_on_error_halt;

#ifndef DISABLE_FAULT_HANDLER
static void fault_restore (void);
static void fault_spin (void);
static void sigint_restore (void);
static gboolean caught_intr = FALSE;
#endif

static GstElement *pipeline;
static gboolean caught_error = FALSE;
static gboolean tags = FALSE;
static gboolean is_live = FALSE;

#ifndef DISABLE_FAULT_HANDLER
#ifndef USE_SIGINFO
static void
fault_handler_sighandler (int signum)
{
  fault_restore ();

  /* printf is used instead of g_print(), since it's less likely to
   * deadlock */
  switch (signum) {
    case SIGSEGV:
      printf ("Caught SIGSEGV\n");
      break;
    case SIGQUIT:
      printf ("Caught SIGQUIT\n");
      break;
    default:
      printf ("signo:  %d\n", signum);
      break;
  }

  fault_spin ();
}

#else /* USE_SIGINFO */

static void
fault_handler_sigaction (int signum, siginfo_t * si, void *misc)
{
  fault_restore ();

  /* printf is used instead of g_print(), since it's less likely to
   * deadlock */
  switch (si->si_signo) {
    case SIGSEGV:
      printf ("Caught SIGSEGV accessing address %p\n", si->si_addr);
      break;
    case SIGQUIT:
      printf ("Caught SIGQUIT\n");
      break;
    default:
      printf ("signo:  %d\n", si->si_signo);
      printf ("errno:  %d\n", si->si_errno);
      printf ("code:   %d\n", si->si_code);
      break;
  }

  fault_spin ();
}
#endif /* USE_SIGINFO */

static void
fault_spin (void)
{
  int spinning = TRUE;

  glib_on_error_halt = FALSE;
  g_on_error_stack_trace ("gst-launch");

  wait (NULL);

  /* FIXME how do we know if we were run by libtool? */
  printf ("Spinning.  Please run 'gdb gst-launch %d' to continue debugging, "
      "Ctrl-C to quit, or Ctrl-\\ to dump core.\n", (gint) getpid ());
  while (spinning)
    g_usleep (1000000);
}

static void
fault_restore (void)
{
  struct sigaction action;

  memset (&action, 0, sizeof (action));
  action.sa_handler = SIG_DFL;

  sigaction (SIGSEGV, &action, NULL);
  sigaction (SIGQUIT, &action, NULL);
}

static void
fault_setup (void)
{
  struct sigaction action;

  memset (&action, 0, sizeof (action));
#ifdef USE_SIGINFO
  action.sa_sigaction = fault_handler_sigaction;
  action.sa_flags = SA_SIGINFO;
#else
  action.sa_handler = fault_handler_sighandler;
#endif

  sigaction (SIGSEGV, &action, NULL);
  sigaction (SIGQUIT, &action, NULL);
}
#endif /* DISABLE_FAULT_HANDLER */

static void
print_tag (const GstTagList * list, const gchar * tag, gpointer unused)
{
  gint i, count;

  count = gst_tag_list_get_tag_size (list, tag);

  for (i = 0; i < count; i++) {
    gchar *str;

    if (gst_tag_get_type (tag) == G_TYPE_STRING) {
      if (!gst_tag_list_get_string_index (list, tag, i, &str))
        g_assert_not_reached ();
    } else if (gst_tag_get_type (tag) == GST_TYPE_BUFFER) {
      GstBuffer *img;

      img = gst_value_get_buffer (gst_tag_list_get_value_index (list, tag, i));
      if (img) {
        gchar *caps_str;

        caps_str = GST_BUFFER_CAPS (img) ?
            gst_caps_to_string (GST_BUFFER_CAPS (img)) : g_strdup ("unknown");
        str = g_strdup_printf ("buffer of %u bytes, type: %s",
            GST_BUFFER_SIZE (img), caps_str);
        g_free (caps_str);
      } else {
        str = g_strdup ("NULL buffer");
      }
    } else {
      str =
          g_strdup_value_contents (gst_tag_list_get_value_index (list, tag, i));
    }

    if (i == 0) {
      g_print ("%16s: %s\n", gst_tag_get_nick (tag), str);
    } else {
      g_print ("%16s: %s\n", "", str);
    }

    g_free (str);
  }
}

#ifndef DISABLE_FAULT_HANDLER
/* we only use sighandler here because the registers are not important */
static void
sigint_handler_sighandler (int signum)
{
  g_print ("Caught interrupt -- ");

  sigint_restore ();

  /* we set a flag that is checked by the mainloop, we cannot do much in the
   * interrupt handler (no mutex or other blocking stuff) */
  caught_intr = TRUE;
}

/* is called every 50 milliseconds (20 times a second), the interrupt handler
 * will set a flag for us. We react to this by posting a message. */
static gboolean
check_intr (GstElement * pipeline)
{
  if (!caught_intr) {
    return TRUE;
  } else {
    caught_intr = FALSE;
    g_print ("handling interrupt.\n");

    /* post an application specific message */
    gst_element_post_message (GST_ELEMENT (pipeline),
        gst_message_new_application (GST_OBJECT (pipeline),
            gst_structure_new ("GstLaunchInterrupt",
                "message", G_TYPE_STRING, "Pipeline interrupted", NULL)));

    /* remove timeout handler */
    return FALSE;
  }
}

static void
sigint_setup (void)
{
  struct sigaction action;

  memset (&action, 0, sizeof (action));
  action.sa_handler = sigint_handler_sighandler;

  sigaction (SIGINT, &action, NULL);
}

static void
sigint_restore (void)
{
  struct sigaction action;

  memset (&action, 0, sizeof (action));
  action.sa_handler = SIG_DFL;

  sigaction (SIGINT, &action, NULL);
}

static void
play_handler (int signum)
{
  switch (signum) {
    case SIGUSR1:
      g_print ("Caught SIGUSR1 - Play request.\n");
      gst_element_set_state (pipeline, GST_STATE_PLAYING);
      break;
    case SIGUSR2:
      g_print ("Caught SIGUSR2 - Stop request.\n");
      gst_element_set_state (pipeline, GST_STATE_NULL);
      break;
  }
}

static void
play_signal_setup (void)
{
  struct sigaction action;

  memset (&action, 0, sizeof (action));
  action.sa_handler = play_handler;
  sigaction (SIGUSR1, &action, NULL);
  sigaction (SIGUSR2, &action, NULL);
}
#endif /* DISABLE_FAULT_HANDLER */

/* returns TRUE if there was an error or we caught a keyboard interrupt. */
static gboolean
event_loop (GstElement * pipeline, gboolean blocking, GstState target_state)
{
  GstBus *bus;
  GstMessage *message = NULL;
  gboolean res = FALSE;
  gboolean buffering = FALSE;

  bus = gst_element_get_bus (GST_ELEMENT (pipeline));

#ifndef DISABLE_FAULT_HANDLER
  g_timeout_add (50, (GSourceFunc) check_intr, pipeline);
#endif

  while (TRUE) {
    message = gst_bus_poll (bus, GST_MESSAGE_ANY, blocking ? -1 : 0);

    /* if the poll timed out, only when !blocking */
    if (message == NULL)
      goto exit;

    switch (GST_MESSAGE_TYPE (message)) {
      case GST_MESSAGE_NEW_CLOCK:
      {
        GstClock *clock;

        gst_message_parse_new_clock (message, &clock);

        g_print ("New clock: %s\n", (clock ? GST_OBJECT_NAME (clock) : "NULL"));
        break;
      }
      case GST_MESSAGE_EOS:
        g_print ("Got EOS from element \"%s\".\n",
            GST_STR_NULL (GST_ELEMENT_NAME (GST_MESSAGE_SRC (message))));
        goto exit;
      case GST_MESSAGE_TAG:
        if (tags) {
          GstTagList *tags;

          gst_message_parse_tag (message, &tags);
          g_print ("FOUND TAG      : found by element \"%s\".\n",
              GST_STR_NULL (GST_ELEMENT_NAME (GST_MESSAGE_SRC (message))));
          gst_tag_list_foreach (tags, print_tag, NULL);
          gst_tag_list_free (tags);
        }
        break;
      case GST_MESSAGE_INFO:{
        GError *gerror;
        gchar *debug;
        gchar *name = gst_object_get_path_string (GST_MESSAGE_SRC (message));

        gst_message_parse_info (message, &gerror, &debug);
        if (debug) {
          g_print ("INFO:\n%s\n", debug);
        }
        g_error_free (gerror);
        g_free (debug);
        g_free (name);
        break;
      }
      case GST_MESSAGE_WARNING:{
        GError *gerror;
        gchar *debug;
        gchar *name = gst_object_get_path_string (GST_MESSAGE_SRC (message));

        /* dump graph on warning */
        GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (pipeline),
            GST_DEBUG_GRAPH_SHOW_ALL, "gst-launch.warning");

        gst_message_parse_warning (message, &gerror, &debug);
        g_print ("WARNING: from element %s: %s\n", name, gerror->message);
        if (debug) {
          g_print ("Additional debug info:\n%s\n", debug);
        }
        g_error_free (gerror);
        g_free (debug);
        g_free (name);
        break;
      }
      case GST_MESSAGE_ERROR:{
        GError *gerror;
        gchar *debug;

        /* dump graph on error */
        GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (pipeline),
            GST_DEBUG_GRAPH_SHOW_ALL, "gst-launch.error");

        gst_message_parse_error (message, &gerror, &debug);
        gst_object_default_error (GST_MESSAGE_SRC (message), gerror, debug);
        g_error_free (gerror);
        g_free (debug);
        /* we have an error */
        res = TRUE;
        goto exit;
      }
      case GST_MESSAGE_STATE_CHANGED:{
        GstState old, newX, pending;

        gst_message_parse_state_changed (message, &old, &newX, &pending);

        /* we only care about pipeline state change messages */
        if (GST_MESSAGE_SRC (message) != GST_OBJECT_CAST (pipeline))
          break;

        /* dump graph for pipeline state changes */
        {
          gchar *dump_name = g_strdup_printf ("gst-launch.%s_%s",
              gst_element_state_get_name (old),
              gst_element_state_get_name (newX));
          GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (pipeline),
              GST_DEBUG_GRAPH_SHOW_ALL, dump_name);
          g_free (dump_name);
        }

        /* ignore when we are buffering since then we mess with the states
         * ourselves. */
        if (buffering) {
          fprintf (stderr,
              "Prerolled, waiting for buffering to finish...\n");
          break;
        }

        /* if we reached the final target state, exit */
        if (target_state == GST_STATE_PAUSED && newX == target_state)
          goto exit;

        /* else not an interesting message */
        break;
      }
      case GST_MESSAGE_BUFFERING:{
        gint percent;

        gst_message_parse_buffering (message, &percent);
        fprintf (stderr, "%s %d%%  \r", "buffering...", percent);

        /* no state management needed for live pipelines */
        if (is_live)
          break;

        if (percent == 100) {
          /* a 100% message means buffering is done */
          buffering = FALSE;
          /* if the desired state is playing, go back */
          if (target_state == GST_STATE_PLAYING) {
            fprintf (stderr,
                "Done buffering, setting pipeline to PLAYING ...\n");
            gst_element_set_state (pipeline, GST_STATE_PLAYING);
          } else
            goto exit;
        } else {
          /* buffering busy */
          if (buffering == FALSE && target_state == GST_STATE_PLAYING) {
            /* we were not buffering but PLAYING, PAUSE  the pipeline. */
            fprintf (stderr, "Buffering, setting pipeline to PAUSED ...\n");
            gst_element_set_state (pipeline, GST_STATE_PAUSED);
          }
          buffering = TRUE;
        }
        break;
      }
      case GST_MESSAGE_LATENCY:
      {
        fprintf (stderr, "Redistribute latency...\n");
        gst_bin_recalculate_latency (GST_BIN (pipeline));
        break;
      }
      case GST_MESSAGE_APPLICATION:{
        const GstStructure *s;

        s = gst_message_get_structure (message);

        if (gst_structure_has_name (s, "GstLaunchInterrupt")) {
          /* this application message is posted when we caught an interrupt and
           * we need to stop the pipeline. */
          fprintf (stderr, "Interrupt: Stopping pipeline ...\n");
          /* return TRUE when we caught an interrupt */
          res = TRUE;
          goto exit;
        }
      }
      default:
        /* just be quiet by default */
        break;
    }
    if (message)
      gst_message_unref (message);
  }
  g_assert_not_reached ();

exit:
  {
    if (message)
      gst_message_unref (message);
    gst_object_unref (bus);
    return res;
  }
}


void mmsGstFree() {

	if (pipeline) {
	    GstState state, pending;

	    fprintf (stderr, "Setting pipeline to NULL ...\n");
		gst_element_set_state (pipeline, GST_STATE_NULL);
		gst_element_get_state (pipeline, &state, &pending, GST_CLOCK_TIME_NONE);

		fprintf (stderr, "FREEING pipeline ...\n");
		gst_object_unref (pipeline);
	}

	gst_deinit ();
}


GstElement *mmsGstLaunch(const char *pipeline_description) {
	GError *error = NULL;
	gint res = 0;

#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

	if (!g_thread_supported ())
		g_thread_init (NULL);

	gst_alloc_trace_set_flags_all (GST_ALLOC_TRACE_LIVE);

#ifndef DISABLE_FAULT_HANDLER
	fault_setup ();

	sigint_setup ();
	play_signal_setup ();
#endif

	// parse the pipeline description
	pipeline = gst_parse_launch(pipeline_description, &error);

	if (!pipeline) {
		if (error) {
			fprintf (stderr, "ERROR: pipeline could not be constructed: %s.\n",
			GST_STR_NULL (error->message));
			g_error_free (error);
		} else {
			fprintf (stderr, "ERROR: pipeline could not be constructed.\n");
		}
		return NULL;
	} else if (error) {
		fprintf (stderr, "WARNING: erroneous pipeline: %s\n",
		GST_STR_NULL (error->message));
		g_error_free (error);
		return NULL;
	}


    GstState state;
    GstStateChangeReturn ret;

    /* If the top-level object is not a pipeline, place it in a pipeline. */
    if (!GST_IS_PIPELINE (pipeline)) {
      GstElement *real_pipeline = gst_element_factory_make ("pipeline", NULL);

      if (real_pipeline == NULL) {
        fprintf (stderr, "ERROR: the 'pipeline' element wasn't found.\n");
        return NULL;
      }
      gst_bin_add (GST_BIN (real_pipeline), pipeline);
      pipeline = real_pipeline;
    }
    fprintf (stderr, "Setting pipeline to PAUSED ...\n");
    ret = gst_element_set_state (pipeline, GST_STATE_PAUSED);

    switch (ret) {
    case GST_STATE_CHANGE_FAILURE:
		fprintf (stderr, "ERROR: Pipeline doesn't want to pause.\n");
		res = -1;
		event_loop (pipeline, FALSE, GST_STATE_VOID_PENDING);
		goto end;
    case GST_STATE_CHANGE_NO_PREROLL:
		fprintf (stderr, "Pipeline is live and does not need PREROLL ...\n");
		is_live = TRUE;
		break;
    case GST_STATE_CHANGE_ASYNC:
		fprintf (stderr, "Pipeline is PREROLLING ...\n");
		caught_error = event_loop (pipeline, TRUE, GST_STATE_PAUSED);
		if (caught_error) {
		  fprintf (stderr, "ERROR: pipeline doesn't want to preroll.\n");
		  goto end;
		}
		state = GST_STATE_PAUSED;
		// fallthrough
	case GST_STATE_CHANGE_SUCCESS:
		fprintf (stderr, "Pipeline is PREROLLED ...\n");
		break;
    }

    // pipe successfully created
    return pipeline;

end:
	mmsGstFree();

	return NULL;
}




GstElement *mmsGstInit(const string uri, MMSFBSurface *surface) {

	// initialize gstreamer
	gst_init(NULL, NULL);
	if (uri == "")
		return NULL;

	if (strToUpr(uri.substr(0,6)) == "GST://") {
		// the uri is an gstreamer pipeline
		// so we use the pipeline and do NOT use playbin with disko video sink
		return mmsGstLaunch(uri.substr(6).c_str());
	}
	else {
		// create top-level pipe element
		pipeline = gst_element_factory_make("playbin", "player");

		// get the diskovideosink element
		GstElement *videosink = gst_element_factory_make("diskovideosink", "diskovideosink");

		// set the surface to the diskovideosink element
		g_object_set(videosink, "surface", surface, NULL);

		// set videosink to the player
		g_object_set(G_OBJECT(pipeline), "video-sink", videosink, NULL);

		// set the uri to the player
		g_object_set(G_OBJECT(pipeline), "uri", uri.c_str(), NULL);

		return pipeline;
	}
}

GstElement *mmsGstInit(const string uri, MMSWindow *window) {

	// initialize gstreamer
	gst_init(NULL, NULL);
	if (uri == "")
		return NULL;

	if (strToUpr(uri.substr(0,6)) == "GST://") {
		// the uri is an gstreamer pipeline
		// so we use the pipeline and do NOT use playbin with disko video sink
		return mmsGstLaunch(uri.substr(6).c_str());
	}
	else {
		// create top-level pipe element
		pipeline = gst_element_factory_make("playbin", "player");

		// get the diskovideosink element
		GstElement *videosink = gst_element_factory_make("diskovideosink", "diskovideosink");

		// set the surface to the diskovideosink element
		// note: if using a window, the diskovideosink can handle keyboard,
		//       mouse, touchscreen inputs which are delivered to the MMSWindow
		g_object_set(videosink, "window", window, NULL);

		// set videosink to the player
		g_object_set(G_OBJECT(pipeline), "video-sink", videosink, NULL);

		// set the uri to the player
		g_object_set(G_OBJECT(pipeline), "uri", uri.c_str(), NULL);

		return pipeline;
	}
}


bool mmsGstPlay(GstElement *pipelineX) {

	pipeline = pipelineX;


	GstState state, pending;


    caught_error = event_loop (pipeline, FALSE, GST_STATE_PLAYING);

    if (caught_error) {
      fprintf (stderr, "ERROR: pipeline doesn't want to preroll.\n");
    } else {
      GstClockTime tfthen, tfnow;
      GstClockTimeDiff diff;

      fprintf (stderr, "Setting pipeline to PLAYING ...\n");
      if (gst_element_set_state (pipeline,
              GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
        GstMessage *err_msg;
        GstBus *bus;

        fprintf (stderr, "ERROR: pipeline doesn't want to play.\n");
        bus = gst_element_get_bus (pipeline);
        if ((err_msg = gst_bus_poll (bus, GST_MESSAGE_ERROR, 0))) {
          GError *gerror;
          gchar *debug;

          gst_message_parse_error (err_msg, &gerror, &debug);
          gst_object_default_error (GST_MESSAGE_SRC (err_msg), gerror, debug);
          gst_message_unref (err_msg);
          g_error_free (gerror);
          g_free (debug);
        }
        gst_object_unref (bus);

        mmsGstFree();

        return false;
      }

      tfthen = gst_util_get_timestamp ();
      caught_error = event_loop (pipeline, TRUE, GST_STATE_PLAYING);
      tfnow = gst_util_get_timestamp ();

      diff = GST_CLOCK_DIFF (tfthen, tfnow);

      g_print ("Execution ended after %" G_GUINT64_FORMAT " ns.\n", diff);
    }

    /* iterate mainloop to process pending stuff */
    while (g_main_context_iteration (NULL, FALSE));

    fprintf (stderr, "Setting pipeline to PAUSED ...\n");
    gst_element_set_state (pipeline, GST_STATE_PAUSED);
    if (!caught_error)
      gst_element_get_state (pipeline, &state, &pending, GST_CLOCK_TIME_NONE);
    fprintf (stderr, "Setting pipeline to READY ...\n");
    gst_element_set_state (pipeline, GST_STATE_READY);
    gst_element_get_state (pipeline, &state, &pending, GST_CLOCK_TIME_NONE);

    // playback is finished
    return true;
}


bool mmsGstSendKeyPress(GstElement *pipeline, MMSKeySymbol key) {
	if (!pipeline)
		return false;

	// if keysym string is empty, do nothing but return success
	const char *ks = convertMMSKeySymbolToXKeysymString(key);
	if (!*ks)
		return true;

	// construct event
	GstStructure *structure =
		gst_structure_new(	"application/x-gst-navigation",
							"event",	G_TYPE_STRING,	"key-press",
							"key",		G_TYPE_STRING,	ks,
							NULL);
	if (!structure)
		return false;
	GstEvent *event = gst_event_new_navigation(structure);
	if (!event)
		return false;

	// send event
	return gst_element_send_event(pipeline, event);
}

bool mmsGstSendKeyRelease(GstElement *pipeline, MMSKeySymbol key) {
	if (!pipeline)
		return false;

	// if keysym string is empty, do nothing but return success
	const char *ks = convertMMSKeySymbolToXKeysymString(key);
	if (!*ks)
		return true;

	// construct event
	GstStructure *structure =
		gst_structure_new(	"application/x-gst-navigation",
							"event",	G_TYPE_STRING,	"key-release",
							"key",		G_TYPE_STRING,	ks,
							NULL);
	if (!structure)
		return false;
	GstEvent *event = gst_event_new_navigation(structure);
	if (!event)
		return false;

	// send event
	return gst_element_send_event(pipeline, event);
}

bool mmsGstSendButtonPress(GstElement *pipeline, int posx, int posy) {
	if (!pipeline)
		return false;

	// construct event
	GstStructure *structure =
		gst_structure_new(	"application/x-gst-navigation",
							"event",	G_TYPE_STRING,	"mouse-button-press",
							"button",	G_TYPE_INT,		0,
							"pointer_x",G_TYPE_DOUBLE,	(double)posx,
							"pointer_y",G_TYPE_DOUBLE,	(double)posy,
							NULL);
	if (!structure)
		return false;
	GstEvent *event = gst_event_new_navigation(structure);
	if (!event)
		return false;

	// send event
	return gst_element_send_event(pipeline, event);
}

bool mmsGstSendButtonRelease(GstElement *pipeline, int posx, int posy) {
	if (!pipeline)
		return false;

	// construct event
	GstStructure *structure =
		gst_structure_new(	"application/x-gst-navigation",
							"event",	G_TYPE_STRING,	"mouse-button-release",
							"button",	G_TYPE_INT,		0,
							"pointer_x",G_TYPE_DOUBLE,	(double)posx,
							"pointer_y",G_TYPE_DOUBLE,	(double)posy,
							NULL);
	if (!structure)
		return false;
	GstEvent *event = gst_event_new_navigation(structure);
	if (!event)
		return false;

	// send event
	return gst_element_send_event(pipeline, event);
}

bool mmsGstSendAxisMotion(GstElement *pipeline, int posx, int posy) {
	if (!pipeline)
		return false;

	// construct event
	GstStructure *structure =
		gst_structure_new(	"application/x-gst-navigation",
							"event",	G_TYPE_STRING,	"mouse-move",
							"button",	G_TYPE_INT,		0,
							"pointer_x",G_TYPE_DOUBLE,	(double)posx,
							"pointer_y",G_TYPE_DOUBLE,	(double)posy,
							NULL);
	if (!structure)
		return false;
	GstEvent *event = gst_event_new_navigation(structure);
	if (!event)
		return false;

	// send event
	return gst_element_send_event(pipeline, event);
}


#endif

