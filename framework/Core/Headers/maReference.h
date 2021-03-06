#ifndef _MAREFERENCE_H
#define _MAREFERENCE_H

#include "CoreModifiableAttribute.h"
#include "AttributeModifier.h"

// ****************************************
// * maReferenceBase class
// * --------------------------------------
/**
* \class	maReferenceBase
* \ingroup CoreModifiableAttibute
* \brief	CoreModifiableAttributeData for reference on CoreModifiable
*/
// ****************************************

struct maReferenceObject
{
	maReferenceObject() = default;

	maReferenceObject(CoreModifiable* lobj)
	{
		obj = lobj;
	}
	maReferenceObject(const kstl::string& nametype) : obj(nullptr)
	{
		search_string = nametype;
	}

	CoreModifiable* obj;
	kstl::string	search_string;
};

template<int notificationLevel>
class maReferenceHeritage : public CoreModifiableAttributeData<maReferenceObject>
{
	DECLARE_ATTRIBUTE_HERITAGE_NO_ASSIGN(maReferenceHeritage, maReferenceHeritage, maReferenceObject, CoreModifiable::ATTRIBUTE_TYPE::REFERENCE);


public:

	maReferenceHeritage(CoreModifiable& owner, bool isInitAttribute, KigsID ID, kstl::string value) : CoreModifiableAttributeData<maReferenceObject>(owner, isInitAttribute, ID)
	{
		_value = maReferenceObject{ value };
		Search();
	}

	maReferenceHeritage() : CoreModifiableAttributeData<maReferenceObject>(KigsID{0u}, maReferenceObject{}) {}

	virtual ~maReferenceHeritage()
	{
		if (_value.obj)
		{
			UnreferenceModifiable(_value.obj);
		}
	}


	// Sets the internal pointer to null, forcing to search again next time we query the reference
	void	ResetFoundModifiable()
	{
		_value.obj = nullptr;
	}
	virtual void CopyData(const CoreModifiableAttributeData<maReferenceObject>& toCopy) override
	{
		const auto& toCopyValue = toCopy.const_ref();
		if (toCopyValue.obj)
		{
			// check if value has changed
			if (_value.obj != toCopyValue.obj)
			{
				if (_value.obj)
					UnreferenceModifiable(_value.obj);

				_value = toCopyValue;

				if (_value.obj)
					ReferenceModifiable(_value.obj);
			}
		}
		else
		{
			if (_value.obj)
				UnreferenceModifiable(_value.obj);

			_value = toCopyValue;
		}
	}

	//! cast to RefCountedClass* operator
	operator CoreModifiable*() { return SearchRef(); }
	//! cast to CoreModifiable* operator
	//operator CoreModifiable*() { return (CoreModifiable*)SearchRef(); }

	template<typename T>
	operator T*(){ return static_cast<T*>(SearchRef()); }

	CoreModifiable*	operator->()
	{
		return SearchRef();
	}

	//! cast to RefCountedClass& operator
	operator CoreModifiable&() { return (*SearchRef()); }

	//! return a reference on internal value
	CoreModifiable& ref() { return (*SearchRef()); }
	//! return a const reference on internal value
	const CoreModifiable& const_ref() { return (*SearchRef()); }


	/// getValue overloads
	virtual bool getValue(kstl::string& value) const override
	{

		((maReferenceHeritage*)this)->SearchRef();
		if (_value.obj)
		{
#ifdef KEEP_NAME_AS_STRING
			value = _value.obj->GetRuntimeType();
#else
			value = std::to_string(_value.obj->GetRuntimeType().toUInt());
#endif

			value += ":";
			value += _value.obj->getName();
			return true;
		}
		else
		{
			value = _value.search_string;
			return true;
		}

		return false;
	}
	virtual bool getValue(usString& value) const override
	{
		// TODO ?
		return false;
	}
	virtual bool getValue(CoreModifiable*&  value) const override
	{
		value = (CoreModifiable*)((maReferenceHeritage*)this)->SearchRef();
		return true;
	}
	virtual bool getValue(void*& value) const override
	{
		value = (void*)((maReferenceHeritage*)this)->SearchRef();
		return true;
	}


