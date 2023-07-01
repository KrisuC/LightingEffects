#include "D3D12Utility.h"

/** Ref-counted resource interface */
class IRefCountResource
{
public:
	IRefCountResource()
		: m_refCount(0u)
	{
	}

protected:
	virtual void FreeResource() = 0;

private:
	void AddRef()
	{
		m_refCount++;
	}

	void Release()
	{
		m_refCount--;
		if (m_refCount == 0)
		{
			FreeResource();
		}
	}

	uint32_t GetRefCount()
	{
		return m_refCount;
	}

private:
	uint32_t m_refCount;

	template <typename T> friend class RefCountPtr;
};


/** Ref-counted pointer */
template <typename T>
class RefCountPtr
{
	static_assert(std::is_base_of_v<IRefCountResource, T>);

public:
	template <typename... ArgsType>
	static RefCountPtr Make(ArgsType&&... args)
	{
		RefCountPtr<T> ptr;
		ptr.m_pRefCountResource = new T(std::forward<ArgsType>(args)...);
		ptr->AddRef();
		return ptr;
	}

public:
	RefCountPtr()
		: m_pRefCountResource(nullptr)
	{
	}

	/** Copy/Move constructor */
	RefCountPtr(RefCountPtr const& rhs)
		: m_pRefCountResource(rhs.m_pRefCountResource)
	{
		m_pRefCountResource->AddRef();
	}

	RefCountPtr(RefCountPtr&& rhs)
	{
		// RefCount is plus 1 and minus 1, so no change
		m_pRefCountResource = rhs.m_pRefCountResource;
		rhs.m_pRefCountResource = nullptr;
	}

	~RefCountPtr()
	{
		SafeRelease();
	}

	/** Copy/Move Assignment */
	RefCountPtr operator=(RefCountPtr const& rhs)
	{
		SafeRelease();
		m_pRefCountResource = rhs.m_pRefCountResource;
		m_pRefCountResource->AddRef();
	}

	RefCountPtr operator=(RefCountPtr&& rhs)
	{
		SafeRelease();
		m_pRefCountResource = rhs.m_pRefCountResource;
		rhs.m_pRefCountResource = nullptr;
	}

	/** Get reference and Dereference interface */
	T* operator->() const
	{
		return m_pRefCountResource;
	}

	T* operator->()
	{
		return m_pRefCountResource;
	}

	T const& operator*() const
	{
		return *m_pRefCountResource;
	}

	T& operator*()
	{
		return *m_pRefCountResource;
	}

	uint32_t GetRefCount() const
	{
		if (m_pRefCountResource)
		{
			return m_pRefCountResource->GetRefCount();
		}
		return 0u;
	}

	void SafeRelease()
	{
		if (m_pRefCountResource)
		{
			m_pRefCountResource->Release();
		}
	}

	bool IsValid() const
	{
		return m_pRefCountResource != nullptr;
	}

private:
	T* m_pRefCountResource;
};
