/*
 *                      GPAC - Multimedia Framework C SDK
 *
 *                      Copyright (c) Jean Le Feuvre 2000-2005
 *                                      All rights reserved
 *
 *  This file is part of GPAC / common tools sub-project
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
 *
 */

#ifndef _GF_CACHE_H_
#define _GF_CACHE_H_

/*!
 *      \file <gpac/cache.h>
 *      \brief Cache management functions.
 */

/*!
 *      \addtogroup dld_grp downloader
 *      \ingroup utils_grp
 *      \brief File Cache Downloader objects
 *
 *      This section documents the file caching tools the GPAC framework.
 *
 *      @{
 */


#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/tools.h>

    /**
     * Handle for Cache Entries.
     * You can use the gf_cache_get_* functions to get the cache properties
     */
    typedef struct __DownloadedCacheEntryStruct * DownloadedCacheEntry;


    typedef struct __CacheReaderStruct * GF_CacheReader;

    /**
     * Free The DownloadedCacheEntry handle
     * \param entry The entry to delete
     * \return GF_OK
     */
    GF_Err gf_cache_delete_entry( const DownloadedCacheEntry entry );

    /**
     * Get the ETag associated with this cache entry if any
     * \param entry The entry
     * \return The ETag if any was defined, NULL otherwise
     */
    const char * gf_cache_get_etag_on_server( const DownloadedCacheEntry entry );

    /**
     * Set the eTag in the cache. Data is duplicated, so original string can be freed by caller.
     * \param entry The entry
     * \param eTag The eTag to set
     * \return GF_OK if entry and eTag are valid, GF_BAD_PARAM otherwise
     */
    GF_Err gf_cache_set_etag_on_disk(const DownloadedCacheEntry entry, const char * eTag );

    /**
     * Get the ETag associated with this cache entry if any
     * \param entry The entry
     * \return The ETag if any was defined, NULL otherwise
     */
    const char * gf_cache_get_etag_on_disk( const DownloadedCacheEntry entry );

    /**
     * Set the eTag in the cache. Data is duplicated, so original string can be freed by caller.
     * \param entry The entry
     * \param eTag The eTag to set
     * \return GF_OK if entry and eTag are valid, GF_BAD_PARAM otherwise
     */
    GF_Err gf_cache_set_etag_on_server(const DownloadedCacheEntry entry, const char * eTag );


    /**
     * Get the Mime-Type associated with this cache entry.
     * \param entry The entry
     * \return The Mime-Type (never NULL if entry is valid)
     */
    const char * gf_cache_get_mime_type( const DownloadedCacheEntry entry );

    /**
     * Set the Mime-Type in the cache. Data is duplicated, so original string can be freed by caller.
     * \param entry The entry
     * \param eTag The mime-type to set
     * \return GF_OK if entry and mime-type are valid, GF_BAD_PARAM otherwise
     */
    GF_Err gf_cache_set_mime_type(const DownloadedCacheEntry entry, const char * mime_type );

    /**
     * Get the URL associated with this cache entry.
     * \param entry The entry
     * \return The Hash key (never NULL if entry is valid)
     */
    const char * gf_cache_get_url( const DownloadedCacheEntry entry );

    /**
     * Get the Hash Key associated with this cache entry.
     * \param entry The entry
     * \return The Hash key (never NULL if entry is valid)
     */
    const char * gf_cache_get_hash( const DownloadedCacheEntry entry );

    /**
     * Tells whether a cache entry should be cached safely (no
     * \param entry The entry
     * \return 1 if entry should be cached
     */
    Bool gf_cache_can_be_cached( const DownloadedCacheEntry entry );

    /**
     * Get the Last-Modified information associated with this cache entry.
     * \param entry The entry
     * \return The Last-Modified header (can be NULL)
     */
    const char * gf_cache_get_last_modified_on_disk ( const DownloadedCacheEntry entry );

    /**
     * Get the Last-Modified information associated with this cache entry.
     * \param entry The entry
     * \return The Last-Modified header (can be NULL)
     */
    const char * gf_cache_get_last_modified_on_server ( const DownloadedCacheEntry entry );

    /**
     * Set the Last-Modified header for this cache entry
     * \param entry The entry
     * \param newLastModified The new value to set, will be duplicated
     * \return GF_OK if everything went alright, GF_BAD_PARAM if entry is NULL
     */
    GF_Err gf_cache_set_last_modified_on_disk ( const DownloadedCacheEntry entry, const char * newLastModified );

    /**
     * Set the Last-Modified header for this cache entry
     * \param entry The entry
     * \param newLastModified The new value to set, will be duplicated
     * \return GF_OK if everything went alright, GF_BAD_PARAM if entry is NULL
     */
    GF_Err gf_cache_set_last_modified_on_server ( const DownloadedCacheEntry entry, const char * newLastModified );

    /**
     * Get the file name of cache associated with this cache entry.
     * \param entry The entry
     * \return The Cache file (never NULL if entry is valid)
     */
    const char * gf_cache_get_cache_filename( const DownloadedCacheEntry entry );

    /**
     * Get the real file size of the cache entry
     * \param entry The entry
     * \return the file size
     */
    u32 gf_cache_get_cache_filesize( const DownloadedCacheEntry entry );

    /**
     * Flushes The disk cache for this entry (by persisting the property file
     * \param entry The entry
     */
    GF_Err gf_cache_flush_disk_cache( const DownloadedCacheEntry entry );

    GF_Err gf_cache_set_content_length( const DownloadedCacheEntry entry, u32 length );

    u32 gf_cache_get_content_length( const DownloadedCacheEntry entry);

    /**
     * \brief append cache directives to an HTTP GET request
     * \param entry The entry of cache to use
     * \param httpRequest The HTTP GET request to populate. The request must have been allocated enough to handle the cache arguments
     * \return GF_OK if everything went fine, GF_BAD_PARAM if parameters are wrong
     */
    GF_Err appendHttpCacheHeaders(const DownloadedCacheEntry entry, char * httpRequest);

    /*
     * Cache Management functions
     */

    /*!
     * Delete all cached files in given directory starting with startpattern
     * \param directory to clean up
     * \return GF_OK if everything went fine
     */
    GF_Err gf_cache_delete_all_cached_files(const char * directory);


    /*
     * Cache Reader functions
     */

    GF_CacheReader gf_cache_reader_new(const DownloadedCacheEntry entry);

    GF_Err gf_cache_reader_del( GF_CacheReader handle );

    s64 gf_cache_reader_seek_at( GF_CacheReader reader, u64 seekPosition);

    s64 gf_cache_reader_get_position( const GF_CacheReader reader);

    s64 gf_cache_reader_get_currentSize( GF_CacheReader reader );

    s64 gf_cache_reader_get_full_size( GF_CacheReader reader );

    s32 gf_cache_reader_read( GF_CacheReader reader, char * buff, s32 length);

    Bool gf_cache_check_if_cache_file_is_corrupted(const DownloadedCacheEntry entry);

    void gf_cache_entry_set_delete_files_when_deleted(const DownloadedCacheEntry entry);

    Bool gf_cache_entry_is_delete_files_when_deleted(const DownloadedCacheEntry entry);

    u32 gf_cache_get_sessions_count_for_cache_entry(const DownloadedCacheEntry entry);

	u64 gf_cache_get_start_range( const DownloadedCacheEntry entry );
	u64 gf_cache_get_end_range( const DownloadedCacheEntry entry );

#ifdef __cplusplus
}
#endif

#endif /* _GF_CACHE_H_ */
