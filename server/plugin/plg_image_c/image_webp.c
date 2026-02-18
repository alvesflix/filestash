#include <stdio.h>
#include <stdlib.h>
#include <webp/decode.h>
#include <webp/encode.h>
#include "utils.h"

#define WEBP_QUALITY 75
#define INITIAL_BUFFER_SIZE 1024*64 // 64kB
#define MAX_BUFFER_SIZE 1024*1024*16 // 16MB

static int passthrough_webp(FILE* output, const uint8_t* data, size_t data_size) {
  if (data == NULL || data_size == 0) return 1;
  if (fwrite(data, data_size, 1, output) != 1) return 1;
  fflush(output);
  return 0;
}

int webp_to_webp(int inputDesc, int outputDesc, int targetSize) {
#ifdef HAS_DEBUG
  clock_t t;
  t = clock();
#endif
  if (targetSize < 0) {
    targetSize = -targetSize;
  }
  int status = 0;
  FILE* input = fdopen(inputDesc, "rb");
  FILE* output = fdopen(outputDesc, "wb");
  if (!input || !output) {
    ERROR("setup");
    return 1;
  }

  // STEP1: setup everything
  size_t data_size = 0;
  size_t buffer_size = INITIAL_BUFFER_SIZE;
  uint8_t* data = (uint8_t*)malloc(buffer_size);
  if (!data) {
    ERROR("malloc");
    return 1;
  }
  size_t bytes_read;
  while ((bytes_read = fread(data + data_size, 1, buffer_size - data_size, input)) > 0) {
    data_size += bytes_read;
    if (buffer_size - data_size == 0) {
      DEBUG("realloc");
      if (buffer_size >= MAX_BUFFER_SIZE) {
        free(data);
        ERROR("abort");
        return 1;
      }
      buffer_size *= 2;
      if (buffer_size > MAX_BUFFER_SIZE) buffer_size = MAX_BUFFER_SIZE;
      uint8_t* new_data = (uint8_t*)realloc(data, buffer_size);
      if (!new_data) {
        free(data);
        ERROR("realloc");
        return 1;
      }
      data = new_data;
    }
  }

  // STEP2: decode
  int width, height, scale_factor;
  if (!WebPGetInfo(data, data_size, &width, &height)) {
    int passthrough_status = passthrough_webp(output, data, data_size);
    free(data);
    if (passthrough_status != 0) {
      ERROR("init");
      return 1;
    }
    return 0;
  }
  DEBUG("init");
  WebPDecoderConfig config;
  if (!WebPInitDecoderConfig(&config)) {
    int passthrough_status = passthrough_webp(output, data, data_size);
    free(data);
    if (passthrough_status != 0) {
      ERROR("config");
      return 1;
    }
    return 0;
  }
  scale_factor = (height > targetSize) ? height / targetSize : 1;
  config.options.use_scaling = 1;
  config.options.scaled_width = width / scale_factor;
  config.options.scaled_height = height / scale_factor;
  config.output.colorspace = MODE_rgbA;
  DEBUG("config");
  if (WebPDecode(data, data_size, &config) != VP8_STATUS_OK) {
    WebPFreeDecBuffer(&config.output);
    int passthrough_status = passthrough_webp(output, data, data_size);
    free(data);
    if (passthrough_status != 0) {
      ERROR("decode");
      return 1;
    }
    return 0;
  }
  DEBUG("decode");

  // STEP3: encode
  size_t output_size = 0;
  uint8_t* output_data = NULL;
  output_size = WebPEncodeRGBA(
      config.output.u.RGBA.rgba, config.options.scaled_width,
      config.options.scaled_height, config.output.u.RGBA.stride,
      WEBP_QUALITY, &output_data
  );
  if (output_data == NULL) {
    WebPFreeDecBuffer(&config.output);
    int passthrough_status = passthrough_webp(output, data, data_size);
    free(data);
    if (passthrough_status != 0) {
      ERROR("encode");
      return 1;
    }
    return 0;
  }
  DEBUG("encode");
  fwrite(output_data, output_size, 1, output);
  fflush(output);
  WebPFree(output_data);
  WebPFreeDecBuffer(&config.output);
  free(data);
  DEBUG("done");
  return status;
}
