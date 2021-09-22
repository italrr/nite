#ifndef GAME_STORYLINE_HPP
    #define GAME_STORYLINE_HPP

    #include "Engine/Tools/Tools.hpp"
    #include "Entity.hpp"

    namespace Game {

        namespace Story {

            struct Identifier {
                int refId;
                String symRefId;
            };

            enum MemoryObjectTempType {
                SHORT_TERM,
                LONG_TERM
            };

            enum MemoryObjectDataType {
                NONE,
                STRING,
                INTEGER,
                FLOAT,
                BOOLEAN
            };

            enum ConditionType {
                ENTITY_POSITION,
                ENTITY_NAME,
                ENTITY_PROXIMITY_TO_ENTITY,
                GLOBAL_MEMORY_VALUE
            };

            struct MemoryObject : Identifier {
                int tempType;
                int dataType;
                int createdBy;
                int modifiedBy;
                UInt64 createdDate;
                UInt64 modifiedDate;
                String description;
                String name;
                char *data;
                size_t size;
                MemoryObject();
                ~MemoryObject();
                void clear();
                bool read(void *target, size_t bytes);
                bool write(const void *source, size_t bytes, int type);
            };

            struct MemoryObjectInt : MemoryObject {
                using MemoryObject::read;
                using MemoryObject::write;
                MemoryObjectInt();
                bool write(int32_t v);
                bool read(int32_t &v);
            };

            struct MemoryObjectFloat : MemoryObject {
                using MemoryObject::read;
                using MemoryObject::write;
                MemoryObjectFloat();
                bool write(float v);
                bool read(float &v);
            };

            struct MemoryObjectBool : MemoryObject {
                using MemoryObject::read;
                using MemoryObject::write;
                MemoryObjectBool();
                bool write(bool v);
                bool read(bool &v);
            };

            struct MemoryObjectString : MemoryObject {
                using MemoryObject::read;
                using MemoryObject::write;
                MemoryObjectString();
                bool write(const String &v);
                bool read(String &v);
            };                                    

            struct Condition : Identifier {
                int condType;
                int subjectId;
                virtual bool check();
            };

            struct Quest : Identifier {
                
                Vector<Shared<Condition>> condition;

            };

            struct Conversation {

            };

            struct StoryLine {
                Dict<int, Shared<MemoryObject>> memoryBank;
                UInt64 runStart;
                UInt64 runEnd;
                int enemKilled;
                int enemEncountered;
            };

        }

    }

#endif