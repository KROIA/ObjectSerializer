#include "Serializer.h"

#include "ISerializable.h"
#include "ISerializableID.h"

namespace ObjectSerializer
{
#if LOGGER_LIBRARY_AVAILABLE == 1
	Log::LogObject& Serializer::getLogger()
	{
		static Log::LogObject logger("ObjectSerializer::Serializer");
		return logger;
	}
#endif
	Serializer::Serializer()
	{

	}
	Serializer::~Serializer()
	{

	}

	bool Serializer::saveToFile(const std::string& filename) const
	{
		return saveToFile(filename, m_objs);
	}
	bool Serializer::loadFromFile(const std::string& filename)
	{
		return loadFromFile(filename, m_objs);
	}

	bool Serializer::saveToFile(const std::string& filename, const std::vector<ISerializable*>& objs)
	{
#if defined(OBJECT_SERIALIZER_DEBUG) && LOGGER_LIBRARY_AVAILABLE == 1
		Log::LogObject& logger = getLogger();
#endif
		const auto& mataMap = getObjectMetaData();
		const VTableMetaData& vTableMetaData = getVTableMetaData();
		std::ofstream outFile(filename, std::ios::binary);
		if (!outFile.is_open())
		{
#if LOGGER_LIBRARY_AVAILABLE == 1
			getLogger().logError("Failed to open file: " + filename);
#endif
			return false;
		}
		for (const auto& obj : objs)
		{
			std::size_t typeHash = std::type_index(typeid(*obj)).hash_code();
			const auto& it = mataMap.find(typeHash);
			if (it != mataMap.end())
			{
				const ObjectMetaData& meta = it->second;

				outFile.write(reinterpret_cast<const char*>(&typeHash), sizeof(typeHash));
				size_t byteCount = meta.size;
				const char* startData = reinterpret_cast<const char*>(obj);

				if (!vTableMetaData.serializeVtable)
				{
					if (meta.size < vTableMetaData.size)
					{
#if LOGGER_LIBRARY_AVAILABLE == 1
						getLogger().logError("Object size is less than vtable size. Type: " + meta.name);
#endif
						continue;
					}
					byteCount -= vTableMetaData.size;
					if (vTableMetaData.location == VTableMetaData::Location::Beginning)
					{
						startData += vTableMetaData.size;
					}
				}
#if defined(OBJECT_SERIALIZER_DEBUG) && LOGGER_LIBRARY_AVAILABLE == 1
				logger.logInfo("Serializing object of type: " + meta.name + " [" + std::to_string(byteCount) + " bytes]");
#endif
				outFile.write(startData, byteCount);
			}
			else
			{
				typeNotRegistered(obj);
			}
		}
		outFile.close();
		return true;
	}
	bool Serializer::loadFromFile(const std::string& filename, std::vector<ISerializable*>& objs)
	{
#if defined(OBJECT_SERIALIZER_DEBUG) && LOGGER_LIBRARY_AVAILABLE == 1
		Log::LogObject& logger = getLogger();
#endif
		std::ifstream inFile(filename, std::ios::binary);
		if (!inFile.is_open())
		{
#if LOGGER_LIBRARY_AVAILABLE == 1
			getLogger().logError("Failed to open file: " + filename);
#endif
			return false;
		}
		objs.clear();
		const VTableMetaData& vTableMetaData = getVTableMetaData();
		const auto& mataMap = getObjectMetaData();

		while (inFile)
		{
			std::size_t typeHash;
			inFile.read(reinterpret_cast<char*>(&typeHash), sizeof(typeHash));
			if (!inFile)
			{
//#if LOGGER_LIBRARY_AVAILABLE == 1
//				getLogger().logError("Failed to read objects");
//#endif
				return true;//return false; // EOF or read error
			}

			const auto& it = mataMap.find(typeHash);
			if (it != mataMap.end()) {
				const ObjectMetaData& meta = it->second;
				// Call the factory function to load the object
				ISerializable* obj = meta.create();

				char* startData = reinterpret_cast<char*>(obj);
				size_t byteCount = meta.size;
				if (!vTableMetaData.serializeVtable)
				{
					byteCount -= vTableMetaData.size;
					if (vTableMetaData.location == VTableMetaData::Location::Beginning)
					{
						startData += vTableMetaData.size;
					}
				}
#if defined(OBJECT_SERIALIZER_DEBUG) && LOGGER_LIBRARY_AVAILABLE == 1
				logger.logInfo("Deserializing object of type: " + meta.name + " [" + std::to_string(byteCount) + " bytes]");
#endif
				inFile.read(startData, byteCount);
				objs.push_back(obj);
			}
			else
			{
				typeWithHashNotRegistered(typeHash);
				// Assume maximum size struct and skip it (adjust if known max size is different)
				inFile.ignore(1024);
			}
		}
		return true;
	}

	
	bool Serializer::overrideInFile(const std::string& filename, const ISerializableID* obj)
	{
		std::fstream file(filename, std::ios::binary | std::ios::in | std::ios::out);
		if (!file.is_open())
		{
#if LOGGER_LIBRARY_AVAILABLE == 1
			getLogger().logError("Failed to open file: " + filename);
#endif
			return false;
		}
		if (setCursorToID(file, obj->getID()))
		{
			const VTableMetaData& vTableMetaData = getVTableMetaData();
			const auto& mataMap = getObjectMetaData();
			std::size_t typeHash = std::type_index(typeid(*obj)).hash_code();
			const auto& it = mataMap.find(typeHash);
			if (it != mataMap.end())
			{
				const ObjectMetaData& meta = it->second;

				file.write(reinterpret_cast<const char*>(&typeHash), sizeof(typeHash));
				size_t byteCount = meta.size;
				const char* startData = reinterpret_cast<const char*>(obj);

				if (!vTableMetaData.serializeVtable)
				{
					if (meta.size < vTableMetaData.size)
					{
#if LOGGER_LIBRARY_AVAILABLE == 1
						getLogger().logError("Object size is less than vtable size. Type: " + meta.name);
#endif
						return false;
					}
					byteCount -= vTableMetaData.size;
					if (vTableMetaData.location == VTableMetaData::Location::Beginning)
					{
						startData += vTableMetaData.size;
					}
				}
#if defined(OBJECT_SERIALIZER_DEBUG) && LOGGER_LIBRARY_AVAILABLE == 1
				getLogger().logInfo("Serializing object of type: " + meta.name + " [" + std::to_string(byteCount) + " bytes]");
#endif
				file.write(startData, byteCount);
				file.close();
				return true;
			}
			else
			{
				typeNotRegistered(obj);
			}
		}
		return false;
	}
	bool Serializer::loadFromFile(const std::string& filename, std::size_t objectID, ISerializableID*& obj)
	{
		std::fstream inFile(filename, std::ios::binary | std::ios::in);
		if (!inFile.is_open())
		{
#if LOGGER_LIBRARY_AVAILABLE == 1
			getLogger().logError("Failed to open file: " + filename);
#endif
			return false;
		}
		obj = nullptr;
		if (setCursorToID(inFile, objectID))
		{
			const VTableMetaData& vTableMetaData = getVTableMetaData();
			const auto& mataMap = getObjectMetaData();

			std::size_t typeHash;
			inFile.read(reinterpret_cast<char*>(&typeHash), sizeof(typeHash));
			if (!inFile)
			{
				return false; 
			}

			const auto& it = mataMap.find(typeHash);

			if (it != mataMap.end())
			{
				const ObjectMetaData& meta = it->second;
				// Call the factory function to load the object
				obj = dynamic_cast<ISerializableID*>(meta.create());

				char* startData = reinterpret_cast<char*>(obj);
				size_t byteCount = meta.size;
				if (!vTableMetaData.serializeVtable)
				{
					byteCount -= vTableMetaData.size;
					if (vTableMetaData.location == VTableMetaData::Location::Beginning)
					{
						startData += vTableMetaData.size;
					}
				}
#if defined(OBJECT_SERIALIZER_DEBUG) && LOGGER_LIBRARY_AVAILABLE == 1
				getLogger().logInfo("Deserializing object of type: " + meta.name + " [" + std::to_string(byteCount) + " bytes]");
#endif
				inFile.read(startData, byteCount);
				inFile.close();
				return true;
			}
		}


		inFile.close();
		return false;
		/*
#ifdef OBJECT_SERIALIZER_DEBUG
		Log::LogObject& logger = getLogger();
#endif
		std::ifstream inFile(filename, std::ios::binary);
		if (!inFile.is_open())
		{
			getLogger().logError("Failed to open file: " + filename);
			return false;
		}
		obj = nullptr;
		const VTableMetaData& vTableMetaData = getVTableMetaData();
		const auto& mataMap = getObjectMetaData();

		struct LoaderData
		{
			ISerializableID* instance = nullptr;
			Serializer::ObjectMetaData metaData;
			LoaderData(ISerializableID* instance, const Serializer::ObjectMetaData& metaData)
				: instance(instance)
				, metaData(metaData)
			{}
			LoaderData(LoaderData&& other) noexcept
				: instance(other.instance)
				, metaData(std::move(other.metaData))
			{
				other.instance = nullptr;
			}
			~LoaderData()
			{
				delete instance;
			}
		};
		
		std::unordered_map<std::size_t, LoaderData> objectMap;
		for (const auto& it : mataMap)
		{
			ISerializable* obj = it.second.create();
			ISerializableID* id = dynamic_cast<ISerializableID*>(obj);
			if (id)
			{
				objectMap.insert({ it.second.typeHash, LoaderData(id, it.second) });
			}
			else
				delete obj;
		}

		while (inFile)
		{
			std::size_t typeHash;
			inFile.read(reinterpret_cast<char*>(&typeHash), sizeof(typeHash));
			if (!inFile)
			{
				getLogger().logError("Failed to read objects");
				return false; // EOF or read error
			}

			const auto& it = mataMap.find(typeHash);
			
			if (it != mataMap.end()) {
				const auto& loaderIt = objectMap.find(typeHash);
				const ObjectMetaData& meta = it->second;
				// Call the factory function to load the object
				
				if (loaderIt != objectMap.end())
				{
					char* startData = reinterpret_cast<char*>(loaderIt->second.instance);
					size_t byteCount = meta.size;
					if (!vTableMetaData.serializeVtable)
					{
						byteCount -= vTableMetaData.size;
						if (vTableMetaData.location == VTableMetaData::Location::Beginning)
						{
							startData += vTableMetaData.size;
						}
					}
#ifdef OBJECT_SERIALIZER_DEBUG
					logger.logInfo("Deserializing object of type: " + meta.name + " [" + std::to_string(byteCount) + " bytes]");
#endif
					inFile.read(startData, byteCount);

					if (loaderIt->second.instance->getID() == objectID)
					{
						obj = loaderIt->second.instance;
						loaderIt->second.instance = nullptr;
						return true;
					}
				}
				else
				{
					// Skip the amount of bytes for this object
					size_t skipSize = meta.size;
					skipSize -= vTableMetaData.size * size_t(!vTableMetaData.serializeVtable);
					inFile.ignore(skipSize);
				}
			}
			else
			{
				typeWithHashNotRegistered(typeHash);
				// Assume maximum size struct and skip it (adjust if known max size is different)
				inFile.ignore(1024);
			}
		}
		return false;*/
	}

