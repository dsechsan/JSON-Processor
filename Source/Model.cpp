//
// Created by Mark on 1/30/2024.
//

#include "Model.h"
#include "Debug.h"

namespace ECE141 {

	// ---Model---

	Model::Model() {

//		TODO; // Remove once you have implemented this method
	}

	Model::Model(const Model& aModel) {
//		TODO;
	}

	Model &Model::operator=(const Model& aModel) {
//		TODO;
		return *this;
	}

	ModelQuery Model::createQuery() {
        return ModelQuery(*this);
//        return *this;
    }


	bool Model::addKeyValuePair(const std::string& aKey, const std::string& aValue, Element aType) {
//        ModelNode theNewNode;
        if(!nodeStack.empty()) {
            auto &currentNode = nodeStack.back();
            if(std::holds_alternative<std::shared_ptr<ModelNode::ObjectType>>(currentNode->value)) {
                auto obj =std::get<std::shared_ptr<ModelNode::ObjectType>>(currentNode->value);
                ModelNode theNewNode;
                switch (aType) {
                    case Element::quoted:
                        theNewNode.value = aValue;
                        break;
                    case Element::constant:
                        if (aValue == "true" || aValue == "false") {
                            bool theBoolValue = convertToType<bool>(aValue);
                            theNewNode.value = theBoolValue;
                        } else if (aValue == "null") {
                            theNewNode.value = ModelNode::NullType{};
                        } else {
                            theNewNode.value = convertToType<double>(aValue);
                        }
                        break;

                    default:
                        theNewNode.value = aValue;
                        break;
                }
                obj->insert({aKey,theNewNode});
                return true;
            }
        }
        return false;
    }


	bool Model::addItem(const std::string& aValue, Element aType) {
        if(!nodeStack.empty()) {
            auto &currentNode = nodeStack.back();
            if(std::holds_alternative<std::shared_ptr<ModelNode::ListType>>(currentNode->value)) {
                auto obj =std::get<std::shared_ptr<ModelNode::ListType>>(currentNode->value);
                ModelNode theNewNode;
                switch (aType) {
                    case Element::quoted:
                        theNewNode.value = aValue;
                        break;
                    case Element::constant:
                        if (aValue == "true" || aValue == "false") {
                            bool theBoolValue = convertToType<bool>(aValue);
                            theNewNode.value = theBoolValue;
                        } else if (aValue == "null") {
                            theNewNode.value = ModelNode::NullType{};
                        } else {
                            theNewNode.value = convertToType<double>(aValue);
                        }
                        break;

                    default:
                        theNewNode.value = aValue;
                        break;
                }
                obj->push_back(theNewNode);
                return true;
            }
        }

        return false;
	}

	bool Model::openContainer(const std::string& aKey, Element aType) {
       auto theNewNode = std::make_shared<ModelNode>();
//       ModelNode theNewNode;

       if(aType == Element::object) {
           theNewNode->value = std::make_shared<ModelNode::ObjectType>();
       }else if(aType == Element::array){
           theNewNode->value = std::make_shared<ModelNode::ListType>();
       }else{
           return false;
       }

       if(nodeStack.empty()){
           rootNode = theNewNode;
       }else{
           auto currentContainer = nodeStack.back();
           if(std::holds_alternative<std::shared_ptr<ModelNode::ObjectType>>(currentContainer->value)){
               auto &obj = std::get<std::shared_ptr<ModelNode::ObjectType>>(currentContainer->value);
               obj->insert({aKey,*theNewNode});

           }else if (std::holds_alternative<std::shared_ptr<ModelNode::ListType>>(currentContainer->value)){
               auto &list = std::get<std::shared_ptr<ModelNode::ListType>>(currentContainer->value);
               list->push_back(*theNewNode);
           }
       }
       nodeStack.push_back(theNewNode);
       return true;
	}

	bool Model::closeContainer(const std::string& aKey, Element aType) {
        if(!nodeStack.empty()){
            nodeStack.pop_back();
        }
        return true;

	}

	// ---ModelQuery---

	ModelQuery::ModelQuery(Model &aModel) : model(aModel) {}

	ModelQuery& ModelQuery::select(const std::string& aQuery) {
//		DBG("select(" << aQuery << ")");
//		TODO;

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