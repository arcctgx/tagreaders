#ifndef _ALBUMARTIST_C_H_
#define _ALBUMARTIST_C_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Retrieve album artist field from audio file metadata.
 * @param[in] path  Filesystem path of the audio file.
 * @returns Pointer to C string containing album artist. It must be freed by the caller.
 */
char *get_album_artist(const char *path);

#ifdef __cplusplus
}
#endif

#endif
