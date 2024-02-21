//
// Created by Mark on 1/30/2024.
//

#pragma once

#include <string>
#include <optional>
#include "JSONParser.h"
#include <variant>
#include <vector>
#include <map>
#include <sstream>
#include <memory>

namespace ECE141 {

	class ModelQuery; // Forward declare

	// STUDENT: Your Model is built from a bunch of these...
	struct ModelNode {
		// Sometimes a node holds a basic value (null, bool, number, string)
		// Sometimes a node holds a list of other nodes (list)
		// Sometimes a node holds a collection key-value pairs, where the value is a node (an object)
        struct NullType{};
        using ListType = std::vector<std::shared_ptr<ModelNode>>;
        using ObjectType = std::map<std::string,std::shared_ptr<ModelNode>>;
        std::variant<bool,double,std::string,NullType,ObjectType,ListType> value;

        ModelNode() : value(NullType{}){};

        ModelNode(std::variant<bool,double,std::string> aVal) {
            if (std::holds_alternative<bool>(aVal)) {
                value = std::get<bool>(aVal);
            } else if (std::holds_alternative<double>(aVal)) {
                value = std::get<double>(aVal);
            } else if (std::holds_alternative<std::string>(aVal)) {
                value = std::get<std::string>(aVal);
            }
        }

        ModelNode(const ObjectType &obj ){
            value = deepCopyObjectType(obj);
        }

        explicit ModelNode(const ListType & list ){
            value = deepCopyListType(list);
        }


        ModelNode(const ModelNode& aCopy) {
            if (std::holds_alternative<bool>(aCopy.value)) {
                value = std::get<bool>(aCopy.value);
            } else if (std::holds_alternative<double>(aCopy.value)) {
                value = std::get<double>(aCopy.value);
            } else if (std::holds_alternative<std::string>(aCopy.value)) {
                value = std::get<std::string>(aCopy.value);
            } else if (std::holds_alternative<NullType>(aCopy.value)) {
                value = NullType{};
            } else if (std::holds_alternative<ObjectType>(aCopy.value)) {
                auto theOriginalObj = std::get<ObjectType>(aCopy.value);
                value = deepCopyObjectType(theOriginalObj);
            } else if (std::holds_alternative<ListType>(aCopy.value)) {
                auto theOriginalList = std::get<ListType>(aCopy.value);
                value = deepCopyListType(theOriginalList);
            }
        }


        static ObjectType deepCopyObjectType(const ObjectType & aOriginalObj) {
            ObjectType theCopy;
            for (const auto &pair : aOriginalObj) {
                theCopy.insert({pair.first,std::make_shared<ModelNode>(*pair.second)});
            }
            return theCopy;
        }

        static ListType deepCopyListType(const ListType & aOriginalList) {
            ListType theCopy;
            for (const auto &val : aOriginalList) {
                theCopy.emplace_back(std::make_shared<ModelNode>(*val));
            }
            return theCopy;
        }
        ~ModelNode() = default;

	};

	class Model : public JSONListener {
	public:

		Model();
		~Model() override = default;
		Model(const Model& aModel);
		Model &operator=(const Model& aModel);

		ModelQuery createQuery();
        std::shared_ptr<ModelNode> getRoot(){
            return rootNode;
        }


	protected:
		// JSONListener methods
		bool addKeyValuePair(const std::string &aKey, const std::string &aValue, Element aType) override;
		bool addItem(const std::string &aValue, Element aType) override;
		bool openContainer(const std::string &aKey, Element aType) override;
		bool closeContainer(const std::string &aKey, Element aType) override;


		// STUDENT: Your model will contain a collection of ModelNode*'s
		//          Choose your container(s) wisely
        std::shared_ptr<ModelNode> rootNode;
        std::vector<std::shared_ptr<ModelNode>> nodeStack;

        template<typename T>
        static T convertToType(const std::string& anInput){
            std::stringstream iss(anInput);
            T value;
            iss >> std::boolalpha >> value;
            return value;
        }


    };

	class ModelQuery {
	public:
		ModelQuery(Model& aModel);

		// ---Traversal---
		ModelQuery& select(const std::string& aQuery);

		// ---Filtering---
		ModelQuery& filter(const std::string& aQuery);

		// ---Consuming---
		size_t count();
		double sum();
		std::optional<std::string> get(const std::string& aKeyOrIndex);

	protected:
		Model &model;
        std::shared_ptr<ModelNode> currentNode;

        std::vector<std::string> parseQuery(const std::string& aQuery){
            std::vector<std::string> components;
            std::stringstream theStream(aQuery);
            std::string component;

            while(std::getline(theStream,component,'.')){
                if(!component.empty())
                components.push_back(component);
            }
            return components;
        }

        bool traversal(const std::string &component){
            if(!currentNode) return false;
            if(component.front() == '\'' && component.back() == '\''){
                std::string theKey = component.substr(1,component.length()-2);
                if(std::holds_alternative<ModelNode::ObjectType>(currentNode->value)){
                    auto obj = std::get<ModelNode::ObjectType>(currentNode->value);
                    // traversal - find keyword works on map, returns end if it doesn't find any;
                    auto found = obj.find(theKey);
                    if (found != obj.end()) {
                        currentNode = found->second;
                        return true;
                    }
                }
            }else{
                int index = std::stoi(component);
                if(std::holds_alternative<ModelNode::ListType>(currentNode->value)){
                    auto list = std::get<ModelNode::ListType>(currentNode->value);
                    int theListLen = static_cast<int>(list.size());
                    if(index >=0 && index < theListLen){
                        currentNode = list.at(index);
                        return true;
                    }
                }

            }
            return false;
        }

	};


} // namespace ECE141