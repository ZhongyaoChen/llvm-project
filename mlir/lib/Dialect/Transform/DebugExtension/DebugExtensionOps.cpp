//===- DebugExtensionOps.cpp - Debug extension for the Transform dialect --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Transform/DebugExtension/DebugExtensionOps.h"

#include "mlir/Dialect/Transform/IR/TransformTypes.h"
#include "llvm/Support/InterleavedRange.h"

using namespace mlir;

#define GET_OP_CLASSES
#include "mlir/Dialect/Transform/DebugExtension/DebugExtensionOps.cpp.inc"

DiagnosedSilenceableFailure
transform::EmitRemarkAtOp::apply(transform::TransformRewriter &rewriter,
                                 transform::TransformResults &results,
                                 transform::TransformState &state) {
  if (isa<TransformHandleTypeInterface>(getAt().getType())) {
    auto payload = state.getPayloadOps(getAt());
    for (Operation *op : payload)
      op->emitRemark() << getMessage();
    return DiagnosedSilenceableFailure::success();
  }

  assert(isa<transform::TransformValueHandleTypeInterface>(getAt().getType()) &&
         "unhandled kind of transform type");

  auto describeValue = [](Diagnostic &os, Value value) {
    os << "value handle points to ";
    if (auto arg = llvm::dyn_cast<BlockArgument>(value)) {
      os << "a block argument #" << arg.getArgNumber() << " in block #"
         << std::distance(arg.getOwner()->getParent()->begin(),
                          arg.getOwner()->getIterator())
         << " in region #" << arg.getOwner()->getParent()->getRegionNumber();
    } else {
      os << "an op result #" << llvm::cast<OpResult>(value).getResultNumber();
    }
  };

  for (Value value : state.getPayloadValues(getAt())) {
    InFlightDiagnostic diag = ::emitRemark(value.getLoc()) << getMessage();
    describeValue(diag.attachNote(), value);
  }

  return DiagnosedSilenceableFailure::success();
}

DiagnosedSilenceableFailure
transform::EmitParamAsRemarkOp::apply(transform::TransformRewriter &rewriter,
                                      transform::TransformResults &results,
                                      transform::TransformState &state) {
  std::string str;
  llvm::raw_string_ostream os(str);
  if (getMessage())
    os << *getMessage() << " ";
  os << llvm::interleaved(state.getParams(getParam()));
  if (!getAnchor()) {
    emitRemark() << str;
    return DiagnosedSilenceableFailure::success();
  }
  for (Operation *payload : state.getPayloadOps(getAnchor()))
    ::mlir::emitRemark(payload->getLoc()) << str;
  return DiagnosedSilenceableFailure::success();
}
