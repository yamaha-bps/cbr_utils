// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_ros/blob/master/LICENSE


#ifndef CBR_UTILS__SYNCHRONIZER_HPP_
#define CBR_UTILS__SYNCHRONIZER_HPP_

#include <algorithm>
#include <deque>
#include <functional>
#include <limits>
#include <mutex>
#include <utility>


namespace cbr
{

// Synchronizer data structure: base definition
template<typename ... T>
class Synchronizer;


template<>
class Synchronizer<>
{
public:
  explicit Synchronizer(int64_t delta_t)
  : delta_t(delta_t),
    next_t(std::numeric_limits<int64_t>::min())
  {}

protected:
  std::mutex search_mtx;  // to run one search at a time
  int64_t delta_t, next_t;
};


// Synchronizer data structure: recursive definition
template<typename T, typename ... Ts>
class Synchronizer<T, Ts...>: public Synchronizer<Ts...>
{
public:
  using CallbackAll = std::function<void (T &&, Ts && ...)>;
  using CallbackThis = std::function<void (T &&)>;

  /**
   * @brief Synchronize a message stream
   * @tparams T, Ts variadic templates for message types
   * @param delta_t minimal time between messages
   *
   * Groups messages into _sets_ which contain one message
   * from each stream, and calls a provided callback on each set.
   *
   * Optionally also calls provided callbacks on non-synchronized elements
   *
   * Messages in each stream are assumed to arrive in order, if they
   * do not they will be discarded without triggerning the non-sync callback
   *
   * Takes ownership over objects passed as rvalues,
   * and also sends rvalues back to callback
   *
   *
   * Usage example:
   *
   * sync.set_time_fcn<0>([] (const Type0 & o0) {
   *   return static_cast<int64_t>(o0.time);
   * });
   * sync.set_time_fcn<1>([] (const Type1 & o1) {
   *   return static_cast<int64_t>(o1.time);
   * });
   * Synchronizer<Type0, Type1> sync;
   * sync.register_callback([] (Type1 && o0, Type1 && o1) {
   *   // performed for all synchronized groups
   * });
   * sync.register_nonsync_callback<1>([] (Type1 && o1) {
   *   // performed for all Type1 elements that are not synchronized into a set
   * });
   *
   * // add data, first callback is executed on all synchronized groups
   * sync.add_and_search<0>(o0_1);
   * sync.add_and_search<1>(o1_1);
   * /// ...
   */
  explicit Synchronizer<T, Ts...>(int64_t delta_t = 0)
  : Synchronizer<Ts...>(delta_t),
    impl{
      {},
      0,
      0,
      [](const T &) {return 0;},
      [](T &&) {return;}
    },
    callback_([](T &&, Ts && ...) {})
  {
  }

  /* Copies not allowed */
  Synchronizer<T, Ts...>(const Synchronizer<T, Ts...> &) = delete;
  Synchronizer<T, Ts...> & operator=(const Synchronizer<T, Ts...> &) = delete;
  /* FIXME(pettni): implement proper moving */
  Synchronizer<T, Ts...>(Synchronizer<T, Ts...> &&) = delete;
  Synchronizer<T, Ts...> & operator=(Synchronizer<T, Ts...> &&) = delete;
  ~Synchronizer<T, Ts...>() = default;

  /**
   * @brief Register a callback to use for synchronized element sets
   * @param c callback taking an r-value of each template type
   *
   * Example:
   *  > Synchronizer<Type1, Type2> sync;
   *  > sync.register_callback<1>([] (Type1 && t1, Type2 && t2) {
   *  >   // will be called on every synchronized set
   *  > })
   */
  template<typename S>
  void register_callback(S && c)
  {
    callback_ = c;
  }

  /**
   * @brief Register a callback to use on individual elements that are not synchronized
   * @param c callback taking an r-value for a single element
   *
   * Example:
   *  > Synchronizer<Type1, Type2> sync;
   *  > sync.register_nonsync_callback<1>([] (Type2 && t2) {
   *  >   // will be called on every non-synchronized t2
   *  > });
   */
  template<size_t k, typename S>
  void register_nonsync_callback(S && c)
  {
    if constexpr (k == 0) {
      impl.callback_this_ = c;
    }
    if constexpr (k != 0) {
      Synchronizer<Ts...>::template register_nonsync_callback<k - 1>(std::forward<S>(c));
    }
  }

  /**
   * @brief Set function to compute timestamps
   * @param f function T -> int64_t
   *
   *  Example:
   *  > Synchronizer<Type1, Type2> sync;
   *  > sync.set_time_fcn<1>([] (const Type2 & t) {
   *  >   return t.time;
   *  > });
   */
  template<size_t k, typename S>
  void set_time_fcn(S && f)
  {
    if constexpr (k == 0) {
      impl.time_fcn = f;
    }
    if constexpr (k != 0) {
      Synchronizer<Ts...>::template set_time_fcn<k - 1>(std::forward<S>(f));
    }
  }

