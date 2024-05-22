#include "json.hpp"
#include "queue.hpp"
#include "sax_event_consumer.hpp"
#include "definitions/define.hpp"

using json = nlohmann::json;

bool hasEmptySpace(const std::string& str)
{
    for (char c : str) {
        if (c == ' ') {
            return true;
        }
    }
    return false;
}

std::string sax_event_consumer::setPath(std::string name)
{
    std::string path = "";
    if (this->graph.lists.listNodes[myStack.top()->parent->name]->type == "array")
    {
        return path = myStack.top()->path + myStack.top()->name + "." + name;
    }
    else if (this->graph.lists.listNodes[myStack.top()->name]->type == "array")
    {
        return path = myStack.top()->path + "." + name + ".object";
    }
    else
    {
        return path = myStack.top()->path + "." + name;
    }
}

void sax_event_consumer::insertChildren(std::string type)
{
    if (this->graph.lists.listNodes[myStack.top()->name]->type != "array") return;
    myStack.top()->children.push(type);
}

void sax_event_consumer::handleArrayEnum(std::string value)
{
    if (this->graph.lists.listNodes[myStack.top()->name]->type != "array") return;
    if (myStack.top()->enums.size()+1 > MAXENUMARRAY)
        this->graph.lists.listNodes[myStack.top()->name]->isEnum = false;
    if (this->graph.lists.listNodes[myStack.top()->name]->isEnum)
    {
        myStack.top()->enums.push_back(value);
    }
}

void sax_event_consumer::handleKey(std::string type, bool isEnum)
{
    if (name == "") name = myStack.top()->name;
    if (myStack.top() != NULL && myStack.top()->parent != NULL)
    {
        graph.insertNode(
            name,
            type,
            myStack.top()->path + "." + name,
            this->graph.lists.listNodes[myStack.top()->name],
            this->graph.lists.listNodes[myStack.top()->parent->name]);
    }
    else graph.insertNode(name, type, myStack.top()->path + "." + name, NULL, NULL);
    name = name + "." + type;
    if ((type == "string" || type == "integer") && isEnum == false)
    {
        this->graph.lists.listNodes[name]->isEnum = isEnum;
    }
    graph.insertEdge(myStack.top()->name, name, "parent", "");
    name = "";
}

bool sax_event_consumer::null()
{
    queue.push(queue.createStruct(name, "null", ""));
    handleKey("null", false);
    if (this->graph.lists.listNodes[myStack.top()->name]->type == "array")
    {
        this->graph.lists.listNodes[myStack.top()->name]->isEnum = false;
    }
    insertChildren("null");
    return true;
}
 
bool sax_event_consumer::boolean(bool val)
{
    queue.push(queue.createStruct(name, "boolean", ""));
    handleKey("boolean", false);
    if (this->graph.lists.listNodes[myStack.top()->name]->type == "array")
    {
        this->graph.lists.listNodes[myStack.top()->name]->isEnum = false;
    }
    insertChildren("boolean");
    return true;
}
 
bool sax_event_consumer::number_integer(number_integer_t val)
{
    std::string auxName = name;

    bool isEnum = val > MAXINT
        ? false
        : true;
    handleKey("integer", isEnum);
    if (val > MAXINT)
    {
        queue.push(queue.createStruct(auxName, "integer", ""));
        if (this->graph.lists.listNodes[myStack.top()->name]->type == "array")
        {
            this->graph.lists.listNodes[myStack.top()->name]->isEnum = false;
        }
        insertChildren("integer");
        return true;
    }
    if (this->graph.lists.listNodes[myStack.top()->name]->type == "array")
        handleArrayEnum(to_string(val));
    else graph.saveEnum2(auxName + "." + "integer", to_string(val));
    queue.push(queue.createStruct(auxName, "integer", to_string(val)));
    insertChildren("integer");
    return true;
}

bool sax_event_consumer::number_unsigned(number_unsigned_t val)
{
    std::string auxName = name;
    bool isEnum = val > MAXINT
        ? false
        : true;
    handleKey("integer", isEnum);
    if (val > MAXINT)
    {
        queue.push(queue.createStruct(auxName, "integer", ""));
        if (this->graph.lists.listNodes[myStack.top()->name]->type == "array")
        {
            this->graph.lists.listNodes[myStack.top()->name]->isEnum = false;
        }
        insertChildren("integer");
        return true;
    }
    if (this->graph.lists.listNodes[myStack.top()->name]->type == "array")
        handleArrayEnum(to_string(val));
    else graph.saveEnum2(auxName + "." + "integer", to_string(val));
    queue.push(queue.createStruct(auxName, "integer", to_string(val)));
    insertChildren("integer");
    return true;
}

bool sax_event_consumer::number_float(number_float_t val, const string_t& s)
{
    queue.push(queue.createStruct(name, "double", ""));
    handleKey("double", false);
    if (this->graph.lists.listNodes[myStack.top()->name]->type == "array")
    {
        this->graph.lists.listNodes[myStack.top()->name]->isEnum = false;
    }
    insertChildren("double");
    return true;
}

