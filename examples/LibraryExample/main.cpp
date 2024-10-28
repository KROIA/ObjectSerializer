#ifdef QT_ENABLED
#include <QApplication>
#endif
#include <iostream>
#include "ObjectSerializer.h"

#ifdef QT_WIDGETS_ENABLED
#include <QWidget>
#endif

#pragma pack(push, 1) // Start 1-byte alignment for the following structs

// Example structs
struct ExampleStruct : public ObjectSerializer::ISerializableID {
    public:
    ExampleStruct() : ObjectSerializer::ISerializableID() {
    }
	~ExampleStruct() {
	}
    /*int a = 65;
    int b = 65;
    int c = 65;
    int d = 65;
	int e = 65;*/
	char text[5] = "    ";
    float b = 0;
};

struct AnotherStruct : public ObjectSerializer::ISerializable {
    public:
    AnotherStruct() : ObjectSerializer::ISerializable() {
    
    }
    ~AnotherStruct()
    {

    }
    int x = 65;
    int y = 65;
	int z = 65;
};

#pragma pack(pop) // Reset to default alignment

int main(int argc, char* argv[])
{
#ifdef QT_WIDGETS_ENABLED
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
#ifdef QT_ENABLED
	QApplication app(argc, argv);
#endif
    ObjectSerializer::Serializer serializer;

    // Register deserialization handlers for each type
    ObjectSerializer::Serializer::registerType<ExampleStruct>();
    ObjectSerializer::Serializer::registerType<AnotherStruct>();

    ObjectSerializer::Profiler::start();
    ObjectSerializer::LibraryInfo::printInfo();

    ExampleStruct example1;
    ExampleStruct example2;
    memcpy(&example1.text, "AABB", 5);
	example1.b = 3.14f;
    AnotherStruct another1;

    serializer.addObject(&example1);
    serializer.addObject(&example2);
    serializer.addObject(&another1);

    // Serialize objects to file
    serializer.saveToFile("data.bin");

    ObjectSerializer::Serializer deserializer;
    // Load all objects from file
    deserializer.loadFromFile("data.bin");

    std::vector<ObjectSerializer::ISerializable*> objects = deserializer.getObjects();
	ObjectSerializer::ISerializable* test = &example1;
    ExampleStruct* another2 = dynamic_cast<ExampleStruct*>(test);
    for (auto obj : objects)
    {
		ObjectSerializer::ISerializableID* id = dynamic_cast<ObjectSerializer::ISerializableID*>(obj);
        ExampleStruct* example = dynamic_cast<ExampleStruct*>(obj);
        AnotherStruct* another = dynamic_cast<AnotherStruct*>(obj);
        if(id)
        {
			std::cout << "ID: " << id->getID() << "  ";
		}
        if (example) {
            std::cout << "Example struct: text=" << example->text << ", b=" << example->b << "\n";
        }
        else if (another) {
            std::cout << "Another struct: x=" << another->x << ", y=" << another->y << "\n";
        }
        else {
            std::cerr << "Unknown object type encountered.\n";
        }

    }
	ObjectSerializer::ISerializableID* objWithID = nullptr;
	ObjectSerializer::ISerializableID* objWithID1 = nullptr;

	if (deserializer.loadFromFile("data.bin", 1, objWithID))
	{
		ExampleStruct* example = dynamic_cast<ExampleStruct*>(objWithID);
        if (example)
        {
            std::cout << "2 ID: " << example->getID() << "  ";
            std::cout << "Example struct: text=" << example->text << ", b=" << example->b << "\n";
        }
    }
	else
	{
		
	}

    ObjectSerializer::Serializer serializer2;
    memcpy(&dynamic_cast<ExampleStruct*>(objWithID)->text, "AABB", 5);
	serializer2.overrideInFile("data.bin", objWithID);

    if (deserializer.loadFromFile("data.bin", 1, objWithID1))
    {
        ExampleStruct* example = dynamic_cast<ExampleStruct*>(objWithID1);
        if (example)
        {
            std::cout << "2 ID: " << example->getID() << "  ";
            std::cout << "Example struct: text=" << example->text << ", b=" << example->b << "\n";
        }
}
    else
    {

    }



#ifdef QT_WIDGETS_ENABLED
	QWidget* widget = StructSerializer::LibraryInfo::createInfoWidget();
	if (widget)
		widget->show();
#endif
	int ret = 0;
#ifdef QT_ENABLED
	ret = app.exec();
#endif
    ObjectSerializer::Profiler::stop((std::string(ObjectSerializer::LibraryInfo::name) + ".prof").c_str());
	return ret;
}