  /**
   * @brief Insert new element
   * @param k queue to insert in
   * @param el new element to insert
   *
   * NOT THREAD-SAFE TOGETHER WITH SEARCH
   */
  template<size_t k, typename S>
  void add(S && el)
  {
    if constexpr (k == 0) {
      auto el_time = impl.time_fcn(el);
      if (el_time < Synchronizer<>::next_t) {
        return;  // doesn't respect minimal delta_t
      }
      if (!impl.queue.empty() && el_time < impl.time_fcn(impl.queue.back())) {
        return;  // time not monotonically increasing
      }
      impl.queue.emplace_back(std::forward<S>(el));
    }
    if constexpr (k != 0) {
      Synchronizer<Ts...>::template add<k - 1>(std::forward<S>(el));
    }
  }

  /**
   * Search for new synchronized sets, callback is called on found sets.
   * Returns true if a synchronized group was found.
   *
   * NOT THREAD-SAFE
   */
  bool search();

  /**
   * @brief insert new element and run search algorithm
   * @param k queue to insert in
   * @param el new element to insert
   *
   * This is thread-safe by skipping search if it is already running
   */
  template<size_t k, typename S>
  void add_and_search(S && el)
  {
    add<k>(std::forward<S>(el));
    if (Synchronizer<Ts...>::search_mtx.try_lock()) {
      bool search_more = true;
      while (search_more) {
        search_more = search();
      }
      Synchronizer<Ts...>::search_mtx.unlock();
    }
  }

  friend std::ostream & operator<<(std::ostream & os, const Synchronizer<T, Ts...> & s)
  {
    s.printOn(os);
    return os;
  }

protected:
  struct Impl
  {
    std::deque<T> queue;
    size_t search_idx, optimal_idx;
    std::function<int64_t(const T &)> time_fcn;
    CallbackThis callback_this_;
  };

  Impl impl{};
  CallbackAll callback_{};

  // Print on stream for debugging
  void printOn(std::ostream & os) const;

  // Move front element in each queue to callback
  template<size_t ... I>
  void call_callback(std::index_sequence<I...>)
  {
    return std::invoke(callback_, std::move(getImpl<I>().queue.front())...);
  }

  // for each queue keep at most n elements with a stamp smaller than time
  // single-element callback is used on those elements that are removed
  void keep_n_before_time(size_t n, int64_t time)
  {
    mapApply(
      [n, time](auto & impl) {
        while (impl.queue.size() >= n + 1 && impl.time_fcn(impl.queue.at(n)) <= time) {
          std::invoke(impl.callback_this_, std::move(impl.queue.front()));
          impl.queue.pop_front();
        }
      },
      std::make_index_sequence<1 + sizeof...(Ts)>{}
    );
  }

  // increase first counter for first element with stamp at least time
  void increase_first_with_time(int64_t time)
  {
    if (time >= impl.time_fcn(impl.queue.at(impl.search_idx))) {
      ++impl.search_idx;
      return;
    }
    if constexpr (sizeof...(Ts) != 0) {
      Synchronizer<Ts...>::increase_first_with_time(time);
    }
  }

  // return Impl & for given index
  template<size_t k>
  decltype(auto) getImpl() {
    if constexpr (k == 0) {
      // *INDENT-OFF*
      return (impl);  // return reference due to ()
      // *INDENT-ON*
    }
    if constexpr (k != 0) {
      return Synchronizer<Ts...>::template getImpl<k - 1>();
    }
  }

  // return const Impl & for given index
  template<size_t k>
  decltype(auto) getImpl() const {
    if constexpr (k == 0) {
      // *INDENT-OFF*
      return (impl);  // return reference due to ()
      // *INDENT-ON*
    }
    if constexpr (k != 0) {
      return Synchronizer<Ts...>::template getImpl<k - 1>();
    }
  }

  // Minimal search time across all queues
  template<size_t ... I>
  int64_t min_first_time(std::index_sequence<I...>) const
  {
    auto search_time = [](const auto & impl) {
        return impl.time_fcn(impl.queue.at(impl.search_idx));
      };
    return std::min<int64_t>({search_time(getImpl<I>()) ...});
  }

  // Maximal search time across all queues
  template<size_t ... I>
  int64_t max_first_time(std::index_sequence<I...>) const
  {
    auto search_time = [](const auto & impl) {
        return impl.time_fcn(impl.queue.at(impl.search_idx));
      };
    return std::max<int64_t>({search_time(getImpl<I>()) ...});
  }

  // fold with && over indices
  template<size_t... I, typename S>
  bool foldWithAnd(S && f, std::index_sequence<I...>) const
  {
    // *INDENT-OFF*
    return (true && ... && f(getImpl<I>()));
    // *INDENT-ON*
  }

  // apply function to each implementation in pack
  template<size_t ... I, typename S>
  void mapApply(S && f, std::index_sequence<I...>)
  {
    (std::invoke(f, getImpl<I>()), ...);
  }

  // apply const function to each implementation in pack
  template<size_t ... I, typename S>
  void mapApply(S && f, std::index_sequence<I...>) const
  {
    (std::invoke(f, getImpl<I>()), ...);
  }
};

}  // namespace cbr

#include "synchronizer_impl.hxx"

#endif  // CBR_UTILS__SYNCHRONIZER_HPP_
