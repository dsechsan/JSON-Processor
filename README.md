[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/DLN_anPa)
# WI-24 Assignment 3 - JSON Processor
## Due Feb 19, 11:59 PM (PST)

Build your own JSON parser, in-memory model, and query command system! 
This will involve
- Parsing JSON text files.
- Building an in-memory model of the JSON.
- Creating and using a simple command language for querying the in-memory model and retrieving data from the JSON.

## 0. What is JSON?

JSON (or **J**ava**S**cript **O**bject **N**otation) was created for use in web development.
The goal was to create a simple text-format to transmit and store complex data. 

For instance, imagine you want to store data for a school classroom. What would you want to store?
- Classroom description (class name, teacher, location, class times, etc.)
- A list of students
  - Store each student's name, age, grade, etc.

Here is how you can represent that data in the JSON format:

```json
{
  "className": "Math 101",
  "teacher": "Mr. Farber",
  "startTime": "10:00am",
  "endTime": "11:00am",
  "hasFinalExam": true,
  "location": {
    "roomNumber": 247,
    "floor": 2
  },
  "students": [
    {
      "name": "Micheal",
      "age": 17,
      "grade": 94
    },
    {
      "name": "Sarah",
      "age": 18,
      "grade": 96.5
    },
    {
      "name": "Brian",
      "age": 16,
      "grade": 73
    },
    {
      "name": "Lisa",
      "age": 19,
      "grade": null
    }
  ]
}
```

Important things to notice:

- All data is either stored within **objects** (created with curly braces `{}`) or **lists** (created with square brackets `[]`).

- Data within **objects** `{}` are stored using **key-value** pairs.
  - For instance, in the line `"className": "Math 101"`, `"className"` is the key and `"Math 101"` is the value.
  - They **keys** will always be strings, while the **values** can be any of the types described below.

- **Lists** `[]` only store **values** -- there are no keys involved.
  - For instance, within the `"students"` list, we see four different objects, each representing a different student.
  - For a simpler example, check out this JSON that contains a list of numbers: `{ "myList": [57, 42, 2, 3] }`.
  - Instead of keys (like in objects), each element in a list is associated with an **index**. And like arrays in C++, the indices start at `0`. 

- A **value** can be 6 different types: `null`, `boolean`, `number`, `string`, `list`, or `object`
  - This means that `object`s and `list`s can store additional `object`s and `list`s within themselves!
  - Notice the `number` type. In JavaScript, `number` can represent both integer and floating-point types. 
    - In C++, we can separate this `number` type into distinct `long` and `double` types to correctly store the values.
