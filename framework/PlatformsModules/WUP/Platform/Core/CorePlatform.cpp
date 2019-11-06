#include "../../../../Core/Headers/Core.h"

#include "../../../../FileManager/Headers/FilePathManager.h"
#include "Platform/Core/PlatformCore.h"
#include "Platform/Core/PlatformBaseApplication.h"

#include "../../../../Core/Headers/ModuleBase.h"
#include "../../../../Core/Headers/CoreBaseApplication.h"

#include <windows.storage.h>

//#include <FileAPI.h>
#include <Windows.Foundation.h>
#include <wrl\wrappers\corewrappers.h>
#include <wrl\client.h>
#include <stdio.h>

#include <codecvt>
#include <locale>

#include <cstdint>
#include <string>
#include <thread>

//#include <ppltasks.h>
#include <windows.storage.h>
#include <wrl.h>

#include <ppltasks.h>
#include <collection.h>

using namespace concurrency;
using namespace ABI::Windows::Storage;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage::Streams;

void WUPprintf(const char* fmt, ...)
{
	char str[4096];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf_s(str, 4096, fmt, argptr);
	va_end(argptr);
	OutputDebugStringA(str);
}

/*
using namespace ABI::Windows;
using namespace Microsoft::WRL;

HRESULT GetTempFolderPath()
{
	HRESULT hr;
	::ComPtr<Storage::IApplicationDataStatics> applicationDataStatics;

	hr = ABI::Windows::Foundation::GetActivationFactory(Wrappers::HStringReference(RuntimeClass_Windows_Storage_ApplicationData).Get(), &applicationDataStatics);
	if (FAILED(hr))
	{
		return hr;
	}

	ComPtr<Storage::IApplicationData> applicationData;
	hr = applicationDataStatics->get_Current(&applicationData);
	if (FAILED(hr))
	{
		return hr;
	}

	ComPtr<Storage::IStorageFolder> storageFolder;
	hr = applicationData->get_LocalFolder(&storageFolder);
	if (FAILED(hr))
	{
		return hr;
	}

	ComPtr<Storage::IStorageItem> storageItem;
	hr = storageFolder.As(&storageItem);
	if (FAILED(hr))
	{
		return hr;
	}

	HSTRING folderName;
	hr = storageItem->get_Path(&folderName);
	if (FAILED(hr))
	{
		return hr;
	}

	UINT32 length;
	PCWSTR value = WindowsGetStringRawBuffer(folderName, &length);
	//path = value;
	WindowsDeleteString(folderName);
	return S_OK;
}*/

// check if file exist and if it's a file or directory
void Win32CheckState(::FileHandle * hndl)
{
	WIN32_FILE_ATTRIBUTE_DATA InfoFile;
	BOOL ret = ::GetFileAttributesExA(hndl->myFullFileName.c_str(), GetFileExInfoStandard, &InfoFile);
	if (!ret)
		return;

	//int attr = GetFileAttributesA(hndl->myFullFileName.c_str());
	if (InfoFile.dwFileAttributes == -1)
	{
		// file doesn't exist
		hndl->resetStatus();
		return;
	}
	hndl->myStatus |= ::FileHandle::Exist;
	if ((InfoFile.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) != 0)
		hndl->myStatus |= ::FileHandle::IsDIr;
}

bool Win32CreateFolderTree(::FileHandle* hndl)
{
	//TODO
/*	if ((hndl->myStatus&::FileHandle::Exist) == 0)
	{
		SmartPointer<::FileHandle> parent = FilePathManager::Create::FileHandle(FilePathManager::GetParentDirectory(hndl->myFullFileName));
		if (parent->myFullFileName != "")
		{
			Win32CheckState(parent);

			Win32CreateFolderTree(parent);
			return CreateDirectoryA(parent->myFullFileName.c_str(), NULL);
		}
	}*/

	return true;
}


SmartPointer<::FileHandle>	Win32OpenFilePicker()
{
	/*
	auto fop = ref new FileOpenPicker();

	fop->ViewMode = PickerViewMode::List;
	fop->SuggestedStartLocation = PickerLocationId::ComputerFolder;
	fop->FileTypeFilter->Append(".xml");
	
	auto pick_async = fop->PickSingleFileAsync();
	if (!WaitForAsyncOperation(pick_async)) return nullptr;

	auto f = pick_async->GetResults();
	if (!f) return nullptr;

	auto stream_async = f->OpenReadAsync(); 
	if (!WaitForAsyncOperation(stream_async)) return nullptr;
	auto stream = stream_async->GetResults();
	if (!stream) return nullptr;

	auto reader = ref new DataReader(stream);
	if (!WaitForAsyncOperation(reader->LoadAsync(stream->Size))) return nullptr;

	{
		//std::lock_guard<std::mutex> lk{ gPickedFileMutex };
		gPickedFileData.resize(stream->Size);
		reader->ReadBytes(Platform::ArrayReference<u8>(gPickedFileData.data(), stream->Size));
	}
	*/
	return {};
	/*auto p = f->Path;
	usString test = (u16*)p->Data();

	SmartPointer<::FileHandle> hdl = FilePathManager::CreateFileHandle(test.ToString());
	hdl->myFullFileName = test.ToString();
	return hdl;*/
}


