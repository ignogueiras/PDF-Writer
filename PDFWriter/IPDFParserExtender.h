#pragma once

#include <string>

class IByteReader;
class PDFName;
class PDFDictionary;

using namespace std;

class IPDFParserExtender
{
public:

	virtual ~IPDFParserExtender(){}

	// for extending fliter support in stream read
	virtual IByteReader* CreateFilterForStream(IByteReader* inStream,PDFName* inFilterName,PDFDictionary* inDecodeParams) = 0;

	// for decryption extension

	// return input stream if did nothing, or a filter stream owning the input stream if encryption is required
	virtual IByteReader* CreateDecryptionFilterForStream(IByteReader* inStream) = 0;
	// decrypt input string. this serves both literal and hexadecimal strings
	virtual string DecryptString(string inStringToDecrypt) = 0;
	// event for indirect object parsing start
	virtual void OnObjectStart(long long inObjectID, long long inGenerationNumber) = 0;
	// event for indirect object parsing end
	virtual void OnObjectEnd(PDFObject* inObject) = 0;
};