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

void send_voice_message(int sock, const char *file_path, int chat_id) {
  // Read and encode the file into base64
  char *encoded_file = read_and_encode_file(file_path);
  if (!encoded_file) {
    g_printerr("Failed to encode file: %s\n", file_path);
    return;
  }

  // Create JSON object for the message
  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "action", "SEND_VOICE_MESSAGE_TO_CHAT");
  cJSON_AddNumberToObject(json, "chat_id", chat_id);
  cJSON_AddStringToObject(json, "file",
                          encoded_file); // Add encoded file content

  // Convert JSON object to string
  char *json_string = cJSON_PrintUnformatted(json);

  // Ensure the entire JSON message is sent
  ssize_t sent = send(sock, json_string, strlen(json_string), 0);
  if (sent < 0) {
    perror("Failed to send voice message");
  } else {
    g_print("Voice message sent successfully. with size %zi\n", sent);
  }

  // Cleanup
  free(encoded_file);
  free(json_string);
  cJSON_Delete(json);
}
