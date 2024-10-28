#pragma once
#include "ObjectSerializer_base.h"

#include <unordered_map>
#include <vector>
#include <functional>
#include <typeindex>
#include <fstream>

namespace ObjectSerializer
{
    class ISerializable;
	class ISerializableID;
    class OBJECT_SERIALIZER_EXPORT Serializer
    {
        struct VTableMetaData
        {
            enum Location
            {
                Beginning,
                End
            };
            std::size_t size = sizeof(void**);
			Location location = Location::Beginning;
            bool serializeVtable = false;
        };
        struct ObjectMetaData
        {
            std::string name;
            std::size_t typeHash;
            std::size_t size;
			std::function<ISerializable*()> create;

			ObjectMetaData(const std::string& name, 
                           const std::size_t typeHash, 
                           const std::size_t size, 
                           const std::function<ISerializable* ()>& create) 
                : name(name)
                , typeHash(typeHash)
                , size(size)
                , create(create)
            {}
            ObjectMetaData(const ObjectMetaData& other)
				: name(other.name)
				, typeHash(other.typeHash)
				, size(other.size)
				, create(other.create)
            {}
            ObjectMetaData(ObjectMetaData&& other) noexcept
                : name(std::move(other.name))
                , typeHash(std::move(other.typeHash))
                , size(std::move(other.size))
                , create(std::move(other.create))
            {}

        };
        public:
        static Log::LogObject& getLogger();
        static VTableMetaData& getVTableMetaData();
        static void saveVtable(bool doSave)
        {
            getVTableMetaData().serializeVtable = doSave;
        }
		static void setVtableSize(std::size_t size)
		{
			getVTableMetaData().size = size;
		}
		static void setVtableLocation(VTableMetaData::Location location)
		{
			getVTableMetaData().location = location;
		}

        Serializer();
        ~Serializer();

        template <typename T>
        static void registerType() {
			size_t hashCode = typeid(T).hash_code();
            if (isTypeRegistered(hashCode))
            {
				getLogger().logWarning("Type: " + std::string(typeid(T).name()) + " already registered");
                return;
            }

			ObjectMetaData meta(typeid(T).name(),
								hashCode,
								sizeof(T),
                                []() { return new T(); });
			getObjectMetaData().insert({ typeid(T).hash_code(), std::move(meta) });
        }

        template <typename T>
        bool addObject(T* obj)
        {
            // Static assert if T is not derived from ISerializable
            static_assert(std::is_base_of<ISerializable, T>::value, "T must be derived from ISerializable");
            if (!obj)
            {
				getLogger().logError("Object is nullptr");
				return false;
            }
            if (!isTypeRegistered(typeid(*obj).hash_code()))
            {
                typeNotRegistered(obj);
                return false;
            }

            m_objs.push_back(obj);
			return true;
        }

		const std::vector<ISerializable*>& getObjects() const
		{
			return m_objs;
		}
        void clear()
        {
			m_objs.clear();
        }

        bool saveToFile(const std::string& filename) const;
		bool loadFromFile(const std::string& filename);

        static bool saveToFile(const std::string& filename, const std::vector<ISerializable*>& objs);
        static bool loadFromFile(const std::string& filename, std::vector<ISerializable*>& objs);
		
        static bool overrideInFile(const std::string& filename, const ISerializableID* obj);
        static bool loadFromFile(const std::string& filename, std::size_t objectID, ISerializableID*& obj);

        private:
		static bool isTypeRegistered(const std::size_t typeHash);

		static void typeWithHashNotRegistered(const std::size_t typeHash);
		static void typeNotRegistered(const ISerializable* obj);

        static bool setCursorToID(std::fstream& file, std::size_t id);
		
        std::vector<ISerializable*> m_objs;

		static std::unordered_map<std::size_t, ObjectMetaData>& getObjectMetaData();
    };
}