	/// setValue overloads
	virtual bool setValue(const char* value) override
	{
		if (this->isReadOnly())
			return false;

		InitAndSearch(value);
		DO_NOTIFICATION(notificationLevel);
		return true;
	}
	virtual bool setValue(const kstl::string& value) override
	{
		if (this->isReadOnly())
			return false;

		InitAndSearch(value);
		DO_NOTIFICATION(notificationLevel);
		return true;
	}
	virtual bool setValue(CoreModifiable* value) override
	{
		if (this->isReadOnly())
			return false;

		if (_value.obj != value)
		{
			if (_value.obj)
				UnreferenceModifiable(_value.obj);

			_value.obj = value;

			if (_value.obj)
				ReferenceModifiable(_value.obj);
		}

		DO_NOTIFICATION(notificationLevel);
		return true;
	}

	/// operators
	auto& operator=(const kstl::string &value)
	{
		InitAndSearch(value);
		DO_NOTIFICATION(notificationLevel);
		return *this;
	}
	auto& operator=(CoreModifiable* value)
	{
		setValue(value);
		return *this;
	}

protected:

	/// Internals;
	void	UnreferenceModifiable(CoreModifiable* current)
	{
		auto& coremodigiablemap = KigsCore::Instance()->getReferenceMap();
		auto found = coremodigiablemap.find(current);

		// ok, the CoreModifiable is here
		if (found != coremodigiablemap.end())
		{
			kstl::vector<CoreModifiableAttribute*>& referencevector = (*found).second;

			kstl::vector<CoreModifiableAttribute*>::iterator	itcurrent = referencevector.begin();
			kstl::vector<CoreModifiableAttribute*>::iterator	itend = referencevector.end();

			while (itcurrent != itend)
			{
				if ((*itcurrent) == this)
				{
					referencevector.erase(itcurrent);
					break;
				}
				++itcurrent;
			}

			if (referencevector.size() == 0)
			{
				current->unflagAsReferenceRegistered();
				coremodigiablemap.erase(found);
			}
		}
	}
	void	ReferenceModifiable(CoreModifiable* current)
	{
		auto& coremodigiablemap = KigsCore::Instance()->getReferenceMap();
		auto found = coremodigiablemap.find(current);

		// ok, the CoreModifiable is already there, add a vector entry
		if (found != coremodigiablemap.end())
		{
			kstl::vector<CoreModifiableAttribute*>& referencevector = (*found).second;
			referencevector.push_back(this);
		}
		else
		{
			// create a new map entry
			kstl::vector<CoreModifiableAttribute*> toAdd;
			toAdd.push_back(this);
			coremodigiablemap[current] = toAdd;

			current->flagAsReferenceRegistered();
		}
	}

	void	Search()
	{
		CMSP obj = nullptr;
		if (!_value.search_string.empty())
		{
			obj = CoreModifiable::SearchInstance(_value.search_string, _owner);
		}
		else
		{
			if (_value.obj)
			{
				UnreferenceModifiable(_value.obj);
				_value.obj = nullptr;
			}
			return;
		}
		if (obj)
		{
			if (_value.obj != obj.get())
			{
				if (_value.obj)
					UnreferenceModifiable(_value.obj);

				_value.obj = obj.get();
				ReferenceModifiable(_value.obj);
			}
		}
		else
		{
			if (_value.obj)
				UnreferenceModifiable(_value.obj);
			_value.obj = nullptr;
		}
	}
	void InitAndSearch(const kstl::string& nametype)
	{
		CoreModifiable* old_obj = _value.obj;
		_value = maReferenceObject{ nametype };
		_value.obj = old_obj;
		Search();
	}
	CoreModifiable*	SearchRef()
	{
		if (_value.obj)
		{
			return _value.obj;
		}
		Search();
		return _value.obj;
	}

};


using maReference = maReferenceHeritage<0>;







#endif //_MAREFERENCE_H