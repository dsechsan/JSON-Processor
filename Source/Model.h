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

        ModelNode() = default;
        ~ModelNode() = default;

	};

	class Model : public JSONListener {
	public:

		Model();
		~Model() override = default;
		Model(const Model& aModel);
		Model &operator=(const Model& aModel);

		ModelQuery createQuery();

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

	};


} // namespace ECE141