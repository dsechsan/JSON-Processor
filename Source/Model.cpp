//
// Created by Mark on 1/30/2024.
//

#include "Model.h"
#include "Debug.h"
#include <numeric>

namespace ECE141 {

	// ---Model---

	Model::Model() : rootNode(std::make_shared<ModelNode>()) {}

	Model::Model(const Model& aModel) {
        if(aModel.rootNode){
            // deep copies the data from the rootNode of aModel;
            rootNode = std::make_shared<ModelNode>(*aModel.rootNode);
        }
	}

	Model &Model::operator=(const Model& aModel) {
        // deep copies the entire model
        if(this!= &aModel){
            if(aModel.rootNode){
                rootNode = std::make_shared<ModelNode>(*aModel.rootNode);
            }else {
                rootNode = nullptr;
            }
        }
        return *this;
	}

	ModelQuery Model::createQuery(){
        return ModelQuery(*this);
    }


	bool Model::addKeyValuePair(const std::string& aKey, const std::string& aValue, Element aType) {
        // get the top of the nodeStack and update the object
        if(!nodeStack.empty()) {
            auto &currentNode = nodeStack.back();
            if(std::holds_alternative<ModelNode::ObjectType>(currentNode->value)) {
                auto &obj =std::get<ModelNode::ObjectType>(currentNode->value);
                auto theNewNode = std::make_shared<ModelNode>();
                updateTheNewNode(aType,theNewNode,aValue);
                obj.insert({aKey,theNewNode});
                return true;
            }
        }
        return false;
    }


	bool Model::addItem(const std::string& aValue, Element aType) {
        //get the top of the nodeStack and update the list
        if(!nodeStack.empty()) {
            auto &currentNode = nodeStack.back();
            if(std::holds_alternative<ModelNode::ListType>(currentNode->value)) {
                auto &list =std::get<ModelNode::ListType>(currentNode->value);
                auto theNewNode = std::make_shared<ModelNode>();
                updateTheNewNode(aType,theNewNode,aValue);
                list.push_back(theNewNode);
                return true;
            }
        }

        return false;
	}

	bool Model::openContainer(const std::string& aKey, Element aType) {
        // Create a new shared pointer, map it to the key and push it to the top of the stack
        // If it is a list, add the pointer directly to the list
       auto theNewNode = std::make_shared<ModelNode>();
       if(aType == Element::object) {
           theNewNode->value = ModelNode::ObjectType();
       }else if(aType == Element::array){
           theNewNode->value = ModelNode::ListType();
       }else{
           return false;
       }

       if(nodeStack.empty()){
           rootNode = theNewNode;
       }else{
           auto &currentContainer = nodeStack.back();
           if(std::holds_alternative<ModelNode::ObjectType>(currentContainer->value)){
               auto &obj = std::get<ModelNode::ObjectType>(currentContainer->value);
               obj.insert({aKey,theNewNode});

           }else if (std::holds_alternative<ModelNode::ListType>(currentContainer->value)){
               auto &list = std::get<ModelNode::ListType>(currentContainer->value);
               list.push_back(theNewNode);
           }
       }
       nodeStack.push_back(theNewNode);
       return true;
	}

	bool Model::closeContainer() {
        // since we have shared pointers, no need to worry about data retention
        if(!nodeStack.empty()){
            nodeStack.pop_back();
            return true;
        }
        return false;
	}


    // ---ModelQuery---

	ModelQuery::ModelQuery(Model &aModel) : model(aModel), currentNode(aModel.getRoot()) {}

	ModelQuery &ModelQuery::select(const std::string& aQuery) {
        auto components = parseQuery(aQuery);
        for(const auto &component : components){
            if(!traversal(component)){
                std::cout << "Invalid component " << component << "\n";
                currentNode = nullptr;
                break;
            }
        }
		return *this;
	}

