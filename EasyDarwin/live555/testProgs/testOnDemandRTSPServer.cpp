/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2015, Live Networks, Inc.  All rights reserved
// A test program that demonstrates how to stream - via unicast RTP
// - various kinds of file on demand, using a built-in RTSP server.
// main program

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

UsageEnvironment* env;

// To make the second and subsequent client for each stream reuse the same
// input stream as the first client (rather than playing the file from the
// start for each client), change the following "False" to "True":
Boolean reuseFirstSource = False;

// To stream *only* MPEG-1 or 2 video "I" frames
// (e.g., to reduce network bandwidth),
// change the following "False" to "True":
Boolean iFramesOnly = False;

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
			   char const* streamName, char const* inputFileName); // fwd

static char newDemuxWatchVariable;

static MatroskaFileServerDemux* matroskaDemux;
static void onMatroskaDemuxCreation(MatroskaFileServerDemux* newDemux, void* /*clientData*/) {
  matroskaDemux = newDemux;
  newDemuxWatchVariable = 1;
}

static OggFileServerDemux* oggDemux;
static void onOggDemuxCreation(OggFileServerDemux* newDemux, void* /*clientData*/) {
  oggDemux = newDemux;
  newDemuxWatchVariable = 1;
}

int main(int argc, char** argv) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  UserAuthenticationDatabase* authDB = NULL;
#ifdef ACCESS_CONTROL
  // To implement client access control to the RTSP server, do the following:
  authDB = new UserAuthenticationDatabase;
  authDB->addUserRecord("username1", "password1"); // replace these with real strings
  // Repeat the above with each <username>, <password> that you wish to allow
  // access to the server.
#endif

  // Create the RTSP server:
  RTSPServer* rtspServer = RTSPServer::createNew(*env, 8554, authDB);
  if (rtspServer == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }

  char const* descriptionString
    = "Session streamed by \"testOnDemandRTSPServer\"";

  // Set up each of the possible streams that can be served by the
  // RTSP server.  Each such stream is implemented using a
  // "ServerMediaSession" object, plus one or more
  // "ServerMediaSubsession" objects for each audio/video substream.

  // A MPEG-4 video elementary stream:
  {
    char const* streamName = "mpeg4ESVideoTest";
    char const* inputFileName = "test.m4e";
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);
    sms->addSubsession(MPEG4VideoFileServerMediaSubsession
		       ::createNew(*env, inputFileName, reuseFirstSource));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // A H.264 video elementary stream:
  {
    char const* streamName = "h264ESVideoTest";
    char const* inputFileName = "test.264";
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);
    sms->addSubsession(H264VideoFileServerMediaSubsession
		       ::createNew(*env, inputFileName, reuseFirstSource));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // A H.265 video elementary stream:
  {
    char const* streamName = "h265ESVideoTest";
    char const* inputFileName = "test.265";
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);
    sms->addSubsession(H265VideoFileServerMediaSubsession
		       ::createNew(*env, inputFileName, reuseFirstSource));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // A MPEG-1 or 2 audio+video program stream:
  {
    char const* streamName = "mpeg1or2AudioVideoTest";
    char const* inputFileName = "test.mpg";
    // NOTE: This *must* be a Program Stream; not an Elementary Stream
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);
    MPEG1or2FileServerDemux* demux
      = MPEG1or2FileServerDemux::createNew(*env, inputFileName, reuseFirstSource);
    sms->addSubsession(demux->newVideoServerMediaSubsession(iFramesOnly));
    sms->addSubsession(demux->newAudioServerMediaSubsession());
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // A MPEG-1 or 2 video elementary stream:
  {
    char const* streamName = "mpeg1or2ESVideoTest";
    char const* inputFileName = "testv.mpg";
    // NOTE: This *must* be a Video Elementary Stream; not a Program Stream
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);
    sms->addSubsession(MPEG1or2VideoFileServerMediaSubsession
	       ::createNew(*env, inputFileName, reuseFirstSource, iFramesOnly));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // A MP3 audio stream (actually, any MPEG-1 or 2 audio file will work):
  // To stream using 'ADUs' rather than raw MP3 frames, uncomment the following:
//#define STREAM_USING_ADUS 1
  // To also reorder ADUs before streaming, uncomment the following:
//#define INTERLEAVE_ADUS 1
  // (For more information about ADUs and interleaving,
  //  see <http://www.live555.com/rtp-mp3/>)
  {
    char const* streamName = "mp3AudioTest";
    char const* inputFileName = "test.mp3";
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);
    Boolean useADUs = False;
    Interleaving* interleaving = NULL;
