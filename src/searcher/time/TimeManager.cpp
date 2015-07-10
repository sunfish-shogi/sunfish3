/* TimeManager.cpp
 *
 * Kubo Ryosuke
 */

#include "TimeManager.h"
#include "core/def.h"
#include "logger/Logger.h"
#include <cmath>
#include <cassert>

#define ENABLE_EASY_LOG 1

namespace {

float sigmoid(float x) {
  CONSTEXPR float g = 4.0;
  return 1.0 / (1.0 + std::exp((-g)*x));
}

} // namespace

namespace sunfish {

void TimeManager::init() {
  depth_ = 0;
}

void TimeManager::nextDepth() {
  depth_++;
  assert(depth_ < Tree::StackSize);
}

void TimeManager::startDepth() {
  stack_[depth_].firstMove = Move::empty();
  stack_[depth_].firstValue = -Value::Inf;
}

void TimeManager::addMove(Move move, Value value) {
  if (value > stack_[depth_].firstValue) {
    stack_[depth_].firstMove = move;
    stack_[depth_].firstValue = value;
  }
}

bool TimeManager::isEasy(float limit, float elapsed) {
  CONSTEXPR int easyDepth = 5;

  if (depth_ <= easyDepth) {
    return false;
  }

  const auto& easy = stack_[depth_-easyDepth];
  const auto& prev = stack_[depth_-1];
  const auto& curr = stack_[depth_];

  limit = std::min(limit, 3600.0f);

  if (elapsed < std::max(limit * 0.02, 3.0)) {
    return false;
  }

  if (elapsed >= limit * 0.85f) {
#if ENABLE_EASY_LOG
    Loggers::message << __FILE_LINE__;
#endif
    return true;
  }

  float baseTime = std::max(limit * 0.25f, 3.0f);
  float r = 5.0f * sigmoid(0.7f * (elapsed - baseTime) / baseTime);

#if ENABLE_EASY_LOG
  {
    int easyDiff = curr.firstValue.int32() - easy.firstValue.int32();
    int prevDiff = curr.firstValue.int32() - prev.firstValue.int32();
    int isSame = curr.firstMove == easy.firstMove ? 1 : 0;
    Loggers::message << "time_manager," << r << ',' << easyDiff << ',' << prevDiff << ',' << isSame;
  }
#endif

  if (curr.firstValue >= easy.firstValue - (32 * r) && curr.firstValue <= easy.firstValue + (512 * r) &&
      curr.firstValue >= prev.firstValue - (16 * r) && curr.firstValue <= prev.firstValue + (256 * r)) {
#if ENABLE_EASY_LOG
    Loggers::message << __FILE_LINE__;
#endif
    return true;
  }

  if (curr.firstMove == easy.firstMove && curr.firstMove == prev.firstMove &&
      curr.firstValue >= prev.firstValue - (32 * r) && curr.firstValue <= prev.firstValue + (512 * r)) {
#if ENABLE_EASY_LOG
    Loggers::message << __FILE_LINE__;
#endif
    return true;
  }

  return false;
}

} // namespace sunfish
