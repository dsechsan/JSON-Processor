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
        std::variant<bool,int,double,std::string,NullType,ObjectType,ListType> value;

        // default constructor
        ModelNode() : value(NullType{}){};

        // constructor with input value;
        explicit ModelNode(std::variant<bool,int,double,std::string> aVal) {
            if (std::holds_alternative<bool>(aVal)) {
                value = std::get<bool>(aVal);
            }else if (std::holds_alternative<int>(aVal)) {
                value = std::get<int>(aVal);
            }else if (std::holds_alternative<double>(aVal)) {
                value = std::get<double>(aVal);
            } else if (std::holds_alternative<std::string>(aVal)) {
                value = std::get<std::string>(aVal);
            }
        }

        explicit ModelNode(const ObjectType &obj ){
            value = deepCopyObjectType(obj);
        }

        explicit ModelNode(const ListType & list ){
            value = deepCopyListType(list);
        }

        // Copy constructor
        ModelNode(const ModelNode& aCopy) {
            if (std::holds_alternative<bool>(aCopy.value)) {
                value = std::get<bool>(aCopy.value);
            } else if (std::holds_alternative<int>(aCopy.value)) {
                value = std::get<int>(aCopy.value);
            }else if (std::holds_alternative<double>(aCopy.value)) {
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

        //helper functions
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
        //destructor
        ~ModelNode() = default;

	};

	class Model : public JSONListener {
	public:

		Model();
		~Model() override = default;
		Model(const Model& aModel);
		Model &operator=(const Model& aModel);

		ModelQuery createQuery();

        //getter for rootNode
        std::shared_ptr<ModelNode> getRoot(){
            return rootNode;
        }

        template<typename T>
        static T convertToType(const std::string& anInput){
            std::stringstream iss(anInput);
            T value;
            iss >> std::boolalpha >> value;
            return value;
        }

	protected:
		// JSONListener methods
		bool addKeyValuePair(const std::string &aKey, const std::string &aValue, Element aType) override;
		bool addItem(const std::string &aValue, Element aType) override;
		bool openContainer(const std::string &aKey, Element aType) override;
		bool closeContainer() override;


		// STUDENT: Your model will contain a collection of ModelNode*'s
		//          Choose your container(s) wisely

        // My tree contains a rootNode that points to an ModelNode object whose value points to a ModelNode object
        // nodeStack is a vector of shared pointers to child objects and lists, whose data ownership gets transferred to rootNode;
        std::shared_ptr<ModelNode> rootNode;
        std::vector<std::shared_ptr<ModelNode>> nodeStack;

        // helper function to store the values of JSON in their original type
        // used the int/double separation from chatgpt
        void updateTheNewNode(Element aType, std::shared_ptr<ModelNode> aModelNodePtr
                                                    , const std::string & aValue) {
            switch (aType) {
                case Element::quoted:
                    aModelNodePtr->value = aValue;
                    break;
                case Element::constant:
                    if (aValue == "true" || aValue == "false") {
                        bool theBoolValue = convertToType<bool>(aValue);
                        aModelNodePtr->value = theBoolValue;
                    } else if (aValue == "null") {
                        aModelNodePtr->value = ModelNode::NullType{};
                    } else {
                        std::stringstream ss(aValue);
                        double dValue;
                        ss >> dValue;

                        if (!ss.fail() && ss.eof()) {
                            if (dValue == static_cast<double>(static_cast<int>(dValue))) {
                                aModelNodePtr->value = static_cast<int>(dValue);
                            } else {
                                aModelNodePtr->value = dValue;
                            }
                        }
                    }
                    break;
                default:
                    aModelNodePtr->value = aValue;
                    break;
            }
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
        // currentNode tracks the position of the pointer to process the query
        std::shared_ptr<ModelNode> currentNode;

        //parseQuery processes string inside 'select' keyword
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

        // traversal moves the currentNode to the location of the selected query
        bool traversal(const std::string &component){
            if(!currentNode) return false;
            if(component.front() == '\'' && component.back() == '\''){
                std::string theKey = component.substr(1,component.length()-2);
                if(std::holds_alternative<ModelNode::ObjectType>(currentNode->value)){
                    auto obj = std::get<ModelNode::ObjectType>(currentNode->value);
                    // traversal - find keyword works on map, returns end if it doesn't find any;
                    auto found = obj.find(theKey);
                    if (found != obj.end()) {
                        // if we find the key, point the currentNode to its value
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
                        // if the index is valid, point the currentNode to its value
                        currentNode = list.at(index);
                        return true;
                    }
                }

            }
            return false;
        }
        // Helper function to print the value of a ModelNode object/list
        static std::string nodePtrToString(const std::shared_ptr<ModelNode>& node){
            std::stringstream ss;

            if(std::holds_alternative<double>(node->value)){
                ss << std::get<double>(node->value);
            }
            if(std::holds_alternative<int>(node->value)){
                ss << std::get<int>(node->value);
            }
            if(std::holds_alternative<std::string>(node->value)){
                ss << "\"" << std::get<std::string>(node->value) << "\"";
            }
            if(std::holds_alternative<bool>(node->value)){
                ss << std::boolalpha<< std::get<bool>(node->value);
            }
            if(std::holds_alternative<ModelNode::NullType>(node->value)){
                ss << "null";
            }

            if(std::holds_alternative<ModelNode::ObjectType>(node->value)){
                auto& obj = std::get<ModelNode::ObjectType>(node->value);
                ss << "{";
                // print nested objects
                for (auto iter = obj.begin(); iter != obj.end(); ++iter) {
                    if (iter != obj.begin()) ss << ", ";
                    ss << "\"" << iter->first << "\":" << nodePtrToString(iter->second);
                }
                ss << "}";
            } else if (std::holds_alternative<ModelNode::ListType>(node->value)) {
                // Serialize nested list
                auto& list = std::get<ModelNode::ListType>(node->value);
                ss << "[";
                // print nested lists
                for (size_t i = 0; i < list.size(); ++i) {
                    if (i > 0) ss << ", ";
                    ss << nodePtrToString(list[i]);
                }
                ss << "]";
        }
            return ss.str();
        }

        // a container to hold the strings in 'filter' query
        struct FilterQuery{
            std::string target;
            std::string operation;
            std::string value;
        };
        
        // Parse the strings in the 'filter' query.
        FilterQuery parseFilterQuery(const std::string &aQuery){
            FilterQuery result;
            std::stringstream ss(aQuery);
            ss >> result.target;
            if(result.target == "key"){
                ss >> result.operation;
                std::getline(ss,result.value, '\'');
                std::getline(ss,result.value,'\'');
            }else if(result.target == "index"){
                ss >> result.operation;
                ss >> result.value;
            }
        return result;
        }
	};


} // namespace ECE141
