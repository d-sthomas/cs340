#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int png_dimensions(const char *png_filename, uint32_t *width, uint32_t *height);

#ifdef __cplusplus
}

#endif