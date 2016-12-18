/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005
 *					All rights reserved
 *
 *  This file is part of GPAC / modules interfaces
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


/*

		Note on video driver: this is not a graphics driver, the only thing requested from this driver
	is accessing video memory and performing stretch of YUV and RGB on the backbuffer (bitmap node)
	the graphics driver is a different entity that performs 2D rasterization

*/

#ifndef _GF_MODULE_AUDIO_OUT_H_
#define _GF_MODULE_AUDIO_OUT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*include event system*/
#include <gpac/module.h>


/*
	Audio hardware output module
*/

/*interface name and version for audio output*/
#define GF_AUDIO_OUTPUT_INTERFACE		GF_4CC('G','A','O', '1')

/*interface returned on query interface*/
typedef struct _audiooutput
{
	/* interface declaration*/
	GF_DECL_MODULE_INTERFACE

	/*setup system 
		Win32: os_handle is HWND

	if num_buffer is set, the audio driver should work with num_buffers with a total amount of audio data
	equal to total_duration ms
	if not set the driver is free to decide what to do
	*/
	GF_Err (*Setup) (struct _audiooutput *aout, void *os_handle, u32 num_buffers, u32 total_duration);

	/*shutdown system */
	void (*Shutdown) (struct _audiooutput *aout);

	/*query output frequency available - if the requested sampleRate is not available, the driver shall return the best 
	possible sampleRate able to handle NbChannels and NbBitsPerSample - if it doesn't handle the NbChannels
	the internal mixer will do it
	*/
	GF_Err (*QueryOutputSampleRate)(struct _audiooutput *aout, u32 *io_desired_samplerate, u32 *io_NbChannels, u32 *io_nbBitsPerSample);

	/*set output config - if audio is not running, driver must start it
	*SampleRate, *NbChannels, *nbBitsPerSample: 
		input: desired value
		output: final values
	channel_cfg is the channels output cfg, eg set of flags as specified in constants.h
	*/
	GF_Err (*ConfigureOutput) (struct _audiooutput *aout, u32 *SampleRate, u32 *NbChannels, u32 *nbBitsPerSample, u32 channel_cfg);

	/*returns total buffer size used in ms. This is needed to compute the min size of audio decoders output*/
	u32 (*GetTotalBufferTime)(struct _audiooutput *aout);

	/*returns audio delay in ms, eg time delay until written audio data is outputed by the sound card
	This function is only called after ConfigureOuput*/ 
	u32 (*GetAudioDelay)(struct _audiooutput *aout);

	/*set output volume(between 0 and 100) */
	void (*SetVolume) (struct _audiooutput *aout, u32 Volume);
	/*set balance (between 0 and 100, 0=full left, 100=full right)*/
	void (*SetPan) (struct _audiooutput *aout, u32 pan);
	/*freezes soundcard flow - must not be NULL for self threaded
		PlayType: 0: pause, 1: resume, 2: reset HW buffer and play.
	*/
	void (*Play) (struct _audiooutput *aout, u32 PlayType);
	/*specifies whether the driver relies on the app to feed data or runs standalone*/
	Bool SelfThreaded;

	/*if not using private thread, this should perform sleep & fill of HW buffer
		the audio render loop in this case is: while (run) {driver->WriteAudio(); if (reconf) Reconfig();}
	the driver must therefore give back the hand to the renderer as often as possible - the usual way is:
		gf_sleep untill hw data can be written
		write HW data
		return
	*/
	void (*WriteAudio)(struct _audiooutput *aout);

	/*if using private thread the following MUST be provided*/
	void (*SetPriority)(struct _audiooutput *aout, u32 priority);

	/*your private data handler - should be allocated when creating the interface object*/
	void *opaque;
	
	/*these are assigned by the audio renderer once module is loaded*/
	
	/*fills the buffer with audio data, returns effective bytes written - the rest is filled with 0*/
	u32 (*FillBuffer) (void *audio_renderer, char *buffer, u32 buffer_size);
	void *audio_renderer;

} GF_AudioOutput;


/*
	Audio hardware output module
*/

/*interface name and version for audio output*/
#define GF_AUDIO_FILTER_INTERFACE		GF_4CC('G','A','F', '1')

/*interface returned on query interface*/
typedef struct _tag_audio_filter GF_AudioFilter;

struct _tag_audio_filter 
{
	/* interface declaration*/
	GF_DECL_MODULE_INTERFACE

	/*sets the current filter. The filterstring is opaque to libgpac and is taken as given 
	in the GPAC configuration file, where filters are listed as a ';;' seperated list in the "Filter" key of 
	the [Audio] section.
		@returns: 1 is this module can handle the filterstring, 0 otherwise.
	*/
	Bool (*SetFilter)(GF_AudioFilter *af, char *filterstring);
	/*configures the filter:
		@samplerate: samplerate of data - this cannot be modified by a filter
		@bits_per_sample: sample format (8 or 16 bit signed PCM data) of data - this cannot be modified by a filter
		@input_channel_number: number of input channels 
		@input_channel_layout: channel layout of input data - cf <gpac/constants.h>
		@output_channel_number: number of ouput channels 
		@output_channel_layout: channel layout of output data - cf <gpac/constants.h>
		&output_block_size_in_samples: size in blocks of the data to be sent to this filter. 
				If 0, data will not be reframed and blocks of any number of samples will be processed
		@delay_ms: delay in ms introduced by this filter
		@inplace_processing_capable: if set to 1, this filter is capable of processing data inplace, in which case
			the same buffer is passed for in_data and out_data in the process call
	*/
	GF_Err (*Configure)(GF_AudioFilter *af, u32 samplerate, u32 bits_per_sample, u32 input_channel_number, u32 input_channel_layout, u32 *output_channel_number, u32 *output_channel_layout, u32 *output_block_size_in_samples, u32 *delay_ms, Bool *inplace_processing_capable);
	/*process a chunk of audio data. 
		@in_data: input sample buffer
		@in_data_size: input sample buffer size. If block len was set in the configure stage, there will be block len sample
		@out_data: output sample buffer - if inplace was set in the configure stage, same as in_data. 
				NOTE: Outputing more samples that input ones may crash the system, the buffer only contains space for 
			the same amount of samples (including channels added/removed by the filter)
		@out_data_size: data size written to output. Usually 0 or in_data_size. 
	*/
	GF_Err (*Process)(GF_AudioFilter *af, void *in_data, u32 in_data_size, void *out_data, u32 *out_data_size);
	
	/*gets an option from the filter - currently not implemented */
	const char *(*GetOption)(GF_AudioFilter *af, char *option);
	/*sets an option to the filter - currently not implemented */
	Bool (*SetOption)(GF_AudioFilter *af, char *option, char *value);
	
	/*Indicates the filter should be reset (audio stop or seek )*/
	void (*Reset)(GF_AudioFilter *af);

	/*private user data for the module*/
	void *udta;
};

#ifdef __cplusplus
}
#endif


#endif	/*_GF_MODULE_AUDIO_OUT_H_*/

