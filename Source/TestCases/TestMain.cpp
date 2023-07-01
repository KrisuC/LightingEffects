#include "RefCountPtr.h"
#include <iostream>

class StringResource : public IRefCountResource
{
public:
	StringResource(char const* pStr, size_t length)
	{
		m_pStr = new char[length];
		strcpy_s(m_pStr, length, pStr);
	}
	char* m_pStr;

	virtual void FreeResource() override
	{
		delete [] m_pStr;
	}
};

void PrintRefStringResource(RefCountPtr<StringResource> const& RefPtr)
{
	if (RefPtr.IsValid())
	{
		printf_s("RefCount %d, data: %s\n", RefPtr.GetRefCount(), RefPtr->m_pStr);
	}
	else
	{
		printf_s("RefCount %d, data: nullptr\n", RefPtr.GetRefCount());
	}
}

void TestRefCountPtr()
{
	RefCountPtr<StringResource> refStringResource = RefCountPtr<StringResource>::Make("Fuck the world!", 20);
	PrintRefStringResource(refStringResource);

	RefCountPtr<StringResource> refString2 = refStringResource;
	PrintRefStringResource(refStringResource);
	PrintRefStringResource(refString2);

	RefCountPtr<StringResource> refString3 = std::move(refString2);
	PrintRefStringResource(refStringResource);
	PrintRefStringResource(refString2);
	PrintRefStringResource(refString3);
}

int main()
{
	TestRefCountPtr();
}