#pragma once
#include "ObjectSerializer_base.h"
#include "ISerializable.h"

namespace ObjectSerializer
{
    class OBJECT_SERIALIZER_API ISerializableID : public ISerializable
    {
        public:
        ISerializableID();
        virtual ~ISerializableID() = default;

		void setID(std::size_t id)
		{
			m_id = id;
		}
		std::size_t getID() const
		{
			return m_id;
		}

        private:
        std::size_t m_id;

        static std::size_t getNextID();
    };
}