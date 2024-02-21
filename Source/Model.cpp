//
// Created by Mark on 1/30/2024.
//

#include "Model.h"
#include "Debug.h"

namespace ECE141 {

	// ---Model---

	Model::Model() : rootNode(std::make_shared<ModelNode>()) {}

	Model::Model(const Model& aModel) {
        if(aModel.rootNode){
            rootNode = std::make_shared<ModelNode>(*aModel.rootNode);
        }
	}

	Model &Model::operator=(const Model& aModel) {
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
//        return *this;
    }


	bool Model::addKeyValuePair(const std::string& aKey, const std::string& aValue, Element aType) {
//        ModelNode theNewNode;
        if(!nodeStack.empty()) {
            auto &currentNode = nodeStack.back();
            if(std::holds_alternative<ModelNode::ObjectType>(currentNode->value)) {
                auto &obj =std::get<ModelNode::ObjectType>(currentNode->value);
                auto theNewNode = std::make_shared<ModelNode>();
                switch (aType) {
                    case Element::quoted:
                        theNewNode->value = aValue;
                        break;
                    case Element::constant:
                        if (aValue == "true" || aValue == "false") {
                            bool theBoolValue = convertToType<bool>(aValue);
                            theNewNode->value = theBoolValue;
                        } else if (aValue == "null") {
                            theNewNode->value = ModelNode::NullType{};
                        } else {
                            theNewNode->value = convertToType<double>(aValue);
                        }
                        break;

                    default:
                        theNewNode->value = aValue;
                        break;
                }
                obj.insert({aKey,theNewNode});
                return true;
            }
        }
        return false;
    }


	bool Model::addItem(const std::string& aValue, Element aType) {
        if(!nodeStack.empty()) {
            auto &currentNode = nodeStack.back();
            if(std::holds_alternative<ModelNode::ListType>(currentNode->value)) {
                auto list =std::get<ModelNode::ListType>(currentNode->value);
                auto theNewNode = std::make_shared<ModelNode>();
                switch (aType) {
                    case Element::quoted:
                        theNewNode->value = aValue;
                        break;
                    case Element::constant:
                        if (aValue == "true" || aValue == "false") {
                            bool theBoolValue = convertToType<bool>(aValue);
                            theNewNode->value = theBoolValue;
                        } else if (aValue == "null") {
                            theNewNode->value = ModelNode::NullType{};
                        } else {
                            theNewNode->value = convertToType<double>(aValue);
                        }
                        break;

                    default:
                        theNewNode->value = aValue;
                        break;
                }
                list.push_back(theNewNode);
                return true;
            }
        }

        return false;
	}

	bool Model::openContainer(const std::string& aKey, Element aType) {
       auto theNewNode = std::make_shared<ModelNode>();
//       ModelNode theNewNode;

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

	bool Model::closeContainer(const std::string& aKey, Element aType) {
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
//		DBG("filter(" << aQuery << ")");
//		TODO;

		return *this;
	}

	size_t ModelQuery::count() {
//		DBG("count()");
//		TODO;

		return 0;
	}

	double ModelQuery::sum() {
//		DBG("sum()");
//		TODO;

		return 0.0;
	}

	std::optional<std::string> ModelQuery::get(const std::string& aKeyOrIndex) {
//		DBG("get(" << aKeyOrIndex << ")");
//		TODO;

		return std::nullopt;
	}


}

// namespace ECE141