#ifdef STREAM_USING_ADUS
    useADUs = True;
#ifdef INTERLEAVE_ADUS
    unsigned char interleaveCycle[] = {0,2,1,3}; // or choose your own...
    unsigned const interleaveCycleSize
      = (sizeof interleaveCycle)/(sizeof (unsigned char));
    interleaving = new Interleaving(interleaveCycleSize, interleaveCycle);
#endif
#endif
    sms->addSubsession(MP3AudioFileServerMediaSubsession
		       ::createNew(*env, inputFileName, reuseFirstSource,
				   useADUs, interleaving));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // A WAV audio stream:
  {
    char const* streamName = "wavAudioTest";
    char const* inputFileName = "test.wav";
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);
    // To convert 16-bit PCM data to 8-bit u-law, prior to streaming,
    // change the following to True:
    Boolean convertToULaw = False;
    sms->addSubsession(WAVAudioFileServerMediaSubsession
	       ::createNew(*env, inputFileName, reuseFirstSource, convertToULaw));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // An AMR audio stream:
  {
    char const* streamName = "amrAudioTest";
    char const* inputFileName = "test.amr";
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);
    sms->addSubsession(AMRAudioFileServerMediaSubsession
		       ::createNew(*env, inputFileName, reuseFirstSource));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // A 'VOB' file (e.g., from an unencrypted DVD):
  {
    char const* streamName = "vobTest";
    char const* inputFileName = "test.vob";
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);
    // Note: VOB files are MPEG-2 Program Stream files, but using AC-3 audio
    MPEG1or2FileServerDemux* demux
      = MPEG1or2FileServerDemux::createNew(*env, inputFileName, reuseFirstSource);
    sms->addSubsession(demux->newVideoServerMediaSubsession(iFramesOnly));
    sms->addSubsession(demux->newAC3AudioServerMediaSubsession());
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // A MPEG-2 Transport Stream:
  {
    char const* streamName = "mpeg2TransportStreamTest";
    char const* inputFileName = "test.ts";
    char const* indexFileName = "test.tsx";
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);
    sms->addSubsession(MPEG2TransportFileServerMediaSubsession
		       ::createNew(*env, inputFileName, indexFileName, reuseFirstSource));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // An AAC audio stream (ADTS-format file):
  {
    char const* streamName = "aacAudioTest";
    char const* inputFileName = "test.aac";
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);
    sms->addSubsession(ADTSAudioFileServerMediaSubsession
		       ::createNew(*env, inputFileName, reuseFirstSource));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // A DV video stream:
  {
    // First, make sure that the RTPSinks' buffers will be large enough to handle the huge size of DV frames (as big as 288000).
    OutPacketBuffer::maxSize = 300000;

    char const* streamName = "dvVideoTest";
    char const* inputFileName = "test.dv";
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);
    sms->addSubsession(DVVideoFileServerMediaSubsession
		       ::createNew(*env, inputFileName, reuseFirstSource));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // A AC3 video elementary stream:
  {
    char const* streamName = "ac3AudioTest";
    char const* inputFileName = "test.ac3";
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);

    sms->addSubsession(AC3AudioFileServerMediaSubsession
		       ::createNew(*env, inputFileName, reuseFirstSource));

    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // A Matroska ('.mkv') file, with video+audio+subtitle streams:
  {
    char const* streamName = "matroskaFileTest";
    char const* inputFileName = "test.mkv";
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);

    newDemuxWatchVariable = 0;
    MatroskaFileServerDemux::createNew(*env, inputFileName, onMatroskaDemuxCreation, NULL);
    env->taskScheduler().doEventLoop(&newDemuxWatchVariable);

    Boolean sessionHasTracks = False;
    ServerMediaSubsession* smss;
    while ((smss = matroskaDemux->newServerMediaSubsession()) != NULL) {
      sms->addSubsession(smss);
      sessionHasTracks = True;
    }
    if (sessionHasTracks) {
      rtspServer->addServerMediaSession(sms);
    }
    // otherwise, because the stream has no tracks, we don't add a ServerMediaSession to the server.

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // A WebM ('.webm') file, with video(VP8)+audio(Vorbis) streams:
  // (Note: ".webm' files are special types of Matroska files, so we use the same code as the Matroska ('.mkv') file code above.)
  {
    char const* streamName = "webmFileTest";
    char const* inputFileName = "test.webm";
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);

    newDemuxWatchVariable = 0;
    MatroskaFileServerDemux::createNew(*env, inputFileName, onMatroskaDemuxCreation, NULL);
    env->taskScheduler().doEventLoop(&newDemuxWatchVariable);

    Boolean sessionHasTracks = False;
    ServerMediaSubsession* smss;
    while ((smss = matroskaDemux->newServerMediaSubsession()) != NULL) {
      sms->addSubsession(smss);
      sessionHasTracks = True;
    }
    if (sessionHasTracks) {
      rtspServer->addServerMediaSession(sms);
    }
    // otherwise, because the stream has no tracks, we don't add a ServerMediaSession to the server.

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // An Ogg ('.ogg') file, with video and/or audio streams:
  {
    char const* streamName = "oggFileTest";
    char const* inputFileName = "test.ogg";
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);

    newDemuxWatchVariable = 0;
    OggFileServerDemux::createNew(*env, inputFileName, onOggDemuxCreation, NULL);
    env->taskScheduler().doEventLoop(&newDemuxWatchVariable);

    Boolean sessionHasTracks = False;
    ServerMediaSubsession* smss;
    while ((smss = oggDemux->newServerMediaSubsession()) != NULL) {
      sms->addSubsession(smss);
      sessionHasTracks = True;
    }
    if (sessionHasTracks) {
      rtspServer->addServerMediaSession(sms);
    }
    // otherwise, because the stream has no tracks, we don't add a ServerMediaSession to the server.

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // An Opus ('.opus') audio file:
  // (Note: ".opus' files are special types of Ogg files, so we use the same code as the Ogg ('.ogg') file code above.)
  {
    char const* streamName = "opusFileTest";
    char const* inputFileName = "test.opus";
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);

    newDemuxWatchVariable = 0;
    OggFileServerDemux::createNew(*env, inputFileName, onOggDemuxCreation, NULL);
    env->taskScheduler().doEventLoop(&newDemuxWatchVariable);

    Boolean sessionHasTracks = False;
    ServerMediaSubsession* smss;
    while ((smss = oggDemux->newServerMediaSubsession()) != NULL) {
      sms->addSubsession(smss);
      sessionHasTracks = True;
    }
    if (sessionHasTracks) {
      rtspServer->addServerMediaSession(sms);
    }
    // otherwise, because the stream has no tracks, we don't add a ServerMediaSession to the server.

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // A MPEG-2 Transport Stream, coming from a live UDP (raw-UDP or RTP/UDP) source:
  {
    char const* streamName = "mpeg2TransportStreamFromUDPSourceTest";
    char const* inputAddressStr = "239.255.42.42";
        // This causes the server to take its input from the stream sent by the "testMPEG2TransportStreamer" demo application.
        // (Note: If the input UDP source is unicast rather than multicast, then change this to NULL.)
    portNumBits const inputPortNum = 1234;
        // This causes the server to take its input from the stream sent by the "testMPEG2TransportStreamer" demo application.
    Boolean const inputStreamIsRawUDP = False; 
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);
    sms->addSubsession(MPEG2TransportUDPServerMediaSubsession
		       ::createNew(*env, inputAddressStr, inputPortNum, inputStreamIsRawUDP));
    rtspServer->addServerMediaSession(sms);

    char* url = rtspServer->rtspURL(sms);
    *env << "\n\"" << streamName << "\" stream, from a UDP Transport Stream input source \n\t(";
    if (inputAddressStr != NULL) {
      *env << "IP multicast address " << inputAddressStr << ",";
    } else {
      *env << "unicast;";
    }
    *env << " port " << inputPortNum << ")\n";
    *env << "Play this stream using the URL \"" << url << "\"\n";
    delete[] url;
  }

  // Also, attempt to create a HTTP server for RTSP-over-HTTP tunneling.
  // Try first with the default HTTP port (80), and then with the alternative HTTP
  // port numbers (8000 and 8080).

  if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080)) {
    *env << "\n(We use port " << rtspServer->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling.)\n";
  } else {
    *env << "\n(RTSP-over-HTTP tunneling is not available.)\n";
  }

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
			   char const* streamName, char const* inputFileName) {
  char* url = rtspServer->rtspURL(sms);
  UsageEnvironment& env = rtspServer->envir();
  env << "\n\"" << streamName << "\" stream, from the file \""
      << inputFileName << "\"\n";
  env << "Play this stream using the URL \"" << url << "\"\n";
  delete[] url;
}
