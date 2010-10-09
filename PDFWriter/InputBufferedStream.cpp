#include "InputBufferedStream.h"
#include <memory.h>
#include <algorithm>

using namespace std;

InputBufferedStream::InputBufferedStream(void)
{
	Initiate(NULL,DEFAULT_BUFFER_SIZE);
}

InputBufferedStream::~InputBufferedStream(void)
{
	delete[] mBuffer;
	delete mSourceStream;
}


InputBufferedStream::InputBufferedStream(IOBasicTypes::LongBufferSizeType inBufferSize)
{
	Initiate(NULL,inBufferSize);
}

InputBufferedStream::InputBufferedStream(IByteReader* inSourceReader,IOBasicTypes::LongBufferSizeType inBufferSize)
{
	Initiate(inSourceReader,inBufferSize);
}

void InputBufferedStream::Assign(IByteReader* inReader)
{
	mSourceStream = inReader;
	mCurrentBufferIndex = mBuffer;
}

LongBufferSizeType InputBufferedStream::Read(Byte* inBuffer,LongBufferSizeType inBufferSize)
{
	if(mSourceStream)
	{
		LongBufferSizeType bytesRead;

		// if there are enough bytes to read from the buffer..just read from the buffer
		// if content to write fits in the buffer write to buffer
		if(inBufferSize <= (LongBufferSizeType)(mLastAvailableIndex - mCurrentBufferIndex))
		{
			if(inBufferSize > 0)
			{
				memcpy(inBuffer,mCurrentBufferIndex,inBufferSize);
				mCurrentBufferIndex+=inBufferSize;
			}
			bytesRead = inBufferSize;
		}
		else
		{
			// if not, read what's left in the buffer, then read moduluso of buffer size directly to the output buffer
			// then read some leftovers to the buffer, and read from the buffer to the output

			// read what's currently in the buffer into the output buffer
			memcpy(inBuffer,mCurrentBufferIndex,mLastAvailableIndex - mCurrentBufferIndex);
			bytesRead = mLastAvailableIndex - mCurrentBufferIndex;
			mCurrentBufferIndex = mLastAvailableIndex;
			
			// if still need to read more than mBufferSize, read all but modulo of buffer size directly to the output buffer
			// [so that can later read into buffer, and copy from there to the output buffer

			if(mSourceStream->NotEnded())
			{
				inBufferSize -= bytesRead;
				LongBufferSizeType bytesToReadToBuffer = inBufferSize % mBufferSize;

				bytesRead += mSourceStream->Read(inBuffer + bytesRead,inBufferSize-bytesToReadToBuffer);
		
				if(mSourceStream->NotEnded())
				{
					mLastAvailableIndex = mBuffer + mSourceStream->Read(mBuffer,mBufferSize);
					mCurrentBufferIndex = mBuffer;
					bytesToReadToBuffer = min<LongBufferSizeType>(bytesToReadToBuffer,mLastAvailableIndex - mBuffer);
					if(bytesToReadToBuffer > 0)
					{
						memcpy(inBuffer + bytesRead,mCurrentBufferIndex,bytesToReadToBuffer);
						mCurrentBufferIndex+=bytesToReadToBuffer;
						bytesRead+=bytesToReadToBuffer;
					}
				}

			}
		}
		return bytesRead;
	}
	else
		return 0;
}

bool InputBufferedStream::NotEnded()
{
	return mSourceStream->NotEnded() || (mCurrentBufferIndex != mLastAvailableIndex);
}

void InputBufferedStream::Initiate(IByteReader* inSourceReader,IOBasicTypes::LongBufferSizeType inBufferSize)
{
	mBufferSize = inBufferSize;
	mBuffer = new Byte[mBufferSize];
	mLastAvailableIndex = mCurrentBufferIndex = mBuffer;
	mSourceStream = inSourceReader;
}

void InputBufferedStream::Skip(LongBufferSizeType inSkipSize)
{
	if(inSkipSize <= (LongBufferSizeType)(mLastAvailableIndex - mCurrentBufferIndex))
	{
		mCurrentBufferIndex+=inSkipSize;
	}
	else
	{
		inSkipSize -= (LongBufferSizeType)(mLastAvailableIndex - mCurrentBufferIndex);
		mCurrentBufferIndex = mLastAvailableIndex;
		mSourceStream->Skip(inSkipSize);
	}
}

void InputBufferedStream::SetPosition(LongFilePositionType inOffsetFromStart)
{
	mLastAvailableIndex = mCurrentBufferIndex = mBuffer;
	mSourceStream->SetPosition(inOffsetFromStart);
}

IByteReader* InputBufferedStream::GetSourceStream()
{
	return mSourceStream;
}