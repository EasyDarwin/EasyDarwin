/*
	Copyleft (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
/*
	File:       HTTPResponseStream.cpp

	Contains:   Impelementation of object in .h
*/

#include "HTTPResponseStream.h"
#include "OSArrayObjectDeleter.h"
#include "OS.h"
#include <errno.h>

QTSS_Error HTTPResponseStream::WriteV(iovec* inVec, UInt32 inNumVectors, UInt32 inTotalLength,
	UInt32* outLengthSent, UInt32 inSendType)
{
	QTSS_Error theErr = QTSS_NoErr;
	UInt32 theLengthSent = 0;
	UInt32 amtInBuffer = this->GetCurrentOffset() - fBytesSentInBuffer;

	if (amtInBuffer > 0)
	{

		// There is some data in the output buffer. Make sure to send that
		// first, using the empty space in the ioVec.

		inVec[0].iov_base = this->GetBufPtr() + fBytesSentInBuffer;
		inVec[0].iov_len = amtInBuffer;
		theErr = fSocket->WriteV(inVec, inNumVectors, &theLengthSent);

		if (fPrintMSG)
		{
			DateBuffer theDate;
			DateTranslator::UpdateDateBuffer(&theDate, 0); // get the current GMT date and time

			qtss_printf("\n#S->C:\n#time: ms=%" _U32BITARG_ " date=%s\n", static_cast<UInt32>(OS::StartTimeMilli_Int()), theDate.GetDateBuffer());
			for (UInt32 i = 0; i < inNumVectors; i++)
			{
				StrPtrLen str(static_cast<char*>(inVec[i].iov_base), static_cast<UInt32>(inVec[i].iov_len));
				str.PrintStrEOL();
			}
		}

		if (theLengthSent >= amtInBuffer)
		{
			// We were able to send all the data in the buffer. Great. Flush it.
			this->Reset();
			fBytesSentInBuffer = 0;

			// Make theLengthSent reflect the amount of data sent in the ioVec
			theLengthSent -= amtInBuffer;
		}
		else
		{
			// Uh oh. Not all the data in the buffer was sent. Update the
			// fBytesSentInBuffer count, and set theLengthSent to 0.

			fBytesSentInBuffer += theLengthSent;
			Assert(fBytesSentInBuffer < this->GetCurrentOffset());
			theLengthSent = 0;
		}
		// theLengthSent now represents how much data in the ioVec was sent
	}
	else if (inNumVectors > 1)
	{
		theErr = fSocket->WriteV(&inVec[1], inNumVectors - 1, &theLengthSent);
	}
	// We are supposed to refresh the timeout if there is a successful write.
	if (theErr == QTSS_NoErr)
		fTimeoutTask->RefreshTimeout();

	// If there was an error, don't alter anything, just bail
	if ((theErr != QTSS_NoErr) && (theErr != EAGAIN))
		return theErr;

	// theLengthSent at this point is the amount of data passed into
	// this function that was sent.
	if (outLengthSent != nullptr)
		*outLengthSent = theLengthSent;

	// Update the StringFormatter fBytesWritten variable... this data
	// wasn't buffered in the output buffer at any time, so if we
	// don't do this, this amount would get lost
	fBytesWritten += theLengthSent;

	// All of the data was sent... whew!
	if (theLengthSent == inTotalLength)
		return QTSS_NoErr;

	// We need to determine now whether to copy the remaining unsent
	// iovec data into the buffer. This is determined based on
	// the inSendType parameter passed in.
	if (inSendType == kDontBuffer)
		return theErr;
	if ((inSendType == kAllOrNothing) && (theLengthSent == 0))
		return EAGAIN;

	// Some or none of the iovec data was sent. Copy the remainder into the output
	// buffer.

	// The caller should consider this data sent.
	if (outLengthSent != nullptr)
		*outLengthSent = inTotalLength;

	UInt32 curVec = 1;
	while (theLengthSent >= inVec[curVec].iov_len)
	{
		// Skip over the vectors that were in fact sent.
		Assert(curVec < inNumVectors);
		theLengthSent -= inVec[curVec].iov_len;
		curVec++;
	}

	while (curVec < inNumVectors)
	{
		// Copy the remaining vectors into the buffer
		this->Put(static_cast<char*>(inVec[curVec].iov_base) + theLengthSent,
			inVec[curVec].iov_len - theLengthSent);
		theLengthSent = 0;
		curVec++;
	}
	return QTSS_NoErr;
}

QTSS_Error HTTPResponseStream::Flush()
{
	UInt32 amtInBuffer = this->GetCurrentOffset() - fBytesSentInBuffer;
	if (amtInBuffer > 0)
	{
		if (fPrintMSG)
		{
			DateBuffer theDate;
			DateTranslator::UpdateDateBuffer(&theDate, 0); // get the current GMT date and time

			qtss_printf("\n#S->C:\n#time: ms=%" _U32BITARG_ " date=%s\n", static_cast<UInt32>(OS::StartTimeMilli_Int()), theDate.GetDateBuffer());
			StrPtrLen str(this->GetBufPtr() + fBytesSentInBuffer, amtInBuffer);
			str.PrintStrEOL();
		}

		UInt32 theLengthSent = 0;
		//(void)fSocket->Send(this->GetBufPtr() + fBytesSentInBuffer, amtInBuffer, &theLengthSent);

		QTSS_Error theErr = fSocket->Send(this->GetBufPtr() + fBytesSentInBuffer, amtInBuffer, &theLengthSent);
		if ((theErr != QTSS_NoErr) && (theErr != EAGAIN))
		{
			return theErr;
		}

		// Refresh the timeout if we were able to send any data
		if (theLengthSent > 0)
			fTimeoutTask->RefreshTimeout();

		if (theLengthSent == amtInBuffer)
		{
			// We were able to send all the data in the buffer. Great. Flush it.
			this->Reset();
			fBytesSentInBuffer = 0;
		}
		else
		{
			// Not all the data was sent, so report an EAGAIN

			fBytesSentInBuffer += theLengthSent;
			Assert(fBytesSentInBuffer < this->GetCurrentOffset());
			return EAGAIN;
		}
	}
	return QTSS_NoErr;
}
