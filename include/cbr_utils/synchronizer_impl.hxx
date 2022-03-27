// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

#ifndef CBR_UTILS__SYNCHRONIZER_IMPL_HXX_
#define CBR_UTILS__SYNCHRONIZER_IMPL_HXX_

#include <deque>
#include <functional>
#include <limits>
#include <utility>

namespace cbr {

template<typename T, typename... Ts>
bool Synchronizer<T, Ts...>::search()
{
  static constexpr auto all_idx = std::make_index_sequence<1 + sizeof...(Ts)>{};

  keep_n_before_time(0, Synchronizer<>::m_next_t);

  //// BOOK KEEPING ////
  auto one_of_every_type =
    foldWithAnd([](const auto & impl) { return !impl.queue.empty(); }, all_idx);
  if (!one_of_every_type) { return false; }

  // keep at most one before pivot time in each queue
  auto pivot_time = max_first_time(all_idx);
  keep_n_before_time(1, pivot_time);

  // check if each queue has at least one element before and after pivot time
  auto searchable = foldWithAnd(
    [pivot_time](const auto & impl) {
      return impl.time_fcn(impl.queue.front()) <= pivot_time
          && impl.time_fcn(impl.queue.back()) >= pivot_time;
    },
    all_idx);

  if (!searchable) {
    // not every queue has an element after pivot time
    return false;
  }

  //// START SEARCH ////

  // base case
  int64_t min_t      = min_first_time(all_idx);
  int64_t max_t      = pivot_time;
  int64_t min_t_best = min_t;
  int64_t max_t_best = max_t;

  while (min_t < pivot_time) {
    increase_first_with_time(min_t);

    min_t = min_first_time(all_idx);
    max_t = max_first_time(all_idx);

    if (max_t - pivot_time >= max_t_best - min_t_best) {
      // can not improve upon optimal since min_t <= pivot_time
      break;
    }

    if (max_t - min_t < max_t_best - min_t_best) {
      max_t_best = max_t;
      min_t_best = min_t;
      mapApply([](auto & impl) { impl.optimal_idx = impl.search_idx; }, all_idx);
    }
  }

  // drop everything before optimal solution
  // call single callback on those that are dropped
  mapApply(
    [](auto & impl) {
      for (size_t i = 0; i != impl.optimal_idx; ++i) {
        std::invoke(impl.callback_this_, std::move(impl.queue.front()));
        impl.queue.pop_front();
      }
    },
    all_idx);

  // keep at most one before max_t_best (in case pivot is very old)
  keep_n_before_time(1, max_t_best);

  // set time to start searching for next msg
  Synchronizer<>::m_next_t = min_t_best + Synchronizer<>::m_delta_t;

  // optimal solution is now at front, call callback on set
  call_callback(all_idx);

  // search finished, drop optimal solution and reset search variables to zero
  mapApply(
    [](auto & impl) {
      impl.queue.pop_front();
      impl.search_idx  = 0;
      impl.optimal_idx = 0;
    },
    all_idx);
  return true;
}

template<typename T, typename... Ts>
void Synchronizer<T, Ts...>::printOn(std::ostream & os) const
{
  os << "Synchronizer size " << 1 + sizeof...(Ts) << " (dt=" << Synchronizer<>::m_delta_t
     << ", nt=" << Synchronizer<>::m_next_t << ")" << std::endl;
  size_t counter = 0;
  auto f         = [&os, &counter](const auto & impl) {
    if (impl.queue.empty()) {
      os << "Queue #" << counter << ": (empty)" << std::endl;
    } else {
      os << "Queue #" << counter << ": ";
      for (auto & item : impl.queue) { os << impl.time_fcn(item) << " "; }
      os << std::endl;
    }
    ++counter;
  };
  mapApply(std::move(f), std::make_index_sequence<1 + sizeof...(Ts)>{});
}

}  // namespace cbr

#endif  // CBR_UTILS__SYNCHRONIZER_IMPL_HXX_
