#ifndef GAME_STORYLINE_HPP
    #define GAME_STORYLINE_HPP

    #include "Engine/Tools/Tools.hpp"
    #include "UI/ListMenu.hpp"
    #include "Dialog.hpp"
    #include "Entity.hpp"

    namespace Game {

        namespace Story {

            struct Identifier {
                int refId;
                String symRefId;
                Identifier();
            };

            namespace InteractionTreeDialogOptionType {
                enum InteractionTreeDialogOptionType {
                    SCRIPT,
                    NEXT,
                    CALLBACK
                };
            }            

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

            struct MemoryBox {
                Dict<String, Shared<MemoryObject>> bank;
                Dict<String, Shared<MemoryObject>> shortTerm;
                MemoryBox();
                void clearShortTerm();
                Shared<MemoryObject> get(const String &name);
                bool store(const String &name, const String &v, bool tmp = false);
                bool store(const String &name, int32_t v, bool tmp = false);
                bool store(const String &name, float v, bool tmp = false);
                bool store(const String &name, bool v, bool tmp = false);
            };

            struct Condition : Identifier {
                int condType;
                int subjectId;
                virtual bool check();
            };

            struct ConditionGroup {
                Vector<Shared<Condition>> conditions;
            };

            namespace InteractionTreeType {
                enum InteractionTreeType {
                    NONE,
                    CONTACT,
                    EXPOSITION,
                    DIALOG, // Player chooses something
                    REACTION
                };
            }

            struct InteractionDevice;

            struct InteractionTree : Identifier {
                int interType;
                InteractionDevice *device;
                Vector<Shared<ConditionGroup>> conditions; // one group per next interaction
                Vector<String> next;
                virtual void run();
                InteractionTree();               
            };

            struct InteractionTreeContact : InteractionTree {
                void run();
                InteractionTreeContact();                
            }; 

            struct InteractionTreeExposition : InteractionTree {
                Vector<Shared<DialogLine>> lines;
                Shared<DialogBox> dialogDevice;
                void run();
                InteractionTreeExposition();               
            };     

            struct InteractionTreeDialogOption {
                int type;
                String next;
                String script;
                String label;
                std::function<void(const String &root)> callback;

                InteractionTreeDialogOption(){
                    type = InteractionTreeDialogOptionType::NEXT;
                }
               
            };

            static Shared<InteractionTreeDialogOption> buildTreeDialogOption(const String &label, const String &next){
                auto obj = std::make_shared<Game::Story::InteractionTreeDialogOption>(Game::Story::InteractionTreeDialogOption());
                obj->label = label;
                obj->next = next;
                obj->type = InteractionTreeDialogOptionType::NEXT;
                return obj;
            }

            struct InteractionTreeDialog : InteractionTree {
                Vector<Shared<InteractionTreeDialogOption>> options;
                String title;
                Shared<UIListMenu> optionsDevice;
                void run();
                InteractionTreeDialog();              
            };                         

            struct InteractionGroup : Identifier {
                Dict<String, Shared<Game::Story::InteractionTree>> interactions;
            };

            struct StoryLine;
            struct InteractionDevice {
                Shared<UIListMenu> optionsDevice;
                Shared<DialogBox> dialogDevice;
                // Shared<InteractionTree> current;
                Game::Story::StoryLine *storyLine;
                Dict<String, Shared<InteractionTree>> interactions;
                String startInter; // symRefId
                InteractionDevice();
                UInt64 lastInter;
                bool busy;
                void addInter(Shared<InteractionTree> inter);
                void next(const String &symRefId);
                void step();
                void start(const String &startInter = "");
                void end();
                void render();
            };


            struct StoryLine {
                Shared<Game::Story::InteractionDevice> interDevice;
                Dict<String, Shared<Game::Story::InteractionGroup>> interactionGroups;
                Shared<Game::Story::MemoryBox> memBox;
                UInt64 runStart;
                UInt64 runEnd;
                int enemKilled;
                int enemEncountered;
                StoryLine();
                void loadStoryLine(const String &file);
                void step();
                void render();
                void setup(Shared<UIListMenu> &optionsDevice, Shared<DialogBox> &dialogDevice);
                                
            };

        }

    }

#endif 