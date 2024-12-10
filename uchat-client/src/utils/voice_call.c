#include <gst/gst.h>
#include <uchat.h>

GstElement *send_pipeline = NULL;
GstElement *receive_pipeline = NULL;

void start_receive_pipeline() {
  char pipeline_str[512];
  snprintf(pipeline_str, sizeof(pipeline_str),
           "udpsrc port=%d caps=\"application/x-rtp\" ! rtpopusdepay ! opusdec "
           "! audioconvert ! autoaudiosink",
           receive_port);

  receive_pipeline = gst_parse_launch(pipeline_str, NULL);

  if (!receive_pipeline) {
    // g_print("Failed to create receive pipeline\n");
    return;
  }

  gst_element_set_state(receive_pipeline, GST_STATE_PLAYING);
  // g_print("Audio receive pipeline started on port %d\n", receive_port);
}

void start_send_pipeline(const char *host, int port) {
  char pipeline_str[512];
  snprintf(pipeline_str, sizeof(pipeline_str),
           "autoaudiosrc ! audioconvert ! audioresample ! opusenc ! rtpopuspay "
           "! udpsink host=%s port=%d",
           host, port);

  send_pipeline = gst_parse_launch(pipeline_str, NULL);

  if (!send_pipeline) {
    // g_print("Failed to create send pipeline\n");
    return;
  }

  gst_element_set_state(send_pipeline, GST_STATE_PLAYING);
  // g_print("Audio send pipeline started, sending to %s:%d\n", host, port);
}

void stop_send_pipeline() {
  if (send_pipeline) {
    // g_print("Stopping the send pipeline...\n");
    gst_element_set_state(send_pipeline, GST_STATE_NULL); // Stop the pipeline
    gst_object_unref(send_pipeline); // Unreference the pipeline to clean up
    send_pipeline = NULL;            // Nullify the pipeline pointer
    // g_print("Send pipeline stopped.\n");
  }
}

void stop_receive_pipeline() {
  if (receive_pipeline) {
    // g_print("Stopping the receive pipeline...\n");
    gst_element_set_state(receive_pipeline,
                          GST_STATE_NULL); // Stop the pipeline
    gst_object_unref(receive_pipeline); // Unreference the pipeline to clean up
    receive_pipeline = NULL;            // Nullify the pipeline pointer
    // g_print("Receive pipeline stopped.\n");
  }
}