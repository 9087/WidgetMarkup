// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "Templates/Casts.h"
#include "Templates/SharedPointer.h"

#include <type_traits>
#include <utility>

class FProperty;

/** Lightweight type descriptor for FElementNode hierarchy. Used for IsA/Cast without RTTI. */
struct FElementNodeClass
{
	const FElementNodeClass* SuperClass;

	explicit FElementNodeClass(const FElementNodeClass* InSuperClass)
		: SuperClass(InSuperClass)
	{
	}

	bool IsChildOf(const FElementNodeClass* Other) const
	{
		for (const FElementNodeClass* Current = this; Current; Current = Current->SuperClass)
		{
			if (Current == Other)
			{
				return true;
			}
		}
		return false;
	}
};

class FElementNode : public TSharedFromThis<FElementNode>
{
	friend class FElementNodeFactory;
	friend class FElementTreeBuilder;
	friend class FPropertyRun;

public:
	/** Returns this instance's type descriptor for IsA/Cast. */
	virtual const FElementNodeClass* GetClass() const = 0;

	static const FElementNodeClass* StaticClass();

	template <typename T>
	bool IsA() const
	{
		static_assert(std::is_base_of_v<FElementNode, T>, "T must be FElementNode or a derived class");
		return GetClass()->IsChildOf(T::StaticClass());
	}

	virtual UObject* GetObject() const { return nullptr; }
	virtual UStruct* GetPropertyOwnerStruct() const { return nullptr; }

	/**
	 * Returns the property a child element should be interpreted as, when the
	 * parent cannot provide a real property chain (e.g. Variable container
	 * elements). nullptr means the child must resolve its own type.
	 */
	virtual FProperty* ResolveExpectedChildProperty() { return nullptr; }

	enum class EMessageType
	{
		Log,
		Error,
	};

	class FMessage
	{
	public:
		FMessage(const FText& InText, EMessageType InType);
		const FText& GetText() const;
		EMessageType GetType() const { return Type; }

	private:
		FText Text;
		EMessageType Type;
	};

	class FResult
	{
	public:
		FResult& Log(const FText& MessageText);
		FResult& Error(const FText& MessageText);

		static FResult Success();
		static FResult Failure();

		operator bool() const;
		bool operator!() const;

		FResult& PrintOnFailure();

		/** Returns the text of the first error message, or an empty text. */
		FText GetFirstErrorText() const;

	private:
		FResult() = default;

		bool bOK = false;
		TArray<TSharedRef<FMessage>> Messages;
	};

	class FContext
	{
	public:
		class FMetaData
		{
		public:
			template<typename TMetaDataType>
			bool IsOfType() const
			{
				return TypeId == GetTypeId<TMetaDataType>();
			}

			virtual ~FMetaData() = default;

		protected:
			template<typename TMetaDataType>
			explicit FMetaData(TMetaDataType*)
				: TypeId(GetTypeId<TMetaDataType>())
			{
			}

		private:
			template<typename TMetaDataType>
			static const void* GetTypeId()
			{
				static int32 TypeKey = 0;
				return &TypeKey;
			}

			const void* TypeId = nullptr;
		};

		template<typename TMetaDataType>
		class TMetaData : public FMetaData
		{
		public:
			TMetaData()
				: FMetaData(static_cast<TMetaDataType*>(nullptr))
			{
			}
		};

		void Push(const TSharedRef<FElementNode>& ElementNode);
		TSharedRef<FElementNode> Pop();
		bool IsEmpty() const;

		TSharedPtr<FElementNode> GetLastNode() const;

		/** Returns the first node (from top of stack) whose GetObject() is non-null. */
		TSharedPtr<FElementNode> GetLastObjectNode() const;

		const auto& GetNodes() const { return Nodes; }
		auto& GetNodes() { return Nodes; }

		UObject* FindObject(UClass* Class) const;

		template <typename T>
		T* FindObject() const
		{
			return Cast<T>(FindObject(T::StaticClass()));
		}

		template <typename TMetaData>
		TSharedPtr<TMetaData> GetMetaData() const
		{
			static_assert(std::is_base_of_v<FMetaData, TMetaData>, "TMetaData must derive from FContext::FMetaData");
			for (const auto& MetaDataEntry : MetaData)
			{
				if (MetaDataEntry->IsOfType<TMetaData>())
				{
					return StaticCastSharedRef<TMetaData>(MetaDataEntry).ToSharedPtr();
				}
			}
			return TSharedPtr<TMetaData>();
		}