bool sax_event_consumer::string(string_t& val)
{
    std::string auxName = name;
    bool isEnum = val.length() > MAXSTRINGLENGHT
        ? false
        : true;
    handleKey("string", isEnum);
    if (val.length() > MAXSTRINGLENGHT)
    {
        queue.push(queue.createStruct(auxName, "string", ""));
        if (this->graph.lists.listNodes[myStack.top()->name]->type == "array")
        {
            this->graph.lists.listNodes[myStack.top()->name]->isEnum = false;
        }
        insertChildren("string");
        return true;
    }
    if (this->graph.lists.listNodes[myStack.top()->name]->type == "array")
        handleArrayEnum(val);
    else graph.saveEnum2(auxName + "." + "string", val);
    queue.push(queue.createStruct(auxName, "string", val));
    insertChildren("string");
    return true;
}

bool sax_event_consumer::start_object(std::size_t elements)
{
    if (myStack.empty())
    {
        queue.push(queue.createStruct("start_object", "object", ""));
        graph.insertNode("start_object", "object", "start_object", NULL, NULL);
        Stack *newOne = new Stack();
        newOne->name = "start_object.object";
        newOne->path = "start_object";
        myStack.push(newOne);
    }
    else
    {
        insertChildren("object");
        std::string nameBefore = name;
        queue.push(queue.createStruct(name, "object", ""));
        if (name == "") name = myStack.top()->name;
        std::string path = "";
        if (myStack.top() != NULL && myStack.top()->parent != NULL)
        {
            path = myStack.top()->path + "." + name;
        
            graph.insertNode(
                name,
                "object",
                path,
                this->graph.lists.listNodes[myStack.top()->name],
                this->graph.lists.listNodes[myStack.top()->parent->name]);
        }
        else graph.insertNode(name, "object", myStack.top()->path + "." + name, NULL, NULL);
        name = name + "." + "object";
        graph.insertEdge(myStack.top()->name, name, "parent", ""); 
        Stack *newOne = new Stack();
        newOne->name = name;
        newOne->path = path == ""
         ? myStack.top()->path + "." + nameBefore
         : path;
        newOne->parent = myStack.top();
        myStack.push(newOne);
        if (this->graph.lists.listNodes[myStack.top()->name]->type == "array")
        {
            this->graph.lists.listNodes[myStack.top()->name]->isEnum = false;
        }
    }
    name = "";
    return true;
}

bool sax_event_consumer::end_object()
{
    queue.push(queue.createStruct("end_object", "", ""));
    myStack.pop();
    return true;
}

bool sax_event_consumer::start_array(std::size_t elements)
{
    insertChildren("array");
    std::string nameBefore = name;
    queue.push(queue.createStruct(name, "array", ""));
    if (this->graph.lists.listNodes[myStack.top()->name]->type == "array")
    {
        this->graph.lists.listNodes[myStack.top()->name]->isEnum = false;
    }
    if (name == "") name = myStack.top()->name;
    std::string path = "";
    if (myStack.top() != NULL && myStack.top()->parent != NULL)
    {
        path = myStack.top()->path + "." + name;
        graph.insertNode(
            name,
            "array",
            path,
            this->graph.lists.listNodes[myStack.top()->name],
            this->graph.lists.listNodes[myStack.top()->parent->name]);
    }
    else graph.insertNode(name, "array", myStack.top()->path + "." + nameBefore, NULL, NULL);
    name = name + "." + "array";
    graph.insertEdge(myStack.top()->name, name, "parent", ""); 
    Stack *newOne = new Stack();
    newOne->name = name;
    newOne->parent = myStack.top();
    newOne->path = path == ""
     ? myStack.top()->path + "." + name
     : path;
    myStack.push(newOne);
    name = "";
    return true;
}

bool sax_event_consumer::end_array()
{
    queue.push(queue.createStruct("end_array", "", ""));
    if (this->graph.lists.listNodes[myStack.top()->name]->isEnum)
        graph.saveEnumArray(myStack.top()->name, myStack.top()->enums);
    graph.setCollectionStructure(myStack.top()->children, this->graph.lists.listNodes[myStack.top()->name]);
    myStack.top()->enums.clear();
    myStack.pop();
    return true;
}

bool sax_event_consumer::key(string_t& val)
{
    name = val;
    return true;
}

bool sax_event_consumer::binary(json::binary_t& val)
{
    queue.push(queue.createStruct(name, "binary", ""));
    handleKey("binary", false);
    if (this->graph.lists.listNodes[myStack.top()->name]->type == "array")
    {
        this->graph.lists.listNodes[myStack.top()->name]->isEnum = false;
    }
    insertChildren("binary");
    return true;
}

bool sax_event_consumer::parse_error(std::size_t position, const std::string& last_token, const json::exception& ex)
{
    return false;
}