SmartPointer<::FileHandle>	Win32FindFullName(const kstl::string&	filename)
{
	
	if (filename[0] != '#')
	{
		return {};
	}
	//TODO
	kstl::string fullFileName = "";

	FilePathManager::DeviceID	id = (FilePathManager::DeviceID)(filename[1]);

	switch (id)
	{
	case FilePathManager::CARD_STORAGE:
		return {};
		break;
	case FilePathManager::DEVICE_STORAGE:
		return {};
		break;
	case FilePathManager::APPLICATION_STORAGE:
	{
		auto local_folder = Windows::Storage::ApplicationData::Current->LocalFolder;
		auto path = local_folder->Path;
		std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
		fullFileName = convert.to_bytes(path->Data());
		fullFileName += "\\";
	}
	case FilePathManager::DB_STORAGE:				// Database is in application storage for us on win32
	{
		/*char resultchar[256];
		if (SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, resultchar) == S_OK)
		{
			fullFileName += resultchar;
			fullFileName += "\\";
		}*/
	}
	break;
	case FilePathManager::DOCUMENT_FOLDER:
	{
		// Determine availability of all files within Pictures library.
		//auto fileList = Windows::Storage::KnownFolders::DocumentsLibrary->GetFileAsync();

	}
	break;
	case FilePathManager::DISTANT_FOLDER1:
	case FilePathManager::DISTANT_FOLDER2:
	case FilePathManager::DISTANT_FOLDER3:
	case FilePathManager::DISTANT_FOLDER4:
		break;

	default:
		if ((id >= FilePathManager::RESERVED1) && (id < FilePathManager::DRIVEA))
		{
			return {};
		}
		else if (id >= FilePathManager::DRIVEA)
		{
			fullFileName += (char)('A' + (int)(id - FilePathManager::DRIVEA));
			fullFileName += "://";
		}
	}
	fullFileName += (const char*)(&(filename.c_str()[3]));

	SmartPointer<::FileHandle> result = FilePathManager::CreateFileHandle((const char*)(&(filename.c_str()[3])));

	result->myDeviceID = id;
	result->myFullFileName = fullFileName;
	result->myVirtualFileAccess = new StorageFileFileAccess(nullptr);
	result->myUseVirtualFileAccess = true;

	Win32CheckState(result.get());
	return result;
}

bool Win32fopen(::FileHandle* handle, const char * mode)
{
	unsigned int flagmode = ::FileHandle::OpeningFlags(mode);

	if (handle->myStatus&::FileHandle::Open) // already opened ? return true
	{
		// check if open mode is the same
		if (flagmode == handle->getOpeningFlags())
		{
			return true;
		}
		else
		{
			fclose(handle->myFile);
		}
	}

	// create parent if in write mode
	if (flagmode & ::FileHandle::Write)
	{
		Win32CreateFolderTree(handle);
	}

	auto err = fopen_s(&handle->myFile, handle->myFullFileName.c_str(), mode);

	if (handle->myFile)
	{
		handle->myStatus |= ::FileHandle::Open;
		handle->myStatus |= ::FileHandle::Exist;
		handle->setOpeningFlags(flagmode);
		return true;
	}
	handle->resetStatus();
	return false;
}

long int Win32fread(void * ptr, long size, long count, ::FileHandle* handle)
{
	return fread(ptr, size, count, handle->myFile);
}

long int Win32fwrite(const void * ptr, long size, long count, ::FileHandle* handle)
{
	return fwrite(ptr, size, count, handle->myFile);
}

long int Win32ftell(::FileHandle* handle)
{
	return ftell(handle->myFile);
}

int Win32fseek(::FileHandle* handle, long int offset, int origin)
{
	return fseek(handle->myFile, offset, origin);
}
int Win32fflush(::FileHandle* handle)
{
	return fflush(handle->myFile);
}
int Win32fclose(::FileHandle* handle)
{
	int result = fclose(handle->myFile);
	handle->resetStatus(); // reset 
	return result;
}

SmartPointer<::FileHandle> Platform_fopen(Windows::Storage::StorageFile^ file, const char * mode)
{
	SmartPointer<::FileHandle> fullfilenamehandle;

	usString str = (u16*)file->Name->Data();

	fullfilenamehandle = FilePathManager::CreateFileHandle(str.ToString());
	fullfilenamehandle->myFullFileName = str.ToString();

	fullfilenamehandle->myUseVirtualFileAccess = true;
	fullfilenamehandle->myVirtualFileAccess = new StorageFileFileAccess(file);

	Platform_fopen(fullfilenamehandle.get(), mode);

	return fullfilenamehandle;
}


template<typename TResult>
bool WaitForAsyncOperation(Windows::Foundation::IAsyncOperation<TResult>^ op)
{
	for (;;)
	{
		
		auto s = op->Status;
		if (s == Windows::Foundation::AsyncStatus::Completed)
			break;

		else if (op->Status != Windows::Foundation::AsyncStatus::Started)
		{
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return true;
}

std::vector<u8> GetDataFromIBuffer(::Windows::Storage::Streams::IBuffer^ buf)
{
	auto reader = ::Windows::Storage::Streams::DataReader::FromBuffer(buf);


	auto len = buf->Length;

	std::vector<u8> data(reader->UnconsumedBufferLength);

	if (!data.empty())
		reader->ReadBytes(
			::Platform::ArrayReference<unsigned char>(
				&data[0], data.size()));

	return data;
}

template<typename TResult,typename progressType>
bool WaitForAsyncOperation(Windows::Foundation::IAsyncOperationWithProgress<TResult, progressType>^ op)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(5));

	for (;;)
	{
		auto s = op->Status;
		if (s == Windows::Foundation::AsyncStatus::Completed)
			break;

		else if (op->Status != Windows::Foundation::AsyncStatus::Started)
		{
			return false;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
	return true;
}

inline bool WaitForAsyncAction(Windows::Foundation::IAsyncAction^ op)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(5));

	for (;;)
	{
		auto s = op->Status;
		if (s == Windows::Foundation::AsyncStatus::Completed)
			break;

		else if (op->Status != Windows::Foundation::AsyncStatus::Started)
		{
			return false;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
	return true;
}


bool		StorageFileFileAccess::Platform_fopen(::FileHandle* handle, const char * mode)
{
	unsigned int flags = ::FileHandle::OpeningFlags(mode);


	if (handle->myDeviceID != FilePathManager::APPLICATION_STORAGE && flags&::FileHandle::Write)
		return false;

	if (!myFile)
	{
		auto local_folder = Windows::Storage::ApplicationData::Current->LocalFolder;

		auto name = handle->myFileName;
		std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
		auto wstr = convert.from_bytes(name.c_str());
		auto name_platform = ref new Platform::String(wstr.data());

		

		auto get_item = local_folder->TryGetItemAsync(name_platform);
		if (!WaitForAsyncOperation(get_item)) return false;

		auto item = get_item->GetResults();
		myFile = safe_cast<Windows::Storage::StorageFile^>(item);

		if (!myFile && flags&::FileHandle::Read)
			return false;

		if (!myFile)
		{
			auto get_file = local_folder->CreateFileAsync(name_platform, mode[0] == 'a' ? Windows::Storage::CreationCollisionOption::OpenIfExists : Windows::Storage::CreationCollisionOption::ReplaceExisting);
			if (!WaitForAsyncOperation(get_file)) return false;
			myFile = get_file->GetResults();
		}
	}
	

	auto properties = myFile->GetBasicPropertiesAsync();

	if (!WaitForAsyncOperation(properties)) return false;

	//auto readAccess = myFile->OpenReadAsync();
	auto openasync = myFile->OpenAsync(flags&::FileHandle::Write ? Windows::Storage::FileAccessMode::ReadWrite : Windows::Storage::FileAccessMode::Read);
	if (!WaitForAsyncOperation(openasync)) return false;
	myFileProperties = properties->GetResults();

	myAccessStream = openasync->GetResults();

	handle->setOpeningFlags(flags);

	return true;
}

void		StorageFileFileAccess::setMainThreadID()
{
	myMainThreadID= std::this_thread::get_id();
}

std::thread::id		StorageFileFileAccess::myMainThreadID;


long int	StorageFileFileAccess::Platform_fread(void * ptr, long size, long count, ::FileHandle* handle)
{
	if (myAccessStream)
	{
		long int readSize = size * count;
		
		if (myDataReader == nullptr)
		{
			myDataReader = ref new DataReader(myAccessStream);
			myDataReader->InputStreamOptions = InputStreamOptions::ReadAhead;
		}

		if (std::this_thread::get_id() == myMainThreadID)
		{
			WaitForAsyncOperation(myDataReader->LoadAsync(readSize));
		}
		else
		{
			create_task(myDataReader->LoadAsync(readSize)).get();
		}

		if (myDataReader->UnconsumedBufferLength < (readSize))
		{
			readSize = myDataReader->UnconsumedBufferLength;
		}

		if (readSize)
			myDataReader->ReadBytes(
				::Platform::ArrayReference<unsigned char>(
				(u8*)ptr, readSize));

		return readSize / size;
		

		
		/*Windows::Storage::Streams::Buffer^ buffer = ref new Windows::Storage::Streams::Buffer(size*count);
		
		auto resultRead=  myAccessStream->ReadAsync(buffer, size*count, InputStreamOptions::None);

		if (!WaitForAsyncOperation(resultRead)) return false;

		Windows::Storage::Streams::IBuffer^ resultbuffer = resultRead->GetResults();

		auto databuffer=GetDataFromIBuffer(resultbuffer);

		memcpy(ptr, databuffer.data(), resultbuffer->Length);

		return resultbuffer->Length;*/
		
	}

	return -1;
}

long int	StorageFileFileAccess::Platform_fwrite(const void * ptr, long size, long count, ::FileHandle* handle)
{
	if (myAccessStream)
	{
		if (mLastWrite)
			WaitForAsyncOperation(mLastWrite);

		DataWriter writer{ };
		writer.WriteBytes(Platform::ArrayReference<u8>{(u8*)ptr, (u32)(size*count)});
		mLastWrite = myAccessStream->WriteAsync(writer.DetachBuffer());
		//if (!WaitForAsyncOperation(flush)) return -1;
		return size * count;
	}
}

long int	StorageFileFileAccess::Platform_ftell(::FileHandle* handle)
{
	if (myAccessStream)
	{
		return myAccessStream->Position;
	}
	return -1L;
}

int			StorageFileFileAccess::Platform_fseek(::FileHandle* handle, long int offset, int origin)
{
	if (myAccessStream)
	{
		long int newpos = myAccessStream->Position;
		switch (origin)
		{
		case SEEK_SET:
			newpos = offset;
			break;
		case SEEK_CUR:
			newpos += offset;
			break;
		case SEEK_END:
			newpos = myFileProperties->Size - offset;
			break;
		}

		if (newpos < 0)
		{
			newpos = 0;
		}
		if (newpos > myFileProperties->Size)
		{
			newpos = myFileProperties->Size;
		}

		myAccessStream->Seek(newpos);

		return 0;
	}
	return -1;
}

int			StorageFileFileAccess::Platform_fflush(::FileHandle* handle)
{
	if (myAccessStream)
	{

		auto result= myAccessStream->FlushAsync();
		if (!WaitForAsyncOperation(result)) return -1;

		return 0;
	}
	return -1;
}

int			StorageFileFileAccess::Platform_fclose(::FileHandle* handle)
{
	if (myAccessStream)
	{
		if (mLastWrite)
			WaitForAsyncOperation(mLastWrite);
		
		mLastWrite = nullptr;
		myAccessStream = nullptr;

		myFileProperties = nullptr;

		myDataReader = nullptr;
		//myFile = nullptr;
		handle->resetStatus();
	}
	return 0;
}

PureVirtualFileAccessDelegate* StorageFileFileAccess::MakeCopy()
{
	auto result = new StorageFileFileAccess(myFile);
	return result;
}

std::string to_utf8(const wchar_t* buffer, int len)
{
	int nChars = ::WideCharToMultiByte(
		CP_UTF8,
		0,
		buffer,
		len,
		NULL,
		0,
		NULL,
		NULL);
	if (nChars == 0) return "";

	std::string newbuffer;
	newbuffer.resize(nChars);
	::WideCharToMultiByte(
		CP_UTF8,
		0,
		buffer,
		len,
		const_cast<char*>(newbuffer.c_str()),
		nChars,
		NULL,
		NULL);

	return newbuffer;
}

std::string to_utf8(const std::wstring& str)
{
	return to_utf8(str.c_str(), (int)str.size());
}

std::wstring to_wchar(const char* buffer, int len)
{
	int nChars = ::MultiByteToWideChar(
		CP_UTF8,
		0,
		buffer,
		len,
		NULL,
		0);

	if (nChars == 0) return L"";

	std::wstring newbuffer;
	newbuffer.resize(nChars);
	::MultiByteToWideChar(
		CP_UTF8,
		0,
		buffer,
		len,
		const_cast<wchar_t*>(newbuffer.c_str()),
		nChars);

	return newbuffer;
}

std::wstring to_wchar(const std::string& str)
{
	return to_wchar(str.c_str(), (int)str.size());
}