		template <typename TMetaDataType>
		bool HasMetaData() const
		{
			static_assert(std::is_base_of_v<FMetaData, TMetaDataType>, "TMetaDataType must derive from FContext::FMetaData");
			for (const auto& MetaDataEntry : MetaData)
			{
				if (MetaDataEntry->IsOfType<TMetaDataType>())
				{
					return true;
				}
			}
			return false;
		}

		template <typename TMetaDataType, typename... TArgs>
		TSharedRef<TMetaDataType> AddMetaData(TArgs&&... Args)
		{
			static_assert(std::is_base_of_v<FMetaData, TMetaDataType>, "TMetaDataType must derive from FContext::FMetaData");
			TSharedRef<TMetaDataType> NewMetaData = MakeShared<TMetaDataType>(std::forward<TArgs>(Args)...);
			MetaData.Add(NewMetaData);
			return NewMetaData;
		}

		template <typename TMetaDataType, typename... TArgs>
		TSharedRef<TMetaDataType> GetOrAddMetaData(TArgs&&... Args)
		{
			static_assert(std::is_base_of_v<FMetaData, TMetaDataType>, "TMetaDataType must derive from FContext::FMetaData");
			if (TSharedPtr<TMetaDataType> ExistingMetaData = GetMetaData<TMetaDataType>())
			{
				return ExistingMetaData.ToSharedRef();
			}
			return AddMetaData<TMetaDataType>(std::forward<TArgs>(Args)...);
		}

		template <typename TMetaDataType>
		void RemoveMetaData()
		{
			static_assert(std::is_base_of_v<FMetaData, TMetaDataType>, "TMetaDataType must derive from FContext::FMetaData");
			for (int32 Index = MetaData.Num() - 1; Index >= 0; --Index)
			{
				const auto& MetaDataEntry = MetaData[Index];
				if (MetaDataEntry->IsOfType<TMetaDataType>())
				{
					MetaData.RemoveAtSwap(Index);
				}
			}
		}

	private:

		TArray<TSharedRef<FElementNode>> Nodes;
		TArray<TSharedRef<FMetaData>> MetaData;
	};

public:
	FResult Begin(const FContext& Context, UObject* Outer, UStruct* Struct);
	FResult End();

protected:

	FElementNode();
	virtual ~FElementNode();

	virtual FResult OnBegin(const FContext& Context, UObject* Outer, UStruct* Struct) = 0;
	virtual FResult OnEnd() = 0;
	virtual FResult OnAddChild(const TSharedRef<FElementNode>& Child) = 0;
	virtual bool HasProperty(const FStringView& AttributeName) = 0;

	/** Receives the text content between open and close tags (ElementData from FastXml). */
	virtual void SetElementData(const TCHAR* InElementData) {}
};

/** Injects StaticClass() and GetClass() for safe Cast/IsA. */
#define DECLARE_ELEMENT_NODE(TClass, TSuperClass) \
public: \
	using Super = TSuperClass; \
	static const FElementNodeClass* StaticClass(); \
	virtual const FElementNodeClass* GetClass() const override { return StaticClass(); }

/** Implements StaticClass() for a derived ElementNode. Place in the .cpp for TClass. */
#define IMPLEMENT_ELEMENT_NODE(TClass, TSuperClass) \
	const FElementNodeClass* TClass::StaticClass() \
	{ \
		static const FElementNodeClass Instance(TSuperClass::StaticClass()); \
		return &Instance; \
	}

template <typename T>
inline T* CastElementNode(FElementNode* Node)
{
	static_assert(std::is_base_of_v<FElementNode, T>, "T must be FElementNode or a derived class");
	return Node && Node->IsA<T>() ? static_cast<T*>(Node) : nullptr;
}

template <typename T>
inline const T* CastElementNode(const FElementNode* Node)
{
	static_assert(std::is_base_of_v<FElementNode, T>, "T must be FElementNode or a derived class");
	return Node && Node->IsA<T>() ? static_cast<const T*>(Node) : nullptr;
}

template <typename T>
inline TSharedPtr<T> CastElementNode(const TSharedPtr<FElementNode>& Node)
{
	static_assert(std::is_base_of_v<FElementNode, T>, "T must be FElementNode or a derived class");
	if (T* Ptr = CastElementNode<T>(Node.Get()))
	{
		return TSharedPtr<T>(Node, Ptr);
	}
	return TSharedPtr<T>();
}