	bool Serializer::isTypeRegistered(const std::size_t typeHash)
	{
		auto& objectMetaData = getObjectMetaData();
		return objectMetaData.find(typeHash) != objectMetaData.end();
	}

	void Serializer::typeWithHashNotRegistered(const std::size_t typeHash)
	{
#if LOGGER_LIBRARY_AVAILABLE == 1
		getLogger().logError("Type with hash: " + std::to_string(typeHash) + " not registered");
#else
		OS_UNUSED(typeHash);
#endif
	}
	void Serializer::typeNotRegistered(const ISerializable* obj)
	{
#if LOGGER_LIBRARY_AVAILABLE == 1
		getLogger().logError("Type: " + std::string(typeid(*obj).name()) + " not registered");
#else
		OS_UNUSED(obj);
#endif
	}

	
	bool Serializer::setCursorToID(std::fstream& file, std::size_t id)
	{
		// go to the start of the file
		file.seekg(0, std::ios::beg);
		const VTableMetaData& vTableMetaData = getVTableMetaData();
		const auto& mataMap = getObjectMetaData();

		struct LoaderData
		{
			ISerializableID* instance = nullptr;
			Serializer::ObjectMetaData metaData;
			LoaderData(ISerializableID* instance, const Serializer::ObjectMetaData& metaData)
				: instance(instance)
				, metaData(metaData)
			{}
			LoaderData(LoaderData&& other) noexcept
				: instance(other.instance)
				, metaData(std::move(other.metaData))
			{
				other.instance = nullptr;
			}
			~LoaderData()
			{
				delete instance;
			}
		};

		std::unordered_map<std::size_t, LoaderData> objectMap;
		for (const auto& it : mataMap)
		{
			ISerializable* obj = it.second.create();
			ISerializableID* id = dynamic_cast<ISerializableID*>(obj);
			if (id)
			{
				objectMap.insert({ it.second.typeHash, LoaderData(id, it.second) });
			}
			else
				delete obj;
		}

		while (file)
		{
			std::size_t typeHash;
			file.read(reinterpret_cast<char*>(&typeHash), sizeof(typeHash));
			if (!file)
			{
#if LOGGER_LIBRARY_AVAILABLE == 1
				getLogger().logError("Failed to read objects");
#endif
				return false; // EOF or read error
			}

			const auto& it = mataMap.find(typeHash);

			if (it != mataMap.end()) {
				const auto& loaderIt = objectMap.find(typeHash);
				const ObjectMetaData& meta = it->second;
				// Call the factory function to load the object

				if (loaderIt != objectMap.end())
				{
					char* startData = reinterpret_cast<char*>(loaderIt->second.instance);
					size_t byteCount = meta.size;
					if (!vTableMetaData.serializeVtable)
					{
						byteCount -= vTableMetaData.size;
						if (vTableMetaData.location == VTableMetaData::Location::Beginning)
						{
							startData += vTableMetaData.size;
						}
					}
					file.read(startData, byteCount);

					if (loaderIt->second.instance->getID() == id)
					{
						// move back to the start of the object
						file.seekg(-byteCount - sizeof(typeHash), std::ios::cur);
						return true;
					}
				}
				else
				{
					// Skip the amount of bytes for this object
					size_t skipSize = meta.size;
					skipSize -= vTableMetaData.size * size_t(!vTableMetaData.serializeVtable);
					file.ignore(skipSize);
				}
			}
		}
		return false;
	}


	std::unordered_map<std::size_t, Serializer::ObjectMetaData>& Serializer::getObjectMetaData()
	{
		static std::unordered_map<std::size_t, ObjectMetaData> objectMetaData;
		return objectMetaData;
	}
	Serializer::VTableMetaData& Serializer::getVTableMetaData()
	{
		static VTableMetaData vTableMetaData;
		return vTableMetaData;
	}
}