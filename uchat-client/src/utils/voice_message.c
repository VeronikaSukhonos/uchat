#include <gst/gst.h>
#include <uchat.h>

#define MAX_DB_LEVEL 60.0 // Define a max dB level for scaling
#define NUM_BARS 60       // Number of bars in the waveform

static gboolean recording = FALSE;
static GstElement *pipeline = NULL;
static GstElement *level = NULL;
static GtkWidget *waveform_area = NULL;
static int level_heights[NUM_BARS] = {0};

void start_recording(const char *output_path) {
  GstElement *source, *encoder, *sink;
  gst_init(NULL, NULL);

  source = gst_element_factory_make("autoaudiosrc", "source");
  level = gst_element_factory_make("level", "audio-level");
  encoder = gst_element_factory_make("wavenc", "encoder");
  sink = gst_element_factory_make("filesink", "sink");

  g_object_set(sink, "location", output_path, NULL);
  pipeline = gst_pipeline_new("audio-recorder");
  gst_bin_add_many(GST_BIN(pipeline), source, level, encoder, sink, NULL);

  if (!gst_element_link_many(source, level, encoder, sink, NULL)) {
    g_printerr("Elements could not be linked.\n");
    gst_object_unref(pipeline);
    pipeline = NULL;
    return;
  }

  GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
  gst_bus_add_signal_watch(bus);
  // g_signal_connect(bus, "message", G_CALLBACK(on_audio_level_message), NULL);
  gst_object_unref(bus);

  gst_element_set_state(pipeline, GST_STATE_PLAYING);
  g_print("Recording started...\n");
}

// Function to stop recording
void stop_recording() {
  if (pipeline) {
    gst_element_set_state(pipeline, GST_STATE_NULL);
    g_print("Recording stopped.\n");
    gst_object_unref(pipeline);
    pipeline = NULL;
  }
}