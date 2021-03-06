// Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include <map>
#include <string>
#include <vector>
#include "lite/utils/all.h"
#include "lite/utils/replace_stl/stream.h"

namespace paddle {
namespace lite {

/*
 * Compatible interfaces for all the different kinds of XXXDesc. All the XXXDesc
 * classes should implement this.
 */

class VarDescAPI {
 public:
  enum class Type {
    // Pod Types
    BOOL = 0,
    INT16,
    INT32,
    INT64,
    FP16,
    FP32,
    FP64,
    // Tensor<size_t> is used in C++.
    SIZE_T,
    UINT8,
    INT8,

    // Other types that may need additional descriptions
    LOD_TENSOR,
    SELECTED_ROWS,
    FEED_MINIBATCH,
    FETCH_LIST,
    STEP_SCOPES,
    LOD_RANK_TABLE,
    LOD_TENSOR_ARRAY,
    PLACE_LIST,
    READER,
    // Any runtime decided variable type is raw
    // raw variables should manage their own allocations
    // in operators like nccl_op
    RAW,
    TUPLE
  };

  using VarDataType = Type;

  virtual ~VarDescAPI() = default;

  // Get var's name
  virtual std::string Name() const = 0;
  // Set var's name
  virtual void SetName(std::string name) = 0;
  // Get var's type
  virtual Type GetType() const = 0;
  // Set var's type
  virtual void SetType(Type type) = 0;
  // Tell whether var is persistable or not
  virtual bool Persistable() const = 0;
  // Set var to be persistable or not
  virtual void SetPersistable(bool persistable) = 0;
  // Get var's shape
  virtual std::vector<int64_t> GetShape() const = 0;
  // Set var's shape
  virtual void SetShape(const std::vector<int64_t>& dims) = 0;
};

/*
 * NOTE Some interfaces are weried, we remain them unchanged to keep compatible
 * with framework::OpDesc in Fluid framework.
 */
class OpDescAPI {
 public:
  // The AttrType is used to make the proto::AttrType portable.
  enum class AttrType {
    INT = 0,
    FLOAT = 1,
    STRING = 2,
    INTS = 3,
    FLOATS = 4,
    STRINGS = 5,
    BOOLEAN = 6,
    BOOLEANS = 7,
    BLOCK = 8,
    LONG = 9,
    BLOCKS = 10,
    LONGS = 11,
    UNK,
  };

  template <AttrType Type>
  struct AttrTypeTrait;

  template <typename T>
  struct DataTypeTrait;

  virtual ~OpDescAPI() = default;

  /// Get operator's type.
  virtual std::string Type() const = 0;
  /// Set operator's type.
  virtual void SetType(const std::string& type) = 0;
  /// Get arguments given the parameter.
  virtual std::vector<std::string> Input(const std::string& param) const = 0;
  /// Get parameters.
  virtual std::vector<std::string> InputArgumentNames() const = 0;
  /// Get arguments given the parameter.
  virtual std::vector<std::string> Output(const std::string& param) const = 0;
  /// Get parameters.
  virtual std::vector<std::string> OutputArgumentNames() const = 0;
  /// Set a input given the parameter and arguments.
  virtual void SetInput(const std::string& param,
                        const std::vector<std::string>& args) = 0;
  virtual void SetOutput(const std::string& param,
                         const std::vector<std::string>& args) = 0;
  /// Tell whether this desc has an attribute.
  virtual bool HasAttr(const std::string& name) const = 0;

  /// Get the type of an attribute.
  virtual AttrType GetAttrType(const std::string& name) const = 0;

  virtual std::vector<std::string> AttrNames() const = 0;

  /// Set an attribute.
  template <typename T>
  void SetAttr(const std::string& name, const T& v);

  /// Get an attribute.
  template <typename T>
  T GetAttr(const std::string& name) const;

  std::string Repr() const {
    STL::stringstream ss;
    ss << Type();
    ss << "(";
    for (auto& arg : InputArgumentNames()) {
      ss << arg << ":";
      for (auto val : Input(arg)) {
        ss << val << " ";
      }
    }
    ss << ") -> (";
    for (auto& arg : OutputArgumentNames()) {
      ss << arg << ":";
      for (auto val : Output(arg)) {
        ss << val << " ";
      }
    }
    ss << ")";
    return ss.str();
  }
};

#define TYPE_TRAIT_IMPL(T, type__)                          \
  template <>                                               \
  struct OpDescAPI::AttrTypeTrait<OpDescAPI::AttrType::T> { \
    typedef type__ DT;                                      \
  };                                                        \
  template <>                                               \
  struct OpDescAPI::DataTypeTrait<type__> {                 \
    static constexpr AttrType AT = OpDescAPI::AttrType::T;  \
    static constexpr const char* ATN = #T;                  \
  };

TYPE_TRAIT_IMPL(INT, int32_t);
TYPE_TRAIT_IMPL(FLOAT, float);
TYPE_TRAIT_IMPL(STRING, std::string);
TYPE_TRAIT_IMPL(BOOLEAN, bool);
TYPE_TRAIT_IMPL(LONG, int64_t);
TYPE_TRAIT_IMPL(INTS, std::vector<int>);
TYPE_TRAIT_IMPL(FLOATS, std::vector<float>);
TYPE_TRAIT_IMPL(STRINGS, std::vector<std::string>);
TYPE_TRAIT_IMPL(LONGS, std::vector<int64_t>);
#undef TYPE_TRAIT_IMPL

class BlockDescAPI {
 public:
  virtual ~BlockDescAPI() = default;

  virtual int32_t Idx() const = 0;

  virtual void SetIdx(int32_t idx) = 0;

  virtual int32_t ParentIdx() const = 0;

  virtual void SetParentIdx(int32_t idx) = 0;

  virtual size_t VarsSize() const = 0;

  virtual void ClearVars() = 0;

  // NOTE: This ugly method is used to compatible interfaces between cpp and
  // pb/nb backends
  // TODO(sangoly): refine this
  template <typename T>
  T* GetVar(int32_t idx);

  template <typename T>
  T* AddVar();

  virtual size_t OpsSize() const = 0;

  virtual void ClearOps() = 0;

  // NOTE: This ugly method is used to compatible interfaces between cpp and
  // pb/nb backends
  // TODO(sangoly): refine this
  template <typename T>
  T* GetOp(int32_t idx);

  template <typename T>
  T* AddOp();

  virtual int32_t ForwardBlockIdx() const = 0;

  virtual void SetForwardBlockIdx(int32_t idx) = 0;
};

class ProgramDescAPI {
 public:
  virtual ~ProgramDescAPI() = default;

  virtual size_t BlocksSize() const = 0;

  virtual void ClearBlocks() = 0;

  // NOTE: This ugly method is used to compatible interfaces between cpp and
  // pb/nb backends
  // TODO(sangoly): refine this
  template <typename T>
  T* GetBlock(int32_t idx);

  template <typename T>
  T* AddBlock();

  virtual bool HasVersion() const = 0;

  virtual int64_t Version() const = 0;

  virtual void SetVersion(int64_t version) = 0;
};

}  // namespace lite
}  // namespace paddle
