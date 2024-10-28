#include "ISerializableID.h"

namespace ObjectSerializer
{
	ISerializableID::ISerializableID()
		: m_id(getNextID())
	{

	}
	std::size_t ISerializableID::getNextID()
	{
		static std::size_t id = 0;
		return id++;
	}
}