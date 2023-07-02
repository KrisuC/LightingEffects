#pragma once

#include <windows.h>
#include <stdexcept>
#include <wrl.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <type_traits>

#if defined(_DEBUG) || defined(DBG) || defined(DEBUG)
	#define BUILD_DEBUG 1
#else
	#define BUILD_DEBUG 0
#endif

using Microsoft::WRL::ComPtr;

// ComPtr specific
#define SAFE_RELEASE(p) if (p) (p)->Release();

inline std::wstring HResultToString(HRESULT hr)
{
	wchar_t str[128]{};
	swprintf_s(str, L"HRESULT of 0x%08x", static_cast<UINT>(hr));
	return std::wstring(str);
}

class HResultException : public std::runtime_error
{
public:
	HResultException(HRESULT hr, const wchar_t* fileName, const wchar_t* lineNum)
		: std::runtime_error("D3D12 Runtime Error")
		, m_hr(hr)
		, m_message(HResultToString(hr) + L", file: " + fileName + L", line: " + lineNum)
	{
	}

	HRESULT Error() const { return m_hr; }
private:
	const HRESULT m_hr;
	std::wstring m_message;
};

// 2 indirection, to avoid creating "__LINE__"
#define __to_string_helper(x)			#x
#define __to_string(x)					__to_string_helper(x)

#define __convert_to_wstring_helper(x)	L##x
#define __convert_to_wstring(x)			__convert_to_wstring_helper(x)

#define __WSTR_FILE__		__convert_to_wstring(__FILE__)
#define __WSTR_LINE__		__convert_to_wstring(__to_string(__LINE__))


inline void ThrowIfFailedImpl(HRESULT hr, const wchar_t* fileName, const wchar_t* lineNum)
{
	if (FAILED(hr))
	{
		throw HResultException(hr, fileName, lineNum);
	}
}
#define ThrowIfFailed(hr) ThrowIfFailedImpl(hr, __WSTR_FILE__, __WSTR_LINE__)

#define check(x) if (x) { throw std::runtime_error("file: " __FILE__ ", " __to_string(__LINE__) ": " #x); } 

#if BUILD_DEBUG
inline void SetName(ID3D12Object* pObject, LPCWSTR name)
{
	pObject->SetName(name);
}
inline void SetNameIndexed(ID3D12Object* pObject, LPCWSTR name, UINT index)
{
	WCHAR fullName[50];
	if (swprintf_s(fullName, L"%s[%u]", name, index) > 0)
	{
		pObject->SetName(fullName);
	}
}
#else
inline void SetName(ID3D12Object* pObject, LPCWSTR name)
{
}
inline void SetNameIndexed(ID3D12Object* pObject, LPCWSTR name, UINT index)
{
}
#endif

#define NAME_D3D12_OBJECT(x) SetName((x).Get(), L#x)
#define NAME_D3D12_OBJECT_INDEXED(x, i) SetName((x)[n].Get(), L#x, i)

#ifdef D3D_COMPILE_STANDARD_FILE_INCLUDE
inline ComPtr<ID3DBlob> CompileShader(
	const std::wstring& fileName,
	const D3D_SHADER_MACRO* defines,
	const std::string& entryPoint,
	const std::string& target)
{
	UINT compileFlags = 0;
#if BUILD_DEBUG
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> byteCode;
	ComPtr<ID3DBlob> errors;

	HRESULT hr = D3DCompileFromFile(
		fileName.c_str(), 
		defines, 
		D3D_COMPILE_STANDARD_FILE_INCLUDE, 
		entryPoint.c_str(), 
		target.c_str(),
		compileFlags,
		0,
		&byteCode,
		&errors);

	if (errors != nullptr)
	{
		OutputDebugStringA((char*)errors->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	return byteCode;
}
#endif

template <class T>
void ResetPtrArray(T* comPtrArray)
{
	static_assert(std::is_array_v(comPtrArray));
	for (auto& i : *comPtrArray)
	{
		i.Release();
	}
}