	ModelQuery& ModelQuery::filter(const std::string& aQuery) {
        // parse query
        // if query has key contains,
        //      check if the current container is an object
        //      deepcopy the current container to a filteredObject
        //      if key doesn't contain the value, erase, else continue
        //      create a new pointer and assign its value to the filteredObject
        //      return the new pointer
        // if query has index,
        //     check if the current container is an object
        //     deepcopy the current container to a filteredList
        //     check if the current index is supposed to be erased or retained
        //     erase the relevant values
        //     create a new pointer and assign its value to the filteredList
        //     return the new pointer

        auto theParsedString = parseFilterQuery(aQuery);

        if(theParsedString.target == "key" and theParsedString.operation == "contains"){
            if (currentNode && std::holds_alternative<ModelNode::ObjectType>(currentNode->value)) {
                auto &obj = std::get<ModelNode::ObjectType>(currentNode->value);
                auto filteredObj = ModelNode::deepCopyObjectType(obj);
                auto iter = filteredObj.begin();
                while(iter != filteredObj.end()){
                    if(iter->first.find(theParsedString.value) == std::string::npos) {
                        iter = filteredObj.erase(iter);
                    }else{
                        ++iter;
                    }
                }
                auto theFilteredNode = std::make_shared<ModelNode>();
                theFilteredNode->value = filteredObj;
                currentNode = theFilteredNode;
            }
        }else if (theParsedString.target == "index" && std::holds_alternative<ModelNode::ListType>(currentNode->value)) {
            auto &list = std::get<ModelNode::ListType>(currentNode->value);
            auto filteredList = ModelNode::deepCopyListType(list);
            auto iter = filteredList.begin();
            auto theOperator = theParsedString.operation;
//            std::cout << std::stoi(theParsedString.value) << "\n";
            int theCurrentIndex = 0;
            while(iter != filteredList.end()){
                bool theEraseCondition = false;
                if(theOperator == "<"){
                    theEraseCondition = theCurrentIndex >= std::stoi(theParsedString.value);
                }else if(theOperator == "<="){
                    theEraseCondition = theCurrentIndex > std::stoi(theParsedString.value);
                }else if(theOperator == "=="){
                    theEraseCondition = theCurrentIndex != std::stoi(theParsedString.value);
                }else if(theOperator == ">"){
                    theEraseCondition = theCurrentIndex <= std::stoi(theParsedString.value);
                }else if(theOperator == ">="){
                    theEraseCondition = theCurrentIndex < std::stoi(theParsedString.value);
                }else if(theOperator == "!="){
                    theEraseCondition = theCurrentIndex == std::stoi(theParsedString.value);
                }

                if(theEraseCondition){
                    iter = filteredList.erase(iter);
                    ++theCurrentIndex;
                }else{
                    ++iter;
                    ++theCurrentIndex;
                }
            }

            auto theFilteredNode = std::make_shared<ModelNode>();
            theFilteredNode->value = filteredList;
            currentNode = theFilteredNode;
        }
        return *this;
    }


	size_t ModelQuery::count() {
        // returns the size of the object/list
        if (!currentNode) {
            return 0;
        }

        if (std::holds_alternative<ModelNode::ObjectType>(currentNode->value)) {
            auto &obj = std::get<ModelNode::ObjectType>(currentNode->value);
            return obj.size();
        }

        else if (std::holds_alternative<ModelNode::ListType>(currentNode->value)) {
            auto &list = std::get<ModelNode::ListType>(currentNode->value);
            return list.size();
        }

        return 0;
	}

	double ModelQuery::sum() {
        double sum = 0.0;
        if(!currentNode){
            return 0.0;
        }

        if (std::holds_alternative<ModelNode::ListType>(currentNode->value)) {
            auto &list = std::get<ModelNode::ListType>(currentNode->value);
            for(auto &val : list){
                if(std::holds_alternative<double>(val->value)){
                    sum+= std::get<double>(val->value) ;
                }else if(std::holds_alternative<int>(val->value)) {
                    sum += static_cast<double>(std::get<int>(val->value));
                }
            }
            return sum;
        }
		return 0.0;
	}

	std::optional<std::string> ModelQuery::get(const std::string& aKeyOrIndex) {
        // checks if the input is * or string or integer;
        // * prints all using a stringstream
        // string prints the value of the key
        // integer prints the values of the list;

        if(!currentNode){
            return std::nullopt;
        }
        std::stringstream ss;
        if(aKeyOrIndex == "*"){
            if(std::holds_alternative<ModelNode::ObjectType>(currentNode->value)){
                auto &obj = std::get<ModelNode::ObjectType>(currentNode->value);
                ss << "{";
                for (auto val = obj.begin(); val != obj.end(); val++){
                    if(val != obj.begin()) ss << ",";
                    ss << "\"" << val->first << "\":" << nodePtrToString(val->second);
                }
                ss << "}";
            }else if(std::holds_alternative<ModelNode::ListType>(currentNode->value)){
                auto &list = std::get<ModelNode::ListType>(currentNode->value);
                ss << "[";
                for (size_t i = 0; i< list.size(); i++){
                    if(i>0) ss << ", ";
                    ss << nodePtrToString(list[i]);
                }
                ss << "]";
            }
            return ss.str();
        }

        if(std::holds_alternative<ModelNode::ObjectType>(currentNode->value)){
            auto &obj = std::get<ModelNode::ObjectType>(currentNode->value);
            std::string theKey;
            if(aKeyOrIndex.front() == '\'' && aKeyOrIndex.back() == '\'') {
                theKey = aKeyOrIndex.substr(1, aKeyOrIndex.length() - 2);
            }
            auto val = obj.find(theKey);
            if(val != obj.end()){
                ss << nodePtrToString(val->second) ;
                return ss.str();
            }
        }

        if(std::holds_alternative<ModelNode::ListType>(currentNode->value)){
            auto &list = std::get<ModelNode::ListType>(currentNode->value);
            int index = std::stoi(aKeyOrIndex);
            if(index >=0 && index < static_cast<int>(list.size())){
                ss << nodePtrToString(list[index]);
                return ss.str();
            }
        }

		return std::nullopt;
	}


}

// namespace ECE141