/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005
 *					All rights reserved
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

#ifndef _GF_DOWNLOAD_H_
#define _GF_DOWNLOAD_H_

/*!
 *	\file <gpac/download.h>
 *	\brief Downloader functions.
 */

/*!
 *	\addtogroup dld_grp downloader
 *	\ingroup utils_grp
 *	\brief File Downloader objects
 *
 *	This section documents the file downloading tools the GPAC framework. Currently HTTP is supported, HTTPS is under testing but may not be supported
 *depending on GPAC compilation options (HTTPS in GPAC needs OpenSSL installed on the system).
 *
 *	@{
 */


#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/tools.h>
#include <gpac/module.h>
#include <gpac/cache.h>


    /*!the download manager object. This is usually not used by GPAC modules*/
    typedef struct __gf_download_manager GF_DownloadManager;
    /*!the download manager session.*/
    typedef struct __gf_download_session GF_DownloadSession;

    typedef struct GF_URL_Info_Struct {
        const char * protocol;
        char * server_name;
        char * remotePath;
        char * canonicalRepresentation;
        char * userName;
        char * password;
        u16 port;
    } GF_URL_Info;

    /*!
     * Extracts the information from an URL. A call to gf_dm_url_info_init() must have been issue before calling this method.
     * \param url The URL to fill
     * \param info This structure will be initialized properly and filled with the data
     * \param baseURL The baseURL to use if any (can be null)
     * \return GF_OK if URL is well formed and supported by GPAC
     */
    GF_Err gf_dm_get_url_info(const char * url, GF_URL_Info * info, const char * baseURL);

    /**
     * Init the GF_URL_Info structure before it can be used
     * \param info The structure to initialize
     */
    void gf_dm_url_info_init(GF_URL_Info * info);

    /*!
     * Frees the inner structures of a GF_URL_Info_Struct
     * \param info The info to free
     */
    void gf_dm_url_info_del(GF_URL_Info * info);

    /*!
     *\brief download manager constructor
     *
     *Creates a new download manager object.
     *\param cfg optional configuration file. Currently the download manager needs a configuration file for cache location and
     *other options. The cache directory must be indicated in the section "General", key "CacheDirectory" of the configuration
     *file. If the cache directory is not found, the cache will be disabled but the downloader will still work.
     *\return the download manager object
    */
    GF_DownloadManager *gf_dm_new(GF_Config *cfg);
    /*
     *\brief download manager destructor
     *
     *Deletes the download manager. All running sessions are aborted
     *\param dm the download manager object
     */
    void gf_dm_del(GF_DownloadManager *dm);

    /*!
     *\brief callback function for authentication
     *
     * The gf_dm_get_usr_pass type is the type for the callback of the \ref gf_dm_set_auth_callback function used for password retrieval
     *\param usr_cbk opaque user data
     *\param site_url url of the site the user and password are requested for
     *\param usr_name the user name for this site. The allocated space for this buffer is 50 bytes. \note this varaibale may already be formatted.
     *\param password the password for this site and user. The allocated space for this buffer is 50 bytes.
     *\return 0 if user didn't fill in the information which will result in an authentication failure, 1 otherwise.
    */
    typedef Bool (*gf_dm_get_usr_pass)(void *usr_cbk, const char *site_url, char *usr_name, char *password);

    /*!
     *\brief password retrieval assignment
     *
     *Assigns the callback function used for user password retrieval. If no such function is assigned to the download manager,
     *all downloads requiring authentication will fail.
     *\param dm the download manager object
     *\param get_pass \ref gf_dm_get_usr_pass callback function for user and password retrieval.
     *\param usr_cbk opaque user data passed to callback function
     */
    void gf_dm_set_auth_callback(GF_DownloadManager *dm, gf_dm_get_usr_pass get_pass, void *usr_cbk);

    /*!downloader session message types*/
    enum
    {
        /*!signal that session is setup and waiting for connection request*/
        GF_NETIO_SETUP = 0,
        /*!signal that session connection is done*/
        GF_NETIO_CONNECTED,
        /*!request a protocol method from the user. Default value is "GET" for HTTP*/
        GF_NETIO_GET_METHOD,
        /*!request a header from the user. */
        GF_NETIO_GET_HEADER,
        /*!requesting content from the user, if any. Content is appended to the request*/
        GF_NETIO_GET_CONTENT,
        /*!signal that request is sent and waiting for server reply*/
        GF_NETIO_WAIT_FOR_REPLY,
        /*!signal a header to user. */
        GF_NETIO_PARSE_HEADER,
        /*!signal request reply to user. The reply is always sent after the headers*/
        GF_NETIO_PARSE_REPLY,
        /*!send data to the user*/
        GF_NETIO_DATA_EXCHANGE,
        /*!all data has been transfered*/
        GF_NETIO_DATA_TRANSFERED,
        /*!signal that the session has been deconnected*/
        GF_NETIO_DISCONNECTED,
        /*!downloader session failed (error code set) or done/destroyed (no error code)*/
        GF_NETIO_STATE_ERROR
    };

    /*!session download flags*/
    enum
    {
        /*!session is not threaded, the user must explicitely fetch the data */
        GF_NETIO_SESSION_NOT_THREADED	=	1,
        /*! session data is live, e.g. data will be sent to the user if threaded mode (live streams like radios & co)
				Whether the data is cached or not to disk cannot be controlled by the user at the current time.
		*/
        GF_NETIO_SESSION_NOT_CACHED	=	1<<1,
		/*indicates that the connection to the server should be kept once the download is successfully completed*/
        GF_NETIO_SESSION_PERSISTENT =	1<<2,
    };


    /*!protocol I/O parameter*/
    typedef struct
    {
        /*!parameter message type*/
        u32 msg_type;
        /*error code if any. Valid for all message types.*/
        GF_Err error;
        /*!data received or data to send. Only valid for GF_NETIO_GET_CONTENT and GF_NETIO_DATA_EXCHANGE (when no cache is setup) messages*/
        const char *data;
        /*!size of associated data. Only valid for GF_NETIO_GET_CONTENT and GF_NETIO_DATA_EXCHANGE messages*/
        u32 size;
        /*protocol header. Only valid for GF_NETIO_GET_HEADER, GF_NETIO_PARSE_HEADER and GF_NETIO_GET_METHOD*/
        const char *name;
        /*protocol header value or server response. Only alid for GF_NETIO_GET_HEADER, GF_NETIO_PARSE_HEADER and GF_NETIO_PARSE_REPLY*/
        char *value;
        /*response code - only valid for GF_NETIO_PARSE_REPLY*/
        u32 reply;
    } GF_NETIO_Parameter;

    /*!
     *\brief callback function for data reception and state signaling
     *
     * The gf_dm_user_io type is the type for the data callback function of a download session
     *\param usr_cbk opaque user data
     *\param parameter the input/output parameter structure
    */
    typedef void (*gf_dm_user_io)(void *usr_cbk, GF_NETIO_Parameter *parameter);



    /*!
     *\brief download session constructor
     *
     *Creates a new download session
     *\param dm the download manager object
     *\param url file to retrieve (no PUT/POST yet, only downloading is supported)
     *\param dl_flags combination of session download flags
     *\param user_io \ref gf_dm_user_io callback function for data reception and service messages
     *\param usr_cbk opaque user data passed to callback function
     *\param error error for failure cases
     *\return the session object or NULL if error. If no error is indicated and a NULL session is returned, this means the file is local
     */
    GF_DownloadSession * gf_dm_sess_new(GF_DownloadManager *dm, const char *url, u32 dl_flags,
                                        gf_dm_user_io user_io,
                                        void *usr_cbk,
                                        GF_Err *error);

    /*!
     *\brief download session simple constructor
     *
     *Creates a new download session
     *\param url file to retrieve (no PUT/POST yet, only downloading is supported)
     *\param dl_flags combination of session download flags
     *\param user_io \ref gf_dm_user_io callback function for data reception and service messages
     *\param usr_cbk opaque user data passed to callback function
     *\param cache_name cache name
     *\param error error for failure cases
     *\return the session object or NULL if error. If no error is indicated and a NULL session is returned, this means the file is local
     */
    GF_DownloadSession *gf_dm_sess_new_simple(GF_DownloadManager * dm, const char *url, u32 dl_flags,
            gf_dm_user_io user_io,
            void *usr_cbk,
            GF_Err *e);

    /*!
     *brief downloader session destructor
     *
     *Deletes the download session, cleaning the cache if indicated in the configuration file of the download manager (section "Downloader", key "CleanCache")
     *\param sess the download session
    */
    void gf_dm_sess_del(GF_DownloadSession * sess);
    /*!
     *\brief aborts downloading
     *
     *Aborts all operations in the session, regardless of its state. The session cannot be reused once this is called.
     *\param sess the download session
     */
    void gf_dm_sess_abort(GF_DownloadSession * sess);
    /*!
     *\brief sets private data
     *
     *associate private data with the session.
     *\param sess the download session
     *\param private_data the private data
     *\warning the private_data parameter is reserved for bandwidth statistics per service when used in the GPAC terminal.
     */
    void gf_dm_sess_set_private(GF_DownloadSession * sess, void *private_data);

    /*!
     *\brief gets private data
     *
     *Gets private data associated with the session.
     *\param sess the download session
     *\return the private data
     *\warning the private_data parameter is reserved for bandwidth statistics per service when used in the GPAC terminal.
     */
    void *gf_dm_sess_get_private(GF_DownloadSession * sess);

	/*!
     *\brief gets last session error
     *
     *Gets the last error that occured in the session
     *\param sess the download session
     *\return the last error
     */
    GF_Err gf_dm_sess_last_error(GF_DownloadSession *sess);

    /*!
     *\brief is download manager thread dead?
     *
     *Indicates whether the thread has ended
	 *\param sess the download session
     */
	Bool gf_dm_is_thread_dead(GF_DownloadSession *sess);

    /*!
     *\brief fetches data on session
     *
     *Fetches data from the server. This will also performs connections and all needed exchange with server.
     *\param sess the download session
     *\param buffer destination buffer
     *\param buffer_size destination buffer allocated size
     *\param read_size amount of data actually fetched
     *\note this can only be used when the session is not threaded
     */
    GF_Err gf_dm_sess_fetch_data(GF_DownloadSession * sess, char *buffer, u32 buffer_size, u32 *read_size);

    /*!
     *\brief get mime type
     *
     *Fetches the mime type of the URL this session is fetching
     *\param sess the download session
     *\return the mime type of the URL, or NULL if error. You should get the error with \ref gf_dm_sess_last_error
     */
    const char *gf_dm_sess_mime_type(GF_DownloadSession * sess);

    /*!
     *\brief sets session range
     *
     *Sets the session byte range. This shll be called before processing the session.
     *\param sess the download session
     *\param start_range HTTP download start range in byte 
     *\param end_range HTTP download end range in byte 
     *\note this can only be used when the session is not threaded
     */
	GF_Err gf_dm_sess_set_range(GF_DownloadSession *sess, u64 start_range, u64 end_range);
    /*!
     *\brief get cache file name
     *
     * Gets the cache file name for the session.
     *\param sess the download session
     *\return the absolute path of the cache file, or NULL if the session is not cached*/
    const char *gf_dm_sess_get_cache_name(GF_DownloadSession * sess);

    /*!
     * \brief Marks the cache file to be deleted once the file is not used anymore by any session
     * \param entry The cache entry to delete
     */
    void gf_dm_delete_cached_file_entry(const GF_DownloadManager * dm, const char * url);

    /*!
     * Convenience function
     * \see gf_dm_delete_cached_file_entry
     */
    void gf_dm_delete_cached_file_entry_session(const GF_DownloadSession * dm, const char * url);

    /*!
     * Get a range of a cache entry file
     * \param entry The session
     * \param startOffset The first byte of the request to get
     * \param endOffset The last byte of request to get
     * \return The temporary name for the file created to have a range of the file
     */
    const char * gf_cache_get_cache_filename_range( const GF_DownloadSession * sess, u64 startOffset, u64 endOffset );

    /*!
     *\brief get statistics
     *
     *Gets download statistics for the session. All output parameters are optional and may be set to NULL.
     *\param sess the download session
     *\param server the remote server address
     *\param path the path on the remote server
     *\param total_size the total size in bytes the file fetched, 0 if unknown.
     *\param bytes_done the amount of bytes received from the server
     *\param bytes_per_sec the average data rate in bytes per seconds
     *\param net_status the session status
     */
    GF_Err gf_dm_sess_get_stats(GF_DownloadSession * sess, const char **server, const char **path, u32 *total_size, u32 *bytes_done, u32 *bytes_per_sec, u32 *net_status);


    /*!
     *\brief fetch session object
     *
     *Fetch the session object (process all headers and data transfer). This is only usable if the session is not threaded
     *\param sess the download session
     *\return the last error in the session or 0 if none*/
    GF_Err gf_dm_sess_process(GF_DownloadSession * sess);

    /*!
     *\brief fetch session object headers
     *
     *Fetch the session object headers and stops after that. This is only usable if the session is not threaded
     *\param sess the download session
     *\return the last error in the session or 0 if none*/
    GF_Err gf_dm_sess_process_headers(GF_DownloadSession * sess);

    /*!
     *\brief fetch session status
     *
     *Fetch the session current status
     *\param sess the download session
     *\return the session status*/
	u32 gf_dm_sess_get_status(GF_DownloadSession * sess);
	/*!
     *\brief Get session resource url
     *
     *Returns the original resource URL associated with the session
     *\param sess the download session
     *\return the associated URL
     */
    const char *gf_dm_sess_get_resource_name(GF_DownloadSession *dnload);
    /*!
     *\brief Get session original resource url
     *
     *Returns the original resource URL before any redirection associated with the session
     *\param sess the download session
     *\return the associated URL
     */
    const char *gf_dm_sess_get_original_resource_name(GF_DownloadSession *dnload);
	

    /*!
     * \brief Download a file over the network using a download manager
     * \param dm The downlaod manager to use, function will use all associated cache ressources
     * \param url The url to download
     * \param filename The filename to download
     * \return GF_OK if everything went fine, an error otherwise
     */
    GF_Err gf_dm_wget_with_cache(GF_DownloadManager * dm,
                                const char *url, const char *filename);

    /*!
     * \brief Same as gf_dm_wget_with_cache, but initializes the GF_DownloadManager by itself.
     * This function is deprecated, please use gf_dm_wget_with_cache instead
     * \param url The url to download
     * \param filename The filename to download
     * \return GF_OK if everything went fine, an error otherwise
     */
    GF_Err gf_dm_wget(const char *url, const char *filename);

    /*!
     *\brief Reset session
     *
     *Resets the session for new processing of the same url
     *\param sess the download session
     *\return error code if any
     */
    GF_Err gf_dm_sess_reset(GF_DownloadSession *sess);

    /*!
     * \brief forces the refresh of a cache entry
     * The entry is still allocated in the session.
     * \param sess The session
     * \return a pointer to the entry of session refreshed
     */
    DownloadedCacheEntry gf_dm_refresh_cache_entry(GF_DownloadSession *sess);
    
    /*!
     * Tells whether session can be cached on disk.
     * Typically, when request has no content length, it deserves being streamed an cannot be cached
     * (ICY or MPEG-streamed content
     * \param sess The session
     * \return True if a cache can be created
     */
    Bool gf_dm_sess_can_be_cached_on_disk(const GF_DownloadSession *sess);


    /*!
     * Reassigns session flags and callbacks. This is only possible if the session is not threaded.
	 * \param sess The session
	 * \param flags The new flags for the session 
	 * \param user_io The new callback function
	 * \param cbk The new user data to ba used in the callback function
     * \return GF_OK or error
     */
	GF_Err gf_dm_sess_reassign(GF_DownloadSession *sess, u32 flags, gf_dm_user_io user_io, void *cbk);

    /*!
     * Re-setup an existing, completed session to download a new URL. If same server/port/protocol is used, the same socket will be reused if the session
	 has the @GF_NETIO_SESSION_PERSISTENT flag set. This is only possible if the session is not threaded.
	 * \param sess The session
	 * \param url The new url for the session 
     * \return GF_OK or error
     */
	GF_Err gf_dm_sess_setup_from_url(GF_DownloadSession *sess, const char *url);

    /*
     *\brief sets download manager max rate per session
     *
     *Sets the maximum rate (per session only at the current time). 
     *\param dm the download manager object
     *\param rate_in_byte_per_sec the new rate in bytes per sec. If 0, HTTP rate will not be limited
     */
    void gf_dm_set_data_rate(GF_DownloadManager *dm, u32 rate_in_byte_per_sec);

    /*
     *\brief gets download manager max rate per session
     *
     *Sets the maximum rate (per session only at the current time). 
     *\param dm the download manager object
     *\return the rate in bytes per sec. If 0, HTTP rate is not limited
     */
    u32 gf_dm_get_data_rate(GF_DownloadManager *dm);

	/*! @} */

#ifdef __cplusplus
}
#endif


#endif		/*_GF_DOWNLOAD_H_*/

