This is a set of simple programs for reading metadata of audio files.

* `id3v1read` reads `ID3v1` metadata from `.mp3` files. It has no dependencies
  on external libraries.

* `tagread` is based on `tagblib` and reads metadata from all types of files
  supported by that library.

* `csvscrob` reads metadata and writes the output in format compatible with
  [Universal Scrobbler](http://universalscrobbler.com) "bulk scrobble" mode.
  It used to be a part of `tagread`, but later became a separate application.

* `isvbr` checks if an `.mp3` file has variable or constant bitrate. It serves
  as an example of `Mutagen` library usage.