- For a more detailed and formal description, check out [json.org](https://www.json.org/json-en.html).

We have addition JSON examples in this repository (in `Resources`), feel free to look through them to better familiarize yourself with the syntax.

## 1. The Parser

Before we can implement any fancy functionality, 
we first need to be able to **read/write** JSON files and **parse** the JSON syntax.

Luckily for you, we provide a fully-functional JSON parser!
We highly recommend that you take the time to familiarize yourself with the parser code.

Even though this parser is built to parse JSON, the same design can be used to parse nearly any language!

### Using the parser

To connect your code to the parser, your `Model` must inherit from `JSONListener` and implement the pure virtual methods.
These listener methods will be called by the parser each time a new JSON element is parsed and such that you can add the element to your `Model`.

It is your responsibility to implement these methods correctly. We will go into more detail about `Model` and the `JSONListener` methods in the
next section.

> We have added convenient `DBG()` print statements to the unfinished implementations of the `JSONListener` methods. Use them to better understand the listener methods!


## 2. The In-Memory Model

Now that we can parse JSON, we need a way for our computers to store the resulting information in memory for quick access!

### Why do we need a in-memory model?

We don't actually *need* one. We could instead perform all the queries on the raw text.
However, working with text is slow. 

It would be much faster if we can build our own representation of the JSON
data which the rest of our code could easily interact with!

Hence, the in-memory model.

### How do we build our in-memory model?

This is up to you. However, there is no point in reinventing the wheel, 
so here are some structures/patterns people use to create such models.

For something as dynamic and flexible as JSON, we need to build some sort of "graph".
This graph will need to store all the various JSON elements/values, 
where the values can be any of the 6 (or 7, if we split `number` into `long` and `double`) types described earlier.

> What STL container can be used to store many different types?

Each value/element in the JSON can be considered a "node" in our graph. 
To paint a clearer picture of what I mean, let's take a look at the following example JSON:

```json
{
  "stringNode": "Hello!",
  "numberNode": 42,
  "objectNode": {
    "innerStringNode": "Hello again!",
    "innerBoolNode": true,
    "innerListNode": [
      "item1", "item2", "item3"
    ]
  }
}
```

Let's start at the very beginning. The first character we see is the open-curly-brace `{`.
This character is associated with the start of an `object` type. 

What can the `object` type do? As we observed earlier, `object`s store key-value pairs. This is exactly what we see inside the curly-braces.

Since the root element of JSON is *always* an `object`, we know our model/graph will always start with an `object`.

So we can add an object to the root of our model/graph:

```
node(object) -> ...
```

Let's look inside this root `object` at the key-value pairs. 
The first one is `"stringNode": "Hello!"`, 
where the key is `"stringNode"` and the value is the string `"Hello!"`.

This means that inside our `object` node, we need to store this and all other key-value pairs,
where each key is a string and each value is another node in our graph.

> Think about which STL container can be used to store key-value pairs...

```
node(object) -> {
    key: "stringNode", value -> node(string)
    key: "numberNode", value -> node(int)
    key: "objectNode", value -> ???
}
```

Now that we have gotten to the `"objectNode"`, how should we update our graph?
Well, the value is a node like the other key-value pairs, just this time the node stores a value of type `object`:

```
node(object) -> {
    key: "stringNode", value -> node(string)
    key: "numberNode", value -> node(int)
    key: "objectNode", value -> node(object) -> {
        ...
    }
}
```

And guess what? Our node of type `object` can store additional key-value pairs, 
which is exactly what see in our JSON:

```
node(object) -> {
    key: "stringNode", value -> node(string)
    key: "numberNode", value -> node(int)
    key: "objectNode", value -> node(object) -> {
        key: "innerStringNode", value -> node(string)
        key: "innerBoolNode", value -> node(bool)
        key: "innerListNode", value -> node(list) -> {
            ...
        }
    }
}
```

What about the `"innerListNode`? Well lists store an array of values (NOT key-value pairs). Since values are just nodes for us, let's update our graph with an array of nodes:

> Think about which STL container can store dynamic arrays...

```
node(object) -> {
    key: "stringNode", value -> node(string)
    key: "numberNode", value -> node(int)
    key: "objectNode", value -> node(object) -> {
        key: "innerStringNode", value -> node(string)
        key: "innerBoolNode", value -> node(bool)
        key: "innerListNode", value -> node(list) -> {
            node(string), node(string), node(string)
        }
    }
}
```

...aaand there we have it! We have successfully created a model/graph representation of the JSON file!

### Using `JSONListener`

We have already touched on the `JSONListener` and its methods, but let's delve deeper now that we better
understand the in-memory model.

Here is the first method:

```cpp
// Add basic key-value data types (null, bool, number, or string)
bool addKeyValuePair(const std::string& aKey, const std::string& aValue, Element aType);
```

Like the comment says, this method will be called by the `JSONParser` each time it encounters a key-value pair
where the value is a basic type (either `null`, `boolean`, `number`, or `string`).

How do we know which these four types the value is? That is your responsibility, as you must assign the value to the appropriate type.

To help differentiate `string`s from other values, the `Element aType` argument will be `Element::quoted` for `string`s, and `Element::constant` otherwise.

This method will only be called when we are inside an `object` container.

```cpp
// Add values to a list
bool addItem(const std::string& aValue, Element aType);
```

If we are within a list rather than an object, 
then we should **not** be storing key-value pairs, just values.
This is what the `addItem()` method is for.

This method will only be called when we are inside a `list` container.

```cpp
// Start of an object or list container ('{' or '[')
bool openContainer(const std::string& aKey, Element aType);
```

When the parser encounters the start of an object or list, `openContainer()` will be called.
Use the `Element aType` argument to differentiate between the two.

If we are within an object, then this container will have a key associate with it, hence the
`const std::string& aKey` argument. However, if we are within a list, then this argument will be an empty string.

```cpp
// End of an object or list container ('}' or ']')
bool closeContainer(const std::string& aKey, Element aType);
```

Lastly, we have the `closeContainer()` method. As you may expect, this method is called at the end of an
object or list. 


## 3. Query Language

Now that we can build JSON in-memory models, we need a way to interact with the model.

Below we will describe the query API/syntax. These methods must be implemented within the `ModelQuery` class.

### Traversal

Used to navigate between nodes in the JSON structure.

```cpp
ModelQuery& select(const std::string& aQuery);
```
Traverse the JSON tree.

- The `aQuery` argument is a string that contains a series of **keys** (for key-value pairs within objects) and/or **indices** (for elements in lists).
  - The indices start counting from `0`, just like indexing an array in C++.
  - All keys will be surrounded by single quotes `'`.
  - All indices will **not** be surrounded by single quotes.
  - The different keys/indices will be separated by periods `.`.
- Think of this command as moving some sort of pointer that points to a JSON element/node.
- Ex: `select("'firstNode'.'secondNode'.5")`. This will navigate to the element with `'firstNode'` as the key, then the child that has the key of `'secondNode'` and lastly, the child that has the index of `5`, as `'secondNode'` contains a list.
- If `select()` is called with invalid keys/indices (meaning they don't exist within the JSON), make sure to handle these errors gracefully (more details on this below).

### Filtering

Used to 'skip' or 'ignore' certain JSON elements.

```cpp
ModelQuery& filter(const std::string& aQuery);
```
Filter certain elements within the currently selected container (list or object).

- The `aQuery` is a string which contains some sort of comparison. This comparison can be applied to the element's key or index.
- The filters do not apply to nested children elements, only the elements within the currently selected container (examples below).

Filtering by key: `filter("key {action} '{value}'")`.
- Actions: `contains`
- Ex: `filter("key contains 'hello'")`: Will only include JSON elements where the key contains the substring `"hello"`.
- This will only apply to key-value pairs within objects.

Filtering by index: `filter("index {comparison} {value}")`
- Comparisons: All 6 (`<`, `>`, `<=`, `>=`, `==`, `!=`) comparisons.
- Ex: `filter("index > 2")`: Will only include JSON elements where the index is greater than 2. 
- This only applies to elements within lists.

### Consuming

After navigating and filtering the JSON, 
these commands will be used at the end of the command chain to actually return some data/values.

- Notice how these methods return actual values instead of `ModelQuery&`.

```cpp
size_t count();
```
- Counts number of child elements of the currently selected node. Don't forget to apply the filter if necessary!

```cpp
double sum();
```
- Sum values in a list. This will only be used in lists of numbers. Don't forget to apply the filter if necessary!


```cpp
std::optional<std::string> get(const std::string& aKeyOrIndex);
```
- Get values of a certain key-value pair or value at index of a list. 
If the value is a list/object, be sure to return all the elements (view examples below).
- Passing `"*"` as the argument will return all child nodes of the currently selected node (see examples below). Don't forget to apply the filter if necessary!

### Example:

```json
{
  "sammy": {
    "name": "Sammy",
    "username": "SammyShark",
    "online": true,
    "followers": {
      "avg-age": 25,
      "count": 100
    }
  },
  "items": [
    {"key1": "100"}
  ],
  "list": [
    100, 250, 3000
  ]
}
```

- `select("'sammy'").get("'username'")`: Should result in `"SammyShark"`.

- `select("'sammy'.'uh_oh'").get("'nope'")`: Should result in `std::nullopt`, as the JSON does not contain these elements.

- `select("'list'").sum()`: Should result in `3350`.

- `select("'list'").filter("index >= 1").sum()`: Should skip the value at index `0` and result in `3250`.

- `select("'sammy'").count()`: There are four elements within the `"sammy"` object (`"name"`,`"username"`, `"online"`, `"followers"`), this should return `4`.

- `select("'sammy'.'followers'").get("*")`: Should return `{"avg-age":25,"count":100}`.

- `select("'sammy'.'followers'").get("'count'")`: Should return `100`.

- `select("'items'").get("0")`: Should return `{"key1":"100"}`.

- `select("'list'").filter("index < 2").get("*")`: Should return `[100,250]`.

- `select("'sammy'").filter("key contains 'name').get("*")`: Should return `{"name":"Sammy","username":"SammyShark"}` since these are the only key-value pairs within the `"sammy"` object that have keys containing the substring `"name"`.

These query commands should be called on the `ModelQuery` object. Here is an example:

```cpp
// Build the model...
std::fstream theJsonFile(getWorkingDirectoryPath() + "/Resources/classroom.json");
ECE141::JSONParser theParser(theJsonFile);
ECE141::Model theModel;
theParser.parse(&theModel);

// Query the model...
auto theQuery = theModel.createQuery();
auto theResult = theQuery.select("'sammy'").get("'username'");
```

#### Notes:

- When returning lists or objects, you do **not** need to introduce any formatting or spacing. The above examples show how you can format your `get()` outputs. That being said, the `AutoGrader` will remove any white space, so if you do want to add some additional formatting, go for it!
  - For formatting `double` values, we provide a `doubleToString()` function in `Formatting.h`.

- The order of these commands will always be the same: `select`, then optionally `filter`, and lastly a consumer (`sum`, `count`, or `get`).

- You are **not** required to support multiple filters (the tests only use a single filter), but it is recommended you try! For instance, imagine `select("'list'").filter("index > 0").filter("index != 2").get("*")`, this should return `[250]`.

- Filtering will only happen on collections (lists or objects). 

- What happens when `sum()` is called on a list without numbers? What about on an object? What if `filter(index > 1)` is called on an object instead of a list? It is up to you to decide what to do for these types of edge/error-cases, but make sure you do consider them. Write some tests for these cases!

#### Tips:

- Try to use the STL as much as possible! Maybe you can even reuse code from your earlier assignments...

- Plan ahead! Use the design patterns we have talked about in lecture as well as those described in the book.
  - Don't be afraid to try new ideas/designs! Commit to Git often so you can undo changes if they turn out to be... not so great. No one makes a perfect design on the first try!
  - Come by to office hours if you have questions!

- Start early! Even when you're not actively working on the assignment, there is a good chance you will at least be *thinking* about it. You might just surprise yourself with when/where you come up with solutions/ideas for this assignment.

- This assignment is easily the most complex one so far. Split the problems into smaller and more manageable chunks!

- Implementing `filter()` will likely be the most difficult part of this assignment, so leave it for the end! We even provide a `NoFilter` test to help you test everything other than `filter()`.

- We have provided debugging macros (in `Debug.h`). These are optional to use, but could be very handy for debugging issues!
  - `DBG(message)`: Like using `std::cout`, except using `DBG()` makes it very clear that the messages are intended for debugging. The `DBG()` macro also gets disabled in release builds.
  - `ASSERT(expression)`: If the expression is `false`, this macro will pause your program and point to the assertion. `ASSERT()` also gets disabled in release builds.
  - `TODO`: Doesn't do anything, just a simple reminder to finish implementing something. `TODO` will (purposely) cause compile errors in release builds to help you catch any unfinished code.


## 4. Tests

Just like the previous two assignments, we want you to write your own tests.
There are parts that the `AutoGrader` does **not** test, for instance, what happens when the `Model` or `ModelNode` copy constructors or assignment operators are called? What do you think should happen?

As before, we provide the `Testable` class. You are free to use this or any other testing approach. But do be sure to write tests!

## What to Submit

1. Functional `Model`, `ModelNode`, and `ModelQuery` classes.
    - Implement the `JSONListener` methods in `Model`.
    - Implement the query commands in `ModelQuery`.

2. Additional tests (using `Testable` or a custom solution).

## Grading

| Test | Grade |
|------|-------|
| Compile Test | 5% |
| Warnings Test | 5% |
| NoFilter Test | 30% |
| Query Test | 10% |
| Basic Test | 20% |
| Advanced Test | 20% |
| Manual Code Review | 10% |
