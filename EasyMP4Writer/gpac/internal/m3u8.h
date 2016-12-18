/**
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005
 *					All rights reserved
 *
 *  This file is part of GPAC
 *
 *  GPAC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  Written by Pierre Souchay for VizionR SAS
 *
 */
#ifndef M3U8_PLAYLIST_H
#define M3U8_PLAYLIST_H
#include <gpac/tools.h>
#include <gpac/list.h>

/**
 * Global Structure
 *
 * For a stream with multiple bandwidths and multiple programs
 *
 * VariantPlayList
 *  |
 *  |_ program id 1
 *  |     |
 *  |     |_ bandwidth X : playlistElement1
 *  |     |- bandwidth Y : playlistElement2
 *  |
 *  |- program id 2
 *        |
 *        |_ bandwidth Z : playlistElement
 *
 * For a "normal" playlist
 *
 * VariantPlayList
 *  |
 *  |_ program id 1
 *        |
 *        |_ bandwidth 0 : playlistElement1
 *
 * Where PlaylistElement can be :
 *  - a stream (real resource)
 *  - a playlist (list of PlaylistElements itself)
 */

#define M3U8_UNKOWN_MIME_TYPE "unknown"

/**
 * Basic Stream structure
 */
typedef struct s_stream {
    u8 i;
} Stream;

/**
 * The playlist contains a list of elements to play
 */
typedef struct s_playList {
    int currentMediaSequence;
    int target_duration;
    int mediaSequenceMin;
    int mediaSequenceMax;
    char is_ended;
    GF_List * elements;
} Playlist;

typedef enum e_playlistElementType  { TYPE_PLAYLIST, TYPE_STREAM, TYPE_UNKNOWN} PlaylistElementType;

/**
 * The Structure containing the playlist element
 */
typedef struct s_playlistElement {
    int durationInfo;
    int bandwidth;
    char * title;
	char * codecs;
    char * url;
    PlaylistElementType elementType;
    union { Playlist playlist;
        Stream stream;
    } element;

} PlaylistElement;

typedef struct s_program {
    int programId;
    GF_List * bitrates;
    int currentBitrateIndex;
} Program;


/**
 * The root playlist, can contains several PlaylistElements structures
 */
typedef struct s_variantPlaylist {
    GF_List * programs;
    int currentProgram;
    Bool playlistNeedsRefresh;
} VariantPlaylist;

/**
 * Creates a new playlist
 * @return NULL if playlist could not be allocated
 *
Playlist * playlist_new();
*/
/**
 * Deletes a given playlist and all of its sub elements
 *
GF_Err playlist_del(Playlist *);
*/

/**
 * Deletes an Playlist element
 */
GF_Err playlist_element_del(PlaylistElement *);

/**
 * Creates a new program properly initialized
 */
Program * program_new(int programId);

/**
 * Deletes the specified program
 */
GF_Err program_del(Program * program);

/**
 * Creates an Playlist element.
 * This element can be either a playlist of a stream according to first parameter.
 * @return NULL if element could not be created. Element deletion will be deleted recusivly by #playlist_del(Playlist*)
 */
PlaylistElement * playlist_element_new(PlaylistElementType elementType, const char * url, const char * title, const char *codecs, int durationInfo);

/**
 * Creates a new VariantPlaylist
 * @return NULL if VariantPlaylist element could not be allocated
 */
VariantPlaylist * variant_playlist_new ();

/**
 * Deletes the given VariantPlaylist and all of its sub elements
 */
GF_Err variant_playlist_del(VariantPlaylist *);

GF_Err playlist_element_dump(const PlaylistElement * e, int indent);

GF_Err variant_playlist_dump(const VariantPlaylist *);

Program * variant_playlist_find_matching_program(const VariantPlaylist *, const u32 programId);

Program * variant_playlist_get_current_program(const VariantPlaylist *);



/**
 * Parse the given playlist file
 * @param file The file from cache to parse
 * @param The playlist to fill. If argument is null, and file is valid, playlist will be allocated
 * @return GF_OK if playlist valid
 */
GF_Err parse_root_playlist(const char * file, VariantPlaylist ** playlist, const char * baseURL);
/**
 * Parse the given playlist file as a subplaylist of an existing playlist
 * @param file The file from cache to parse
 * @param The playlist to fill.
 * @param baseURL base URL of the playlist
 * @param program in which the playlist is parsed
 * @param sub_playlist existing subplaylist element in the @playlist in which the playlist is parsed
 * @return GF_OK if playlist valid
 */
GF_Err parse_sub_playlist(const char * file, VariantPlaylist ** playlist, const char * baseURL, Program * in_program, PlaylistElement *sub_playlist);

#endif /* M3U8_PLAYLIST